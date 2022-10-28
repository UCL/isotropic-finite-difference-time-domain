/**
 * @file arrays.h
 * @brief Classes describing arrays, vertices etc.
 */
#pragma once

#include <complex>
#include <string>
#include <stdexcept>

#include <fftw3.h>

#include "matlabio.h"
#include "utils.h"
#include "globals.h"

template<typename T>
class XYZTensor3D {
public:
  T ***x = nullptr;
  T ***y = nullptr;
  T ***z = nullptr;

  T*** operator[] (char c) const{
    switch (c) {
      case 'x': return x;
      case 'y': return y;
      case 'z': return z;
      default: throw std::runtime_error("Have no element " + to_string(c));
    }
  }
  T*** operator[] (AxialDirection d) const{
    switch (d) {
      case AxialDirection::X: return x;
      case AxialDirection::Y: return y;
      case AxialDirection::Z: return z;
      default: throw std::runtime_error("Have no element " + to_string(d));
    }
  }

  /**
   * @brief Allocates x, y, and z as (K_total+1) * (J_total+1) * (I_total+1) arrays
   *
   * @param I_total,J_total,K_total Dimensions of the tensor size to set
   */
  void allocate(int I_total, int J_total, int K_total) {
    x = (T ***) malloc(K_total * sizeof(T **));
    y = (T ***) malloc(K_total * sizeof(T **));
    z = (T ***) malloc(K_total * sizeof(T **));
    for (int k = 0; k < K_total; k++) {
      x[k] = (T **) malloc(J_total * sizeof(T *));
      y[k] = (T **) malloc(J_total * sizeof(T *));
      z[k] = (T **) malloc(J_total * sizeof(T *));
      for (int j = 0; j < J_total; j++) {
        x[k][j] = (T *) malloc(I_total * sizeof(T));
        y[k][j] = (T *) malloc(I_total * sizeof(T));
        z[k][j] = (T *) malloc(I_total * sizeof(T));
      }
    }
  }
};

class XYZVectors {
public:
  double* x = nullptr;
  double* y = nullptr;
  double* z = nullptr;

  /**
   * Default constructor
   */
  XYZVectors() = default;

  /**
   * Set the pointer for one of the vectors in this collection with a name of c
   * @param c Character labeling the vector
   * @param ptr Pointer to assign
   */
  void set_ptr(char c, double* ptr);
  /**
   * Set the pointer for one of the vectors in this collection with a name of c
   * @param d AxialDirection labeling the vector
   * @param ptr Pointer to assign
   */
  void set_ptr(AxialDirection d, double *ptr);
};

// TODO: docstring
class MaterialCollection{
protected:
  static void init_xyz_vectors(const mxArray *ptr, XYZVectors &arrays, const std::string &prefix);
};

class CCollectionBase {
public:
  XYZVectors a;
  XYZVectors b;
  XYZVectors c;
};

// TODO: docstring
class CCollection : public CCollectionBase {
private:
  void init_xyz_vectors(const mxArray *ptr, XYZVectors &arrays, const std::string &prefix);

public:
  bool is_multilayer = false;
  bool is_disp_ml = false;

  explicit CCollection(const mxArray *ptr);
};

// TODO: docstring
class CMaterial : public CCollectionBase, MaterialCollection {
public:
  explicit CMaterial(const mxArray *ptr);
};

class DCollectionBase {
public:
  XYZVectors a;
  XYZVectors b;
};

// TODO: docstring
class DCollection: public DCollectionBase{
private:
  static void init_xyz_vectors(const mxArray *ptr, XYZVectors &arrays, const std::string &prefix);

public:
  explicit DCollection(const mxArray *ptr);
};

// TODO: docstring
class DMaterial : public DCollectionBase, MaterialCollection {
public:
  explicit DMaterial(const mxArray *ptr);
};

class DispersiveMultiLayer{
public:
  double* alpha = nullptr;
  double* beta = nullptr;
  double* gamma = nullptr;
  XYZVectors kappa;
  XYZVectors sigma;

public:
  explicit DispersiveMultiLayer(const mxArray *ptr);
};

template<typename T>
class Matrix{
protected:
  int n_rows = 0;
  int n_cols = 0;
  T** matrix = nullptr;

public:
  inline T* operator[] (int value) const { return matrix[value]; }

  bool has_elements(){ return matrix != nullptr; };

  /**
   * Allocate the memory for this matrix. Must be defined in the header
   * @param n_rows Number of rows
   * @param n_cols Number of columns
   */
  void allocate(int n_rows, int n_cols){
    this->n_rows = n_rows;
    this->n_cols = n_cols;

    matrix = (T **) malloc(sizeof(T *) * n_rows);
    for (int i = 0; i < n_rows; i++){
      matrix[i] = (T *) malloc(sizeof(T) * n_cols);
    }
  };

  /**
   * Destructor. Must be defined in the header
   */
  ~Matrix(){
    if (has_elements()) {
      for (int i = 0; i < n_rows; i++) { free(matrix[i]); }
      free(matrix);
    }
  };
};

class GratingStructure: public Matrix<int>{

public:
  GratingStructure(const mxArray *ptr, int I_tot);

  ~GratingStructure();
};

template<typename T>
class Vector{
protected:
  int n = 0;        // Number of elements
  T* vector = nullptr; // Internal array

public:
  Vector() = default;

  explicit Vector(const mxArray *ptr);

  inline T operator[] (int value) const { return vector[value]; };

  inline int size() const { return n; };
};

class FrequencyExtractVector: public Vector<double>{
public:
  FrequencyExtractVector(const mxArray *ptr, double omega_an);

  double max();
};

class FrequencyVectors{
public:
  Vector<double> x;
  Vector<double> y;

  void initialise(const mxArray *ptr);
};

// TODO: docstring
class Pupil: public Matrix<double>{
public:
  Pupil() = default;

  void initialise(const mxArray *ptr, int n_rows, int n_cols);

  ~Pupil();
};

template<typename T>
class Tensor3D{
protected:
  int n_layers = 0;
  int n_cols = 0;
  int n_rows = 0;
  T*** tensor = nullptr;

public:
  bool is_matlab_initialised = false;

  Tensor3D() = default;

  Tensor3D(T*** tensor, int n_layers, int n_cols, int n_rows) {
    initialise(tensor, n_layers, n_cols, n_rows);
  }

  void initialise(T*** _tensor, int _n_layers, int _n_cols, int _n_rows) {
    tensor = _tensor;
    n_layers = _n_layers;
    n_cols = _n_cols;
    n_rows = _n_rows;
  }

  inline T** operator[] (int value) const { return tensor[value]; };

  bool has_elements(){ return tensor != nullptr; };

  void zero() {
    for (int k = 0; k < n_layers; k++)
      for (int j = 0; j < n_cols; j++)
        for (int i = 0; i < n_rows; i++) { tensor[k][j][i] = 0; }
  }

  void allocate(int nK, int nJ, int nI){
    n_layers = nK, n_cols = nJ, n_rows = nI;
    tensor = (T ***)malloc(n_layers * sizeof(T **));

    for(int k=0; k < n_layers; k++){
      tensor[k] = (T **)malloc(n_cols * sizeof(T *));
    }

    for(int k=0; k < n_layers; k++){
      for(int j=0; j < n_cols; j++){
        tensor[k][j] = (T *)malloc(n_rows * sizeof(T));
      }
    }
  };

  /**
   * @brief Computes the Frobenius norm of the tensor
   *
   * fro_norm = \f$\sqrt{ \sum_{i=0}^{I_tot}\sum_{j=0}^{J_tot}\sum_{k=0}^{K_tot} |t[k][j][i]|^2 }\f$
   */
  double frobenius() {
    T norm_val = 0;
    for (int i1 = 0; i1 < n_layers; i1++) {
      for (int i2 = 0; i2 < n_cols; i2++) {
        for (int i3 = 0; i3 < n_rows; i3++) { norm_val += abs(tensor[i1][i2][i3]) * abs(tensor[i1][i2][i3]); }
      }
    }
    return sqrt(norm_val);
  }

  ~Tensor3D(){
    if (tensor == nullptr) return;
    if (is_matlab_initialised){
      free_cast_matlab_3D_array(tensor, n_layers);
    } else {
      for (int k = 0; k < n_layers; k++) {
        for (int j = 0; j < n_cols; j++) {
          free(tensor[k][j]);
        }
        free(tensor[k]);
      }
      free(tensor);
    }
  }
};

class DTilde{
protected:
  int n_det_modes = 0;
  static void set_component(Tensor3D<std::complex<double>> &tensor, const mxArray *ptr,
                     const std::string &name, int n_rows, int n_cols);
public:
  inline int num_det_modes() const { return n_det_modes; };

  Tensor3D<std::complex<double>> x;
  Tensor3D<std::complex<double>> y;

  void initialise(const mxArray *ptr, int n_rows, int n_cols);
};

class IncidentField{
protected:
  void set_component(Tensor3D<double> &component, const mxArray *ptr, const std::string &name);

public:
  Tensor3D<double> x;
  Tensor3D<double> y;

  explicit IncidentField(const mxArray *ptr);
};

class FieldSample{

private:
  double**** tensor = nullptr;

public:
  mxArray* mx;       // Matlab array

  Vector<int> i;     // Indices along the x-direction of locations at which to sample the field
  Vector<int> j;     // Indices along the y-direction of locations at which to sample the field
  Vector<int> k;     // Indices along the z-direction of locations at which to sample the field
  Vector<double> n;  // Vector of the moments of the field to sample

  explicit FieldSample(const mxArray *ptr);

  bool all_vectors_are_non_empty() const{
          return i.size() > 0 && j.size() > 0 && k.size() > 0 && n.size() > 0;
  };

  inline double*** operator[] (int value) const { return tensor[value]; };

  ~FieldSample();
};

/**
 * List of field components as integers
 */
class FieldComponentsVector: public Vector<int>{
public:

  FieldComponentsVector() = default;

  void initialise(const mxArray *ptr);

  /**
   * Get the index of a particular integer in this vector. If it does not exist
   * then return -1. Returns the first occurrence.
   * @param value value to find in the vector
   * @return index or -1
   */
  int index(int value);
};

class Vertices: public Matrix<int>{
public:

  Vertices() = default;

  void initialise(const mxArray *ptr);

  int n_vertices(){ return n_rows; }

  ~Vertices(){
    if (has_elements()){
      free_cast_matlab_2D_array(matrix);
    }
    matrix = nullptr;
  };
};

/**
 * Complex amplitude samples. Abbreviated to CAmpSample in MATLAB code
 */
class ComplexAmplitudeSample {

public:
  Vertices vertices;                 // N x 3 matrix of indices to sample
  FieldComponentsVector components;  //

  explicit ComplexAmplitudeSample(const mxArray *ptr);

  int n_vertices(){ return vertices.n_vertices(); }
};

class DetectorSensitivityArrays{
public:
  fftw_complex* v = nullptr;            // Flat fftw vector
  fftw_plan plan = nullptr;             // fftw plan for the setup
  std::complex<double>** cm = nullptr;  // Column major matrix

  void initialise(int n_rows, int n_cols);

  ~DetectorSensitivityArrays();
};

/**
 * Matrix of c coefficients. See the pdf documentation for their definition
 */
class CCoefficientMatrix: public Matrix<double>{};

/**
 * Temporary storage 'vector'
 */
class EHVec: public Matrix<fftw_complex>{
public:
  ~EHVec();
};
