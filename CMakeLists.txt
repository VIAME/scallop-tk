project( ScallopTK )
cmake_minimum_required( VERSION 2.8 )

# Configure version number options
set( ScallopTK_MAJOR_VER 0 )
set( ScallopTK_MINOR_VER 9 )
set( ScallopTK_PATCH_VER 1 )
set( ScallopTK_VERSION "${ScallopTK_MAJOR_VER}.${ScallopTK_MINOR_VER}.${ScallopTK_PATCH_VER}" )

# Add build shared library option
option( BUILD_SHARED_LIBS "Build shared libraries." OFF )

# Configure build and install path
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH} )

set( EXECUTABLE_OUTPUT_PATH      ${CMAKE_BINARY_DIR}/bin )
if( WIN32 )
  set( LIBRARY_OUTPUT_PATH       ${CMAKE_BINARY_DIR}/bin )
else( WIN32 )
  set( LIBRARY_OUTPUT_PATH       ${CMAKE_BINARY_DIR}/lib )
endif( WIN32 )

if( WIN32 )
  set( EXECUTABLE_INSTALL_PATH   ${CMAKE_INSTALL_PREFIX}/bin )
  set( LIBRARY_INSTALL_PATH      ${CMAKE_INSTALL_PREFIX}/bin )
else( WIN32 )
  set( EXECUTABLE_INSTALL_PATH   ${CMAKE_INSTALL_PREFIX}/bin )
  set( LIBRARY_INSTALL_PATH      ${CMAKE_INSTALL_PREFIX}/lib )
endif (WIN32)
set( INCLUDE_INSTALL_PATH        ${CMAKE_INSTALL_PREFIX}/include )

# Find required dependencies and add necessary source paths
include_directories( SYSTEM ${CMAKE_SOURCE_DIR} )

find_package( OpenCV REQUIRED )
include_directories( SYSTEM ${OpenCV_INCLUDE_DIRS} )

if( OpenCV_VERSION VERSION_LESS "2.4" )
  message( FATAL_ERROR "OpenCV version must be at least 2.4" )
endif()

if( OpenCV_VERSION VERSION_GREATER "3.0" )
  message( FATAL_ERROR "OpenCV v3.0+ currently not supported" )
endif()

option( ENABLE_CAFFE "Build with Caffe support enabled" ON )

if( ENABLE_CAFFE )
  find_package( Caffe REQUIRED )
  add_definitions( -DUSE_CAFFE )
  include_directories( SYSTEM ${Caffe_INCLUDE_DIRS} )
endif()

# Add options for other misc things
option( ENABLE_FASTER_RCNN "Build with faster RCNN support enabled" OFF )

if( ENABLE_FASTER_RCNN )
  message( FATAL_ERROR "Faster R-CNN not supported in this build" )
endif()

option( ENABLE_BENCHMARKING "Output timing statistics to a text file" OFF )

if( ENABLE_BENCHMARKING )
  add_definitions( -DENABLE_BENCHMARKING )
endif()

option( ENABLE_VISUAL_DEBUGGER "Enable visual debugger, requires OpenGL" OFF )

if( ENABLE_VISUAL_DEBUGGER )
  add_definitions( -DENABLE_VISUAL_DEBUGGER )
endif()

# Add subdirs to project
add_subdirectory( Documentation )
add_subdirectory( ScallopTK )
add_subdirectory( Tools )

# Options to build a CPack driven installer package
set( CPACK_SYSTEM_NAME           "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" )
set( CPACK_PACKAGE_NAME          "Scallop-TK" )
set( CPACK_PACKAGE_VENDOR        "Multiple" )
set( CPACK_PACKAGE_CONTACT       "matthew.d.dawkins@gmail.com" )
set( CPACK_MONOLITHIC_INSTALL    true )
set( CPACK_PACKAGE_VERSION_MAJOR "${ScallopTK_MAJOR_VER}" )
set( CPACK_PACKAGE_VERSION_MINOR "${ScallopTK_MINOR_VER}" )
set( CPACK_PACKAGE_VERSION_PATCH "${ScallopTK_PATCH_VER}" )
set( CPACK_PACKAGE_VERSION       "${ScallopTK_VERSION}" )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt" )

include( CPack )
