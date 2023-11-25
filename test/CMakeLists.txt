cmake_minimum_required(VERSION 3.13)
project(test_runner LANGUAGES CXX)

include(GoogleTest)

add_subdirectory(googletest)
mark_as_advanced(
  BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
  gmock_build_tests gtest_build_samples gtest_build_tests
  gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

include_directories("${PROJECT_SOURCE_DIR}/../new-include")

file(GLOB_RECURSE gameengine_SRC "${PROJECT_SOURCE_DIR}/../src/build-order/**.cpp")
file(GLOB_RECURSE tests_SRC "${PROJECT_SOURCE_DIR}/build-order/**.t.cpp")

add_executable(test_runner ${tests_SRC} ${gameengine_SRC})
target_link_libraries(test_runner
  gtest
  gtest_main
  m
  SDL2
  stdc++
  stdc++fs
  GL
  GLU
  glut
)

gtest_add_tests(
  TARGET test_runner
  SOURCE ${tests_SRC})
