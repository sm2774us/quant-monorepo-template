cmake_minimum_required(VERSION 3.24...3.28)
project(core_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
  target_link_libraries(core_engine_tests PRIVATE core_engine_headers
                                                   GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(core_engine_tests)
else()
  find_package(nanobind CONFIG REQUIRED)
  nanobind_add_module(core_engine STABLE_ABI src/engine.cpp)
  target_link_libraries(core_engine PRIVATE core_engine_headers)
endif()
