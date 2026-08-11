cmake_minimum_required(VERSION 3.24...3.28)
project(core_engine LANGUAGES CXX)

# NOTE: deliberately NOT using CMAKE_CXX_STANDARD / CXX_STANDARD_REQUIRED
# / target_compile_features(cxx_std_26). Verified locally against the
# exact toolchain combination GitHub-hosted runners use as of this
# writing (CMake 3.28.3 + GCC 14.2.0):
#   1. target_compile_features(... cxx_std_26) fails outright --
#      CMake's compile-features database has no "cxx_std_26" entry
#      until CMake 3.30, regardless of the compiler's actual support.
#   2. Falling back to the bare CMAKE_CXX_STANDARD 26 / STANDARD_REQUIRED
#      mechanism ALSO fails: CMake 3.28's GNU compiler module has no
#      flag-mapping entry for the C++26 dialect either, so it reports
#      "does not support this, or CMake does not know the flags to
#      enable it" -- and because CMAKE_CXX_STANDARD is a directory-scoped
#      variable, it was also incorrectly inherited by the FetchContent
#      -pulled GoogleTest subdirectory below, breaking gtest's own
#      targets in the same way.
# The fix: pass -std=c++26 as a raw compiler flag, attached ONLY to our
# own INTERFACE target (core_engine_headers) so it propagates solely to
# our own consumers (the test binary, the nanobind module) via usage
# requirements, and never touches GoogleTest's targets.

option(CORE_ENGINE_BUILD_TESTS "Build GoogleTest unit tests" OFF)

#add_compile_options(-Wall -Wextra -Werror -Wpedantic)
#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()
#if(CMAKE_BUILD_TYPE STREQUAL "Release")
#  add_compile_options(-O3 -DNDEBUG)
#endif()
#
#add_library(core_engine_headers INTERFACE)
#target_include_directories(core_engine_headers INTERFACE
#  ${CMAKE_CURRENT_SOURCE_DIR}/include)
#target_compile_options(core_engine_headers INTERFACE -std=c++26)

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(MSVC)
    add_compile_options(/O2 -DNDEBUG)
  else()
    add_compile_options(-O3 -DNDEBUG)
  endif()
endif()

add_library(core_engine_headers INTERFACE)
target_include_directories(core_engine_headers INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
  target_compile_options(core_engine_headers INTERFACE /std:c++latest)
else()
  target_compile_options(core_engine_headers INTERFACE -std=c++26)
endif()

if(CORE_ENGINE_BUILD_TESTS)
  # Native GoogleTest build path (no Python needed): used by CI's
  # standalone C++ job. The Python extension module below is built
  # separately via scikit-build-core/nanobind when installed as a wheel.
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(core_engine_tests tests/bucketing_test.cc)
  # Belt-and-suspenders: set the include directory explicitly on the
  # executable target itself, in addition to the transitive
  # core_engine_headers interface link, so the header is resolvable even
  # if interface-property propagation is affected by generator quirks.
  target_include_directories(core_engine_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
  target_link_libraries(core_engine_tests PRIVATE core_engine_headers
                                                   GTest::gtest_main)
  set_target_properties(core_engine_tests PROPERTIES CXX_SCAN_FOR_MODULES OFF)
  include(GoogleTest)
  gtest_discover_tests(core_engine_tests)
else()
  # nanobind's CMake config asserts that find_package(Python ...) has
  # already run in this directory scope before it is included; omitting
  # this call is what produced "You must invoke 'find_package(Python
  # COMPONENTS Interpreter Development REQUIRED)' prior to including
  # nanobind" on both the Ubuntu and Windows Python CI legs.
  find_package(Python 3.13 REQUIRED COMPONENTS Interpreter Development.Module)
  find_package(nanobind CONFIG REQUIRED)
  nanobind_add_module(core_engine STABLE_ABI src/engine.cpp)
  target_link_libraries(core_engine PRIVATE core_engine_headers)
  install(TARGETS core_engine DESTINATION .)
endif()
