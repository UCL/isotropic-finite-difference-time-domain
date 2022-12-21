/**
 * @file main.cpp
 * @brief The main function. Launches TDMS.
 */
#include <vector>
#include <string>

#include <spdlog/spdlog.h>

#include "openandorder.h"
#include "iterator.h"
#include "mat_io.h"

#define NMATRICES 49              //< number of input matrices
#define NOUTMATRICES_WRITE 23     //< number of output matrices to be written to output file
#define NOUTMATRICES_WRITE_ALL 25 //< number of output matrices to be written to output file
#define NOUTMATRICES_PASSED 31    //< number of output matrices passed by mexFunction

int main(int nargs, char *argv[]){

  // Set the logging level with a compile-time #define for debugging.
  #if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
  #elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_INFO
    spdlog::set_level(spdlog::level::info);
  #endif

  // the names of the matrices expected in the input file with a separate grid file
  std::vector<std::string> matrixnames_infile = {
    "Cmaterial", "Dmaterial", "C", "D", "freespace", "disp_params", "delta",
    "interface", "Isource", "Jsource", "Ksource", "grid_labels", "omega_an",
    "to_l", "hwhm", "Dxl", "Dxu", "Dyl", "Dyu", "Dzl", "Dzu", "Nt", "dt",
    "tind", "sourcemode", "runmode", "exphasorsvolume", "exphasorssurface",
    "intphasorssurface", "phasorsurface", "phasorinc", "dimension",
    "conductive_aux", "dispersive_aux", "structure", "f_ex_vec",
    "exdetintegral", "f_vec", "Pupil", "D_tilde", "k_det_obs_global",
    "air_interface", "intmatprops", "intmethod", "tdfield", "tdfdir",
    "fieldsample", "campssample"
  };
  // the name of the matrix expected in a separate grid file
  std::vector<std::string> matrixnames_gridfile = {"fdtdgrid"};

  // the names of the matrices expected in the input file if the grid is included
  std::vector<std::string> matrixnames_input_and_grid(matrixnames_infile);
  matrixnames_input_and_grid.insert(
    matrixnames_input_and_grid.end(),
    matrixnames_gridfile.begin(),
    matrixnames_gridfile.end()
  );

  std::vector<std::string> outputmatrices_all = {
    "Ex_out", "Ey_out", "Ez_out", "Hx_out", "Hy_out", "Hz_out", "x_out",
    "y_out", "z_out", "Ex_i", "Ey_i", "Ez_i", "Hx_i", "Hy_i", "Hz_i", "x_i",
    "y_i", "z_i", "vertices", "camplitudes", "facets", "maxresfield", "Id",
    "fieldsample", "campssample"
  };
  std::vector<std::string> outputmatrices = {
    "Ex_out", "Ey_out", "Ez_out", "Hx_out", "Hy_out", "Hz_out", "x_out",
    "y_out", "z_out", "Ex_i", "Ey_i", "Ez_i", "Hx_i", "Hy_i", "Hz_i", "x_i",
    "y_i", "z_i", "camplitudes", "maxresfield", "Id", "fieldsample",
    "campssample"
  };
  int matricestosave_all[] = {0, 1, 2, 3, 4, 5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
  int matricestosave[]     = {0, 1, 2, 3, 4, 5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,     23,     25, 26, 27, 28};

  mxArray *plhs[NOUTMATRICES_PASSED];
  const mxArray *matrixptrs[NMATRICES];

  auto args = ArgumentParser::parse_args(nargs, argv);
  check_files_can_be_accessed(args);

  // Now it is safe to use matlab routines to open the file and order the matrices.
  // There are two cases to consider, when the fdtdgrid matrix is specified in a
  // separate mat file and when it is in the same file as the other matrices.
  if(!args.has_grid_filename())
    openandorder(args.input_filename(), matrixnames_input_and_grid, matrixptrs, NMATRICES);
  else{
    openandorder(args.input_filename(), matrixnames_infile, matrixptrs+1, NMATRICES-1);
    openandorder(args.grid_filename(), matrixnames_gridfile, matrixptrs, 1);
  }

  // decide which derivative method to use (PSTD or FDTD)
  SolverMethod solver_method = PseudoSpectral; // default
  if (args.finite_difference())
    solver_method = SolverMethod::FiniteDifference;

  // decide whether to toggle off the band-limited interpolation methods
  PreferredInterpolationMethods preferred_interpolation_methods =
          PreferredInterpolationMethods::BandLimited;// default
  if (args.cubic_interpolation()) {
    preferred_interpolation_methods = PreferredInterpolationMethods::Cubic;
  }

  // now run the time propagation code
  execute_simulation(NOUTMATRICES_PASSED, (mxArray **) plhs, NMATRICES,
                     (const mxArray **) matrixptrs, solver_method, preferred_interpolation_methods);

  if (!args.have_flag("-m")) {//prints vertices and facets
    saveoutput(plhs, matricestosave_all, outputmatrices_all, NOUTMATRICES_WRITE_ALL,
               args.output_filename());
  } else {// minimise the file size by not printing vertices and facets
    saveoutput(plhs, matricestosave, outputmatrices, NOUTMATRICES_WRITE,
               args.output_filename());
  }

  return 0;
}
