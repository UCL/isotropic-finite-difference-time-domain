import os
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from hdf5storage import loadmat, savemat

LOCATION_OF_THIS_FILE = os.path.abspath(os.path.dirname(__file__))


@dataclass
class OverwriteWith:
    """Small container class that will be used when overwriting the "flag" variables usecd and intmethod when a config file specifies an 'adjust' field.

    Intended workflow along the lines of:
    intmethod = OverwriteWith(whether_to_overwrite, value_to_replace_with)
    if intmethod.write:
        h5data['intmethod'] = intmethod.value
    else:
        pass
    """

    # Flags whether or not to overwrite a value with self.value
    write: bool
    # The value to write if we are intending to replace a value in a file
    value: bool | int | float | None


def edit_mat_file(
    test_directory: Path | str,
    to_produce: Path | str,
    options: dict[str, str | bool | int],
) -> None:
    """Produce a new .mat file by copying another .mat file and changing some of the TDMS "flag" variables according to the options provided.

    This function can adjust the values of the following variables:
    - usecd: Default value is 1 (FDTD). 0 results in use of PSTD
    - intmethod: Default value is 1 (cubic). 2 results in BLI

    :param test_directory: Directory in which the existing .mat file to use as a basis is stored.
    :param to_produce: Filename to write the edited .mat file to.
    :param options: Dictionary specifying the variables to adjust and the new values to assign to them.
    """
    # Get name of input file to produce immediately
    to_produce = f"{str(test_directory)}/{str(to_produce)}"
    # Read the name of the .mat file to adjust
    adjust_file = f"{test_directory}/{options['adjust']}"

    # Read the variable values that we want to override, provided they exist
    # Check if we want to overwrite the solver method
    if "solver_method" in options.keys():
        # If this key is present, we want to overwrite the value of usecd with the value provided
        if options["solver_method"] == "pstd":
            # Overwrite usecd with 0 (pstd)
            usecd = OverwriteWith(True, 0.0)
        elif options["solver_method"] == "fdtd":
            # Overwrite usecd with 1 (fdtd, default)
            usecd = OverwriteWith(True, 1.0)
        else:
            raise RuntimeError(
                f"Error: {options['solver_method']} is not a valid solver method"
            )
    else:
        # This key is not present, so we do not want to overwrite
        # Note that the self.usecd.value variable will not be used in this case, so we can set it to anything
        usecd = OverwriteWith(False, None)

    # Check if we want to overwrite the interpolation method
    if "interpolation" in options.keys():
        # We will be overwriting the value here
        if options["interpolation"] == "bli":
            # Overwrite intmethod with 2 (band-limited)
            intmethod = OverwriteWith(True, 2.0)
        elif options["interpolation"] == "cubic":
            # Overwrite intmethod with 1 (cubic, default)
            intmethod = OverwriteWith(True, 1.0)
        else:
            raise RuntimeError(
                f"Error: {options['interpolation']} is not a valid interpolation method"
            )
    else:
        # Overwrite not requested
        intmethod = OverwriteWith(False, None)

    # Load the .mat file that is to be adjusted
    mat_data = loadmat(adjust_file)

    # Adjust usecd if necessary
    if usecd.write:
        mat_data["usecd"] = np.array([[usecd.value]], dtype=float)
    # Adjust intmethod if necessary
    if intmethod.write:
        mat_data["intmethod"] = np.array([[intmethod.value]], dtype=float)

    # Write the data to the desired location - THROWS DEPRECATION WARNING FROM NUMPY!
    savemat(to_produce, mat_data, format="7.3", store_python_metadata=False)

    # Complete, return
    return
