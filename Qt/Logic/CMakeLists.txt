project(vtkSlicerEMSegmentModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

SET(module_logic_name "${PROJECT_NAME}")

# The header '${PROJECT_NAME}Export.h' will be automatically configured.
SET(module_logic_export_directive "VTK_EMSEGMENT_EXPORT")

# Additional directories to include
SET(module_logic_include_directories
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_BINARY_DIR}
  )

# Source files
SET(module_logic_SRCS
  ${EMSegment_BASE_SRCS}
  )

# Additional Target libraries
SET(module_logic_target_libraries
  #EMSegmentMRML
  #EMSegmentRegistration
  vtkSlicerVolumesModuleLogic
  )

if(${VTK_VERSION_MAJOR} GREATER 5)
  list(APPEND VTK_WRAP_INCLUDE_DIRS ${EMSegment_SOURCE_DIR})
endif()

SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
