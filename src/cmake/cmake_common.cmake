message("[VHACD] Generating " ${PROJECT_NAME} "...")
file(GLOB PROJECT_INC_FILES "inc/*.h" "public/*.h")
file(GLOB PROJECT_INL_FILES "inc/*.inl")
file(GLOB PROJECT_CPP_FILES "src/*.cpp")
file(GLOB PROJECT_C_FILES "src/*.c")
file(GLOB PROJECT_CL_FILES "cl/*.cl")
source_group (Inc FILES ${PROJECT_INC_FILES})
source_group (Inl FILES ${PROJECT_INL_FILES})
source_group (Src FILES ${PROJECT_CPP_FILES}  )
source_group (SrcC FILES ${PROJECT_C_FILES}  )
source_group (CL FILES ${PROJECT_CL_FILES})

#include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
# message("[VHACD] \t INC_FILES: ${PROJECT_INC_FILES}")
# message("[VHACD] \t INL_FILES: ${PROJECT_INL_FILES}")
# message("[VHACD] \t CPP_FILES: ${PROJECT_CPP_FILES}")
# message("[VHACD] \t C_FILES:   ${PROJECT_C_FILES}")
# message("[VHACD] \t CL_FILES:   ${PROJECT_CL_FILES}")
