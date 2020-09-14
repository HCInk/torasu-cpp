# TORASU CPP

[![language: C++](https://img.shields.io/badge/language-C%2B%2B-f34b7d)](https://en.wikipedia.org/wiki/C%2B%2B)
[![C++: 20](https://img.shields.io/badge/C%2B%2B-20-f34b7d)](https://en.cppreference.com/w/cpp/20)
[![GCC: gnu++2a](https://img.shields.io/badge/GCC-gnu%2B%2B2a-f34b7d)](https://www.gnu.org/software/gcc/projects/cxx-status.html#cxx2a)
[![build: cmake](https://img.shields.io/badge/build-cmake-89e051)](https://cmake.org/)
[![license: LGPLv3](https://img.shields.io/badge/license-LGPLv3-blue)](https://choosealicense.com/licenses/lgpl-3.0/)
[![pipeline status](https://ci.hcink.org/buildStatus/icon?job=torasu-cpp)](https://gitlab.com/hcink/torasu/torasu-cpp/pipelines)
[![](https://ci.hcink.org/buildStatus/icon?job=torasu-cpp&build=0&config=cloc)](https://gitlab.com/hcink/torasu/torasu-cpp/pipelines)

C++ Implementation of the TORASU Compute Framework

## Dev Dependencies

### Mandatory

- [nlohmann/json](https://github.com/nlohmann/json) (compile)
- [asan](https://github.com/google/sanitizers/wiki/AddressSanitizer) (address-sanatizing - will be optional soon)

### Optional

- [cpplint](https://github.com/google/styleguide) (linting - pip version recommended)
- [astyle](http://astyle.sourceforge.net/) (formatting)
- [Catch2](https://github.com/catchorg/Catch2) (testing)
