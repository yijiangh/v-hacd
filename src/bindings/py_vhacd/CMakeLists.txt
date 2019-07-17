# # needed for pybind11 to find eigen path
# include_directories(${EIGEN_INCLUDE_DIRS})

pybind11_add_module(py_vhacd
	py_vhacd.cpp
	)

# add_executable(testVHACD ${PROJECT_CPP_FILES} ${PROJECT_C_FILES} ${PROJECT_INC_FILES} ${PROJECT_INL_FILES})
target_include_directories(py_vhacd PRIVATE ${CMAKE_SOURCE_DIR}/VHACD_Lib/public ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(py_vhacd PRIVATE vhacd)

# this is needed for linking a static lib into a dynamic lib
# see:
# https://stackoverflow.com/questions/38296756/what-is-the-idiomatic-way-in-cmake-to-add-the-fpic-compiler-option
# https://github.com/pybind/pybind11/issues/466
set_target_properties(vhacd PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
