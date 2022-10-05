/**
 * @file globals.h
 * @brief Type definitions and global constants.
 */
#pragma once
#include <complex>

// ******************
//  Type Definitions
// ******************

typedef int*				IArray_1d;
typedef IArray_1d*			IArray_2d;
typedef IArray_2d*			IArray_3d;

typedef double*				DArray_1d;
typedef DArray_1d*			DArray_2d;
typedef DArray_2d*			DArray_3d;

typedef std::complex<double>*	CArray_1d;
typedef CArray_1d*			CArray_2d;
typedef CArray_2d*			CArray_3d;

typedef struct PlanarInterface	// Structure definition for a planar six-face interface 
{
	int I1;
	int I2;
	int J1;
	int J2;
	int K1;
	int K2;
} PlanarInterface;

typedef struct complex_vector
{
	std::complex<double> X;
	std::complex<double> Y;
	std::complex<double> Z;
} complex_vector;


// **********************
//  Enumerated constants
// **********************

enum ModeOfRun { Pass1 , Pass2 };
enum RCSType   { parallel , perpendicular };

// **************************************
//			Mathematical Constants
// **************************************

namespace tdms_math_constants
{
const double dcpi = 3.14159265358979323846;                                  // Pi
const std::complex<double> IMAGINARY_UNIT = std::complex<double>(0.0, 1.0);  // Imaginary unit
}

// **************************************
//			Physical Constants
// **************************************

namespace tdms_phys_constants
{
const double eo = 8.85400e-12;                               // free space electric permitivity
const double mo = 4.0 * tdms_math_constants::dcpi * 1.0e-7;  // free space magnetic permeability
const double light_v = 1.0 / sqrt(mo * eo);                  // free space light velocity
const double Zo = 376.734;                                   // free space inpedance
}

