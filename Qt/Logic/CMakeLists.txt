project(vtkSlicerEMSegmentModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#


# --------------------------------------------------------------------------
set(${PROJECT_NAME}_ITK_COMPONENTS
  ${EMSegment_BASE_ITK_COMPONENTS}
  )
find_package(ITK 4.6 NO_MODULE COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS ${ITKFactoryRegistration_INCLUDE_DIRS})
include(${ITK_USE_FILE})

# --------------------------------------------------------------------------
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
  ${ITK_LIBRARIES}
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
