/**
 * @file iterator.h
 * @brief Class that handles variables during the TDMS simulation; it is passed the inputs and command line arguments, runs the FDTD solver, and passes appropriate information back to the output pointers, then tears down any memory that was reserved but is no longer needed.
 *
 * Also declares the execute_simulation() function, which is the workflow for the process described above.
 */
#pragma once

#include <spdlog/spdlog.h>

#include "matrix.h"

#include "iterator_executor.h"

/**
 * @brief Class that handles the setup, execution, output passing, and tear-down of the FDTD/PSTD simulation.
 *
 * This class is the lowest in the Iterator_ hierarchy, and explicitly defines methods for passing the results of the simulation back to the main() function via the plhs[] pointer-array.
 * It also handles the tear-down/cleanup of any malloc'd memory THAT IS NOT REQUIRED by the outputs themselves, but must be reserved when writing the outputs themselves (MATLAB is weird, TODO: Update when HDF5 comes in).
 *
 * This class handles the functionality of what was previously the file iterator.cpp.
 */
class Iterator : public Iterator_Executor {
public:
  Iterator(InputMatrices matrices_from_input_file, SolverMethod _solver_method,
           PreferredInterpolationMethods interpolation_method)
      : Iterator_Executor(matrices_from_input_file, _solver_method, interpolation_method){};

  /**
   * @brief Normalise the {E,H}-field phasors in the volume (if extracting phasors in the whole volume)
   */
  void normalise_field_volumes();
  /**
   * @brief Normalise the phasors on the user-defined surface (if extracting there)
   */
  void normalise_surface_phasors();
  /**
   * @brief Normalise the phasors at the user requested vertices (if there are any)
   */
  void normalise_vertex_phasors();
  /**
   * @brief Normalise the Id-array entries (if this is necessary)
   */
  void normalise_Id_arrays();

  void initialise_output_labels_from_input_labels() {
    output_grid_labels.initialise_from(input_grid_labels, E.il, E.iu, E.jl, E.ju, E.kl, E.ku);
  }

  /**
   * @brief Interpolate the extracted field values to the centres of the Yee cells, placing the interpolated values into the output.
   *
   * @param output_matrices The collection of output pointers
   */
  void interpolate_field_values(mxArray *output_matrices[]);
  /**
   * @brief Write the gridlabels (co-ordinates) of the interpolated fields to the output.
   *
   * @param output_matrices The collection of output pointers
   */
  void write_interpolated_gridlabels(mxArray *output_matrices[]);

  /**
   * @brief Regenerate the mesh of the user-defined surface.
   *
   * This is deleted prior to iterating to save memory, we recompute it after the main loop only if the user has requested it be exported.
   * Additionally, when we regenerate it we no longer want to re-initialise the surface_phasors we have computed, so we cannot recycle the setup_surface_mesh() method.
   *
   * @param output_matrices The collection of output pointers
   */
  void regenerate_mesh_for_facets(mxArray *output_matrices[]);

  /**
   * @brief Return the largest split-field value across the E- and H-split-fields
   */
  double compute_max_split_field_value() {
    return std::max(E_s.largest_field_value(), H_s.largest_field_value());
  }
};

/*This function will take in the following arguments and perform the
 entire simulation

  fdtdgrid
  Cmaterial
  Dmaterial
  C
  D
  freespace
  disp_params
  delta
  interface
  Isource
  Jsource
  Ksource
  grid_labels
  omega_an
  to_l
  hwhm
  Dxl
  Dxu
  Dyl
  Dyu
  Dzl
  Dzu
  Nt
  dt
  start_tind
  sourcemode
  runmode
  exphasorsvolume
  exphasorssurface
  intphasorsurface
  phasorsurface
  phasorinc
  dimension
  conductive_aux
  dispersive_aux
  structure
  tdfield

  fdtdgrid - A structure with the following members, each of which is a 3 dimensional
  array:

  fdtdgrid.Exy       (double)
  fdtdgrid.Exz
  fdtdgrid.Eyx
  fdtdgrid.Eyz
  fdtdgrid.Ezx
  fdtdgrid.Ezy
  fdtdgrid.Hxy
  fdtdgrid.Hxz
  fdtdgrid.Hyx
  fdtdgrid.Hyz
  fdtdgrid.Hzx
  fdtdgrid.Hzy

  fdtdgrid.materials (uint8)

  Cmaterial - A structure with the following members, each of which is a 1 dimensional
  array indexed by indices od scattering materials:

  Cmaterial.Cax      (double)
  Cmaterial.Cay
  Cmaterial.Caz
  Cmaterial.Cbx
  Cmaterial.Cby
  Cmaterial.Cbz
  Cmaterial.Ccx
  Cmaterial.Ccy
  Cmaterial.Ccz

  Dmaterial - A structure with the following members, each of which is a 1 dimensional
  array indexed by indices od scattering materials:

  Dmaterial.Cax      (double)
  Dmaterial.Cay
  Dmaterial.Caz
  Dmaterial.Cbx
  Dmaterial.Cby
  Dmaterial.Cbz

  C - A structure with the same elements as Cmaterial whose elements relate to the background
  region. Elements are not restricted to 1 dimension.

  D - A structure with the same elements as Cmaterial whose elements relate to the background
  region. Elements are not restricted to 1 dimension.

  freespace - A structure with the following members each of which is a double

  freespace.Cbx      (double)
  freespace.Cby
  freespace.Cbz
  freespace.Dbx
  freespace.Dby
  freespace.Dbz

  disp_params - A structure with the following members:

  disp_params.alpha      (double)
  disp_params.beta
  disp_params.gamma

  delta - A structure with the following elements representing Yee cell dimension

  delta.x (double)
  delta.y
  delta.z

  interface - A structure with the following members each of which is a double

  interface.I0 - [I0 apply]
  interface.I1 - [I1 apply]
  interface.J0 - [J0 apply]
  interface.J1 - [J1 apply]
  interface.K0 - [K0 apply]
  interface.K1 - [K1 apply]

  where, for example, I0 is the position of the interface plane between
  total and scattered formulations. apply is a boolean which indicates
  whether or not to apply the boundary condition

  Isource - A 3d matrix of dimension 8x(J1-J0+1)x(K1-K0+1) which contains
  complex amplitude information for the interface equations at the I0 and I1
  planes. Should be Isource(1,:,:) - Ey, Isource(2,:,:) - Ez, Isource(3,:,:) - Hy,
  Isource(4,:,:) - Hz,Isource(5,:,:) - Ey, Isource(6,:,:) - Ez, Isource(7,:,:) - Hy,
  Isource(8,:,:) - Hz

  Jsource - A 3d matrix of dimension 8x(I1-I0+1)x(K1-K0+1) which contains
  complex amplitude information for the interface equations at the J0 and J1
  planes. Should be Jsource(1,:,:) - Ex, Jsource(2,:,:) - Ez, Jsource(3,:,:) - Hx,
  Jsource(4,:,:) - Hz,Jsource(5,:,:) - Ex, Jsource(6,:,:) - Ez, Jsource(7,:,:) - Hx,
  Jsource(8,:,:) - Hz

  Ksource - A 3d matrix of dimension 8x(I1-I0+1)x(J1-J0+1) which contains
  complex amplitude information for the interface equations at the K0 and K1
  planes. Should be Ksource(1,:,:) - Ex, Ksource(2,:,:) - Ey, Ksource(3,:,:) - Hx,
  Ksource(4,:,:) - Hy,Ksource(5,:,:) - Ex, Ksource(6,:,:) - Ey, Ksource(7,:,:) - Hx,
  Ksource(8,:,:) - Hy

  grid_labels - A structure with 3 elements, represents the axis labels:
  x_grid_label - cartesian labels of Yee cell origins (x-coordinates)
  y_grid_label - cartesian labels of Yee cell origins (y-coordinates)
  z_grid_label - cartesian labels of Yee cell origins (z-coordinates)

  omega_an - analytic angular frequency source

  to_l - time delay of pulse

  hwhm - hwhm of pulse

  Dxl - thickness of upper pml in the x direction
  Dxu - thickness of upper pml in the x direction
  Dyl - thickness of upper pml in the y direction
  Dyu - thickness of upper pml in the y direction
  Dzl - thickness of upper pml in the z direction
  Dzu - thickness of upper pml in the z direction

  lower_boundary_update - boolean for applying the lower update equations

  Nt - the number of iterations

  start_tind - the starting value of the tind index which controls the current time
  step of each fdtd iteration. The default for this would be 0;

  sourcemode - integer of value 0 for s steady state simulation and value 1 for a pulse simulation

  runmode - integer of value 0 for "complete" and 1 for "analyse"

  exphasorsvolume - Extract phasors in the FDTD volume if set to true

  exphasorssurface - Extract phasors about a specified cuboid surface if set to true

  intphasorssurface - Interpolate surface phasors onto a common point if true

  phasorsurface - A list of indices defining the cuboid to extract the phasors at

  phasorinc - An integer vector of three elements describing the factor by which to reduce the
  density of vertices in the enclosing observation mesh

  dimension - A string of value "3", "TE" or "TM"

  conductive_aux - auxiliary parameters required to model conductive multilayer

  dispersive_aux - auxiliary parameters required to model dispersive multilayer

  structure - 2 x (I_tot+1) integer array describing the grating structure, if one is present

  f_ex_vec - 1xN or Nx1 vector of frequencies at which to perform the extraction of complex amplitudes

  tdfield - structure containing elements exi and eyi which have dimension (I1-I0+1)x(J1-J0+1)xNt

  fieldsample.i - indices along the x-direction of locations at which to sample the field
  fieldsample.j - indices along the y-direction of locations at which to sample the field
  fieldsample.k - indices along the z-direction of locations at which to sample the field
  fieldsample.n - vector of the moments of the field to sample

  campssample.vertices - N x 3 matrix of indices where the complex amplitudes will be sampled
  campssample.components - numerical array of up to six elements which defines which field components
                           will be sampled, 1 means Ex, 2 Ey etc.
*/
void execute_simulation(int nlhs, mxArray *plhs[], int nrhs, InputMatrices in_matrices,
                        SolverMethod solver_method,
                        PreferredInterpolationMethods preferred_interpolation_methods);
