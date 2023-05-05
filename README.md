<!-- \cond
 -->
<!-- 👆 this comment and the endcond below, tells doxygen to ignore the badges
and title at the top of README.md when building the project page (the title
would be duplicated) everything else in README.md is also the project homepage. -->

# TDMS · [![latest release](https://badgen.net/github/release/UCL/TDMS)](https://github.com/UCL/TDMS/releases)  [![license](https://badgen.net/github/license/UCL/TDMS)](https://github.com/UCL/TDMS/blob/main/LICENSE) [![Build and test](https://github.com/UCL/TDMS/actions/workflows/ci.yml/badge.svg)](https://github.com/UCL/TDMS/actions/workflows/ci.yml) [![MATLAB tests](https://github.com/UCL/TDMS/actions/workflows/matlab_tests.yml/badge.svg)](https://github.com/UCL/TDMS/actions/workflows/matlab_tests.yml) [![codecov](https://codecov.io/gh/UCL/TDMS/branch/main/graph/badge.svg?token=3kqP14kslL)](https://codecov.io/gh/UCL/TDMS)

> **Warning**
> This repository is a _work in progress_. The API will change without notice

<!-- \endcond -->

# Time-Domain Maxwell Solver

TDMS, the Time Domain Maxwell Solver, is a hybrid C++ and MATLAB tool for simulating light propagation through a medium by solving Maxwell's equations.
For further details about the method, please refer to the [PDF documentation](https://github.com/UCL/TDMS/blob/gh-doc/masterdoc.pdf).

![The normed z-component of the H field incident on a cylinder](doc/assets/HzNormBanner.png)

## Getting started

To use TDMS, it needs to be built against [FFTW](https://www.fftw.org/) and [MATLAB](https://www.mathworks.com/products/matlab.html), which must be downloaded and installed first.
To install, follow these steps:

```bash
$ git clone git@github.com:UCL/TDMS.git
$ cd TDMS
$ git checkout v1.0.0 # the stable version
$ mkdir build; cd build
$ cmake ../tdms \
# -DMatlab_ROOT_DIR=/usr/local/MATLAB/R2019b/ \
# -DFFTW_ROOT=/usr/local/fftw3/ \
# -DCMAKE_INSTALL_PREFIX=$HOME/.local/
$ make install
```

If CMake cannot find MATLAB, FFTW, or install to the default installation prefix, uncomment the relevant line(s) and modify the path(s) accordingly.

<details>
<summary>Mac-specific instructions</summary>

To compile TDMS on a Mac, you will need an x86 compiler with libraries for OpenMP.
You can install these using [Homebrew](https://brew.sh) with the command:

```{sh}
brew install llvm
```

After installing with Homebrew, you may need to set the following CMake arguments:

```{sh}
-DCMAKE_CXX_COMPILER=/Users/username/.local/homebrew/opt/llvm/bin/clang++
-DOMP_ROOT=/Users/username/.local/homebrew/opt/llvm/
-DCXX_ROOT=/Users/username/.local/homebrew/opt/llvm
```

On an ARM Mac, you will need to install the x86 version of Homebrew.
To do so, use the following commands:

```{sh}
arch -x86_64 zsh
arch -x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
arch -x86_64 /usr/local/bin/brew install llvm
```
</details>

You can check that `tdms` was installed correctly and is in your `PATH` by running:
```{sh}
tdms --help
```
in a new terminal.

## How to run

You can run TDMS either directly or from a MATLAB script.
For beginners, we recommend starting with the demonstration MATLAB script, which you can find in the `examples/arc_01` directory.
Move into this directory, launch MATLAB, and run the MATLAB script [`run_pstd_bscan.m`](https://github.com/UCL/TDMS/blob/main/examples/arc_01/run_pstd_bscan.m).
This script will generate the input to TDMS, run TDMS, and display sample output.

### On the command line

If you want to run TDMS standalone at the command line, the basic operation is with two arguments: an input file containing simulation parameters, and an output file name.
You can choose between two solver methods (finite-difference or pseudo-spectral) and two interpolation methods (cubic or band-limited) by setting the "flag" variables in the input file.
TDMS will check whether the input file has a dataset that matches the name of these flags when it reads the inputs.
At present, there are two flags that can be set in the input file:
<details>
<summary> `use_pstd` </summary>
- If not provided, or provided as `false`, then the default timestepping method of finite-differences (FDTD) will be used.
- If present and set to `true`, then `tdms` will use the pseudo-spectral (PSTD) method when performing simulation timesteps.
</details>
<details>
<summary> `use_bli` </summary>
- If not provided, or provided as `false`, then the default interpolation method of cubic interpolation will be used to obtain field values at the centres of Yee cells.
- If present and set to `true`, then `tdms` will use band-limited interpolation when obtaining field values at Yee cell centres.

\note Typically band-limited interpolation is superior to cubic interpolation when the extent of the Yee cell is of approximately the same order as, but slightly less than, one-sixth of the shortest wavelength of interest.
Otherwise, cubic interpolation typically enjoys superior accuracy.
</details>

### Parallelism

TDMS is parallelised with [OpenMP](https://en.wikipedia.org/wiki/OpenMP). The
maximum number of threads can be set with the `OMP_NUM_THREADS` environment
variable.  For example, to use 4 threads, in a bash shell, use:

```bash
$ export OMP_NUM_THREADS=4
```

</details>

## Citation

If you used TDMS in your research and found it helpful, please cite this work.
<!-- [zenodo/FIXME](https://zenodo.org/) -->

<!-- If you use TDMS in your work and have examples that you would like to share with other users, please get in touch with us at -->
<!-- [contact_address)[mailto:FIXME] -->
<details>
<summary>BibTEX</summary>

```bibtex
@software{tdms,
    author  = {Munro, Peter and others},
    license = {GPL-3.0},
    title   = {{TDMS - The Time-Domain Maxwell Solver}},
    URL     = {https://github.com/UCL/TDMS}
}
```

</details>
<details>
<summary>LaTeX</summary>

```tex
\bibitem{tdms}
P. Munro, et al \emph{TDMS - The Time-Domain Maxwell Solver}, \url{https://github.com/UCL/TDMS}.
```

</details>

## Acknowledgements

The TDMS source code was released under a GPL-3.0 License as part of a joint project between University College London's [Medical Physics and Biomedical Engineering](https://ucl.ac.uk/medphys) and [Centre for Advanced Research Computing](https://ucl.ac.uk/arc) with generous funding from the [Royal Society](https://royalsociety.org).

![medphys](doc/assets/biomedlogo.png)&nbsp;![arc](doc/assets/arclogo.png)

Development of this software has previously benefited from funding from the [Commonwealth Scholarships Commission](https://cscuk.fcdo.gov.uk/about-us/scholarships-and-fellowships/), the [Engineering and Physical Sciences Research Council](https://www.ukri.org/councils/epsrc/), and the [Australian Research Council](https://www.arc.gov.au/).

## Want to contribute?

We're grateful for bug reports, feature requests, and pull requests. Please see our [contribution guidelines](https://github-pages.ucl.ac.uk/TDMS/md__c_o_n_t_r_i_b_u_t_i_n_g.html) (we also have some [developer documentation](https://github-pages.ucl.ac.uk/TDMS/md_doc_developers.html)).
