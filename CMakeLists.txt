cmake_minimum_required(VERSION 3.13)
project(mo-boo LANGUAGES CXX VERSION 0.0.1)

option(RUN_TESTS "Build tests" ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/obj)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP)

add_definitions(
  -D_REENTRANT
  -msse4.2
  -mavx2
  -march=x86-64
  -malign-double
  -ftree-vectorize
  -ftree-vectorizer-verbose=2
)

include_directories("${PROJECT_SOURCE_DIR}/new-include")

if(RUN_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

file(GLOB_RECURSE moboo_SRC "${PROJECT_SOURCE_DIR}/src/**.cpp")

add_executable(mo-boo ${moboo_SRC})
target_compile_definitions(mo-boo PRIVATE MOBOO_SINGLE_THREADED)
target_link_libraries(mo-boo
  m
  stdc++
)

if (OPENMP_FOUND)
  add_executable(mo-boo-mp ${moboo_SRC})
  set_target_properties(mo-boo-mp PROPERTIES COMPILE_FLAGS ${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS})
  set_target_properties(mo-boo-mp PROPERTIES LINK_OPTIONS ${CMAKE_EXE_LINKER_FLAGS} -fopenmp)
  target_link_libraries(mo-boo-mp
    m
    stdc++
  )
endif()
