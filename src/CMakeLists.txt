cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(VHACD)
option(NO_OPENCL "NO_OPENCL" OFF)
option(NO_OPENMP "NO_OPENMP" OFF)

message("NO_OPENCL " ${NO_OPENCL})
message("NO_OPENMP " ${NO_OPENMP})

option(VHACD_WITH_PYTHON "Build Python Bindings" ON)

#set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/output" CACHE PATH "project install prefix" FORCE)
set(CMAKE_COMMON_INC "${CMAKE_SOURCE_DIR}/cmake/cmake_common.cmake")

add_subdirectory ("${CMAKE_SOURCE_DIR}/VHACD_Lib")
add_subdirectory ("${CMAKE_SOURCE_DIR}/test")

if(VHACD_WITH_PYTHON)
  add_subdirectory ("${CMAKE_SOURCE_DIR}/ext")
	message(STATUS "Building python bindings.")
  add_subdirectory ("${CMAKE_SOURCE_DIR}/bindings/py_vhacd")
endif(VHACD_WITH_PYTHON)
