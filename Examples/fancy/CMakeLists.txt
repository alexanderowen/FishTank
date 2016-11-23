cmake_minimum_required(VERSION 2.8.12.1)

PROJECT(fancy)
SET(VTK_DIR /Users/hank/Hartree/VTK/VTK6.0.0)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(fancy MACOSX_BUNDLE fancy)

if(VTK_LIBRARIES)
  target_link_libraries(fancy ${VTK_LIBRARIES})
else()
  target_link_libraries(fancy vtkHybrid)
endif()

