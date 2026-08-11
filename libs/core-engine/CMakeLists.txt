cmake_minimum_required(VERSION 3.24...3.28)
project(core_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake >= 3.28 auto-enables C++20+ modules dependency scanning (P1689)
# with the Ninja generator once CXX_STANDARD >= 20 is requested. That
# scanner runs a separate preprocessing pass ahead of the normal compile
# and, in combination with FetchContent-provided interface targets, has
# been observed to drop this project's own include directories, causing
# a spurious "header not found" failure before any real compilation
# happens. This project does not use C++ modules, so disable the scan
# globally rather than debug scanner include propagation.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(CORE_ENGINE_BUILD_TESTS "Build GoogleTest unit tests" OFF)

add_compile_options(-Wall -Wextra -Werror -Wpedantic)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3 -DNDEBUG)
endif()

add_library(core_engine_headers INTERFACE)
target_include_directories(core_engine_headers INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(core_engine_headers INTERFACE cxx_std_26)

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
endif()
