[![doc](https://img.shields.io/badge/PDF-latest-orange.svg?style=flat)](https://github.com/UCL/TDMS/blob/gh-doc/masterdoc.pdf)
# TDMS
_Time Domain Maxwell Solver_


***
## Introduction

TDMS is a hybrid C++ and MATLAB code to solve Maxwell's equations in the time 
domain.

> **Warning**
> This repository is a _work in progress_. The API will change without notice


***
## Compilation

TDMS requires building against [FFTW](https://www.fftw.org/) and 
[MATLAB](https://www.mathworks.com/products/matlab.html), thus both need to be
downloaded and installed prior to compiling TDMS. Install with

```bash
cd tdms
mkdir build; cd build
cmake .. \
# -DMatlab_ROOT_DIR=/usr/local/MATLAB/R2019b/ \
# -DFFTW_ROOT=/usr/local/fftw3/ \
# -DCMAKE_INSTALL_PREFIX=$HOME/.local/
make install
```
where lines need to be commented in and the paths modified if cmake cannot 
(1) find MATLAB, (2) find FFTW or (3) install to the default install prefix.

- <details>
    <summary>Mac specific instructions</summary>

    To compile on a Mac an x86 compiler with libraries for OpenMP are required,
    which can be installed using [brew](https://brew.sh/) with `brew install llvm` 
    then (optionally) set the following cmake arguments

    ```
    -DCMAKE_CXX_COMPILER=/Users/username/.local/homebrew/opt/llvm/bin/clang++
    -DOMP_ROOT=/Users/username/.local/homebrew/opt/llvm/
    -DCXX_ROOT=/Users/username/.local/homebrew/opt/llvm
    ```
  
    On an ARM Mac install the x86 version of brew with
    ```bash
    arch -x86_64 zsh  
    arch -x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    arch -x86_64 /usr/local/bin/brew install llvm
    ```
</details>


***
## Usage: Running the demonstration code

Once the executable has been compiled, move into directory:
tests/arc_01

launch Matlab and run the Matlab script:
run_pstd_bscan.m

This script will generate the input to the executable, run the
executable and display sample output.
