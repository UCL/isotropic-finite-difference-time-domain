import os
from io import StringIO
from pathlib import Path
from typing import Tuple, Union

import matlab.engine as matlab
from matlab.engine import MatlabEngine

LOCATION_OF_THIS_FILE = os.path.dirname(os.path.abspath(__file__))
# Additional options for running matlab on the command-line
MATLAB_OPTS_LIST = ["-nodisplay", "-nodesktop", "-nosplash", "-r"]
MATLAB_STARTUP_OPTS = " ".join(MATLAB_OPTS_LIST)
# Paths to matlab functions not in LOCATION_OF_THIS_FILE
MATLAB_EXTRA_PATHS = [
    os.path.abspath(LOCATION_OF_THIS_FILE + "/bscan"),
    os.path.abspath(LOCATION_OF_THIS_FILE + "/matlab"),
]


class BScanArguments:
    """Class to tidily contain the input arguments that need to be passed into the MATLAB function of the same name, to generate the input data.

    The bscan/run_bscan.m file contains the matlab function which generates the input data. Regrettably, we need to specify particular inputs to this script for each test, which requires us to translate the argument values as read from the config.yaml file into a long string of values in the correct order, which can in turn be called from matlab.

    BScanArguments is essentially a glorified dictionary, it's members sharing the names of the input arguments to run_bscan. It's create_bscan_argument can be used to convert the values that need to be passed into a string of the form:
    run_bscan(input_arguments_in_the_correct_order).
    """

    # Without these tags in the input_generation field of the config file, we cannot setup the test
    REQUIRED_FIELDS: list[str] = ["input_file", "spatial_obstacles"]

    # The directory of the test whose input data is being generated
    test_directory: str
    # The path to the input file that defines the variables iteratefdtd_matrix.m takes in
    input_file: str
    # The obstacles present in the simulations we are generating inputs for
    obstacles: list[str]
    # Flags whether the BScan call requires a call to iteratefdtd_matrix in illsetup mode as well as filesetup mode
    # Default is False
    illsetup: bool
    # Radius of the spatial obstacle in microns (circular face radius for cyl, sphere radius for sph)
    # Default is 15.e-6
    obstacle_radius: float
    # Whether calc_field_tdfield needs to be run prior to setting up the scattering matrix
    calc_tdfield: bool

    def __init__(
        self, test_directory: Union[Path, str], input_generation_config: dict[str, any]
    ) -> None:
        """Initialise by unpacking values from the dictionary generated by the config file."""
        # This information is not in the dictionary we get passed, it is set within each system test itself.
        self.test_directory = str(test_directory)
        # Keys present in the config file
        keys = input_generation_config.keys()

        # Check for compulsory inputs, error if not present
        for required_field in self.REQUIRED_FIELDS:
            if required_field not in keys:
                raise RuntimeError(
                    f"Error: required field {required_field} not found in config"
                )

        # Set required fields, which exist if we got to here without throwing an error
        # Input file [check if file exists beforehand, and preppend the absolute path to the expected location]
        path_to_input_file = Path(
            LOCATION_OF_THIS_FILE, input_generation_config["input_file"]
        )
        if not path_to_input_file.exists():
            raise RuntimeError(f"{path_to_input_file} does not exist")
        else:
            self.input_file = str(path_to_input_file)
        # Obstacles [check if obstacles are specified as a list of strings]
        self.obstacles = list(input_generation_config["spatial_obstacles"])
        if not all(isinstance(obstacle, str) for obstacle in self.obstacles):
            raise RuntimeError(
                f"Error: obstacles list {self.obstacles} contains non-string elements"
            )

        # Set optional fields to defaults if not present, or use values provided otherwise
        if "illsetup" in keys:
            self.illsetup = bool(input_generation_config["illsetup"])
        else:
            self.illsetup = False
        if "obstacle_radius" in keys and (
            input_generation_config["obstacle_radius"] != None
        ):
            self.obstacle_radius = float(input_generation_config["obstacle_radius"])
        else:
            self.obstacle_radius = 15.0e-6
        if "calc_tdfield" in keys:
            self.calc_tdfield = bool(input_generation_config["calc_tdfield"])
        else:
            self.calc_tdfield = False

        # Setup complete, return
        return

    def _temp_filesetup_name(self) -> str:
        """In the event that illsetup is required for this run, generate the temporary name for the input file to be passed to iteratefdtd_matrix in filesetup mode.

        Only occurs when illumination files are required in input-data regeneration. In this case, self.input_file is passed to iteratefdtd_matrix in 'illsetup' mode.
        """
        # Input file name without the extension (.m)
        raw_input_name = os.path.splitext(self.input_file)[0]
        # Append to filename an suitably unique extension
        raw_input_name += "temp_filesetup_file____.m"
        return raw_input_name

    def _create_temp_filesetup(self) -> None:
        """Generate the tempoaray file to be passed to iteratefdtd_matrix in filesetup mode when an illumination file has also been specified. The "filesetup" mode-file is essentially identical to it's counterpart, but needs the efname and hfname variables TO to be present, but set to empty strings.

        As such, the optimal way to get around this is to have Python copy the input_file (which in this instance is what needs to be used to setup the illumination), and then change the definition of the efname & hfname variables to empty strings in the copied file to create the "filesetup" input.
        """
        # Copy the input_file (illumination-input) line-by-line to a temporary location for the filesetup-input
        # Do not copy across the lines that define the efname and hfname variables
        with open(self.input_file, "r") as illumination_input:
            with open(self._temp_filesetup_name(), "w") as filesetup_input:
                for line in illumination_input:
                    # Remove any potential whitespace padding from the line
                    # This avoids funny business if there's whitespace around the = symbol where {ef,hf}name are defined
                    stripped_line = line.replace(" ", "")
                    # Write line, provided efname or hfname are not defined on it
                    if not (
                        ("efname=" in stripped_line) or ("hfname=" in stripped_line)
                    ):
                        filesetup_input.write(line)
                    elif "efname=" in stripped_line:
                        filesetup_input.write("efname = '';\n")
                    elif "hfname=" in stripped_line:
                        filesetup_input.write("hfname = '';\n")
        # filesetup_input is now ready, and identical to illumination_input save in the definition of efname and hfname
        return

    def non_freespace_obstacle(self) -> str:
        """Returns the shape of the scattering object present in the non-freespace simulation, as a string.

        Throws an error in the event that there is no such object, or if there are multiple such objects.
        """
        # Error if self.obstacles is not set
        if not self.obstacles:
            raise RuntimeError(f"Error: no obstacles specified in call to BScan!")
        # Create a copy of obstacles so we don't overwrite our member variable
        obstacles = list(self.obstacles)
        # We should be able to pop out 'fs' now - if we can't then we should error
        # We can use while here in case there are duplications of 'fs', as we self-update each time
        while "fs" in obstacles:
            obstacles.remove("fs")
        # Now obstacles should be left with only 1 element (the desired obstacle), if not then the non-freespace obstacle is non-unique and we error
        if len(obstacles) != 1:
            raise RuntimeError(
                f"Error: non-freespace obstacle is not unique ({obstacles})"
            )
        # If we didn't error, we can return the only element of obstacles, which is the non-fs obstacle
        return obstacles[0]

    def run_bscan(self, engine: MatlabEngine) -> Tuple[StringIO, StringIO]:
        """Runs the run_bscan function in the MatlabEngine provided.

        The bscan/ and matlab/ directories are assumed to already be in the includepath of the engine instance, so that the run_bscan and supporting matlab files can be called.

        Returns a tuple of the stdout and stderr from the engine upon executing this command.
        """
        # If illumination file is needed, we need to copy the input file and remove the lines that define the efname and hfname as necessary
        illfile_extra_file = ""
        if self.illsetup:
            # If this is _not_ an empty string, we need to generate the illumination file from the input file
            # It is essentially identical, but needs the efname and hfname variables to be:
            # present in the workspace/file when calling with 'illsetup'
            # absent from the workspace/file when calling with 'filesetup'
            # Otherwise, the "input" file to the illsetup and input file for the .mat creation are identical. As such, the optimal way to get around this is to have Python pass the input_file via 'illsetup', then copy this file, remove the definition of the efname & hfname variables, then run 'filesetp' mode. We can then cleanup our "extra" file that we created.
            illfile_extra_file = self._temp_filesetup_name()
            self._create_temp_filesetup()

        # prepare IO objects to save polluting the terminal
        stdout_capture = StringIO()
        stderr_capture = StringIO()

        # function [] = run_bscan(test_directory, input_filename, non_fs_obstacle, illfile_extra_file, obstacle_radius, calc_tdfield)
        # pass nargout=0 to indicate no value is to be returned to Python
        engine.run_bscan(
            self.test_directory,
            self.input_file,
            self.non_freespace_obstacle(),
            illfile_extra_file,
            self.obstacle_radius,
            self.calc_tdfield,
            nargout=0,
            stdout=stdout_capture,
            stderr=stderr_capture,
        )

        # Cleanup the temporary illumination file, if we created it
        if self.illsetup:
            os.remove(illfile_extra_file)
        return stdout_capture, stderr_capture


class MATLABEngineWrapper:
    """When we regenerate input data, we always need to add the bscan/ and matlab/ directories to the MATLAB instance's search path. correspondingly, we always want to kill the MATLAB instance after running the run_bscan function and generating the data.

    This class serves as a wrapper for that purpose. It stores instance(s) of the BScanArguments class, which it will run in sequence between the aforementioned addpath() setup and then engine shutdown. The .run() method performs exactly this.
    """

    # List containing the subsequent matlab commands to be executed by the interpreter
    bscan_calls: list[BScanArguments]

    # The MATLAB session that will run
    engine: MatlabEngine
    # The working directory this engine operated in. Useful for cleanup operations later
    cwd: str

    def __init__(self, bscans: Union[BScanArguments, list[BScanArguments]]) -> None:
        """Initialise with the provided (list of) BScanArguments."""
        self.bscan_calls = []
        # add the additional commands here
        if isinstance(bscans, BScanArguments):
            # handed BScanArgument, construct command from this
            self.bscan_calls = [bscans]
        elif isinstance(bscans, list):
            # validate this is a list of BScanArguments and not some other datatype
            non_bscan_items = sum([isinstance(b, BScanArguments) for b in bscans])
            if non_bscan_items > 0:
                raise RuntimeError(
                    f"Error: not all inputs are BScan calls ({non_bscan_items}/{len(bscans)})"
                )
            else:
                self.bscan_calls = bscans
        else:
            # I don't know what we've been handed, but it's not what I was expecting
            raise RuntimeError(
                f"Error: expected BScanArguments or list[BScanArguments] but got {type(bscans)}"
            )
        # do not start the engine yet
        self.engine = None
        # likewise, we do not know the working directory yet
        self.cwd = None
        return

    def _addpath_commands(self) -> None:
        """Adds the bscan/ and matlab/ directories to the session's path."""
        for path in MATLAB_EXTRA_PATHS:
            self.engine.addpath(path)
        return

    def _stop_engine(self) -> None:
        """Kills engine if it is still running."""
        self.engine.quit()
        return

    def run(self, kill_on_complete: bool = True) -> None:
        """Run the bscan arguments saved to the instance, in the same session.

        Engine is terminated if kill_on_complete is True, otherwise the engine is left running and manual cleanup is needed. It can be useful for debugging to leave the engine running, however it is recommended to stop it after we have finished using it.
        """
        # If we have no bscan arguments to run, don't bother starting the engine and report this
        if len(self.bscan_calls) == 0:
            raise RuntimeWarning(
                "No bscan calls specified in this instance. Engine not started."
            )
        else:
            # Start the engine
            self.engine = matlab.start_matlab(MATLAB_STARTUP_OPTS)
            # Move to a suitable working directory, just to keep things tidy
            self.engine.cd(LOCATION_OF_THIS_FILE)
            # Pull the matlabengine working directory for reference later
            self.cwd = self.engine.pwd()
            # Add necessary paths
            self._addpath_commands()

            # Run every bscan call
            for b in self.bscan_calls:
                b.run_bscan(self.engine)

            # If requested, kill the instance
            if kill_on_complete:
                self._stop_engine()
        return
