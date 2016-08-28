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

  option( CAFFE_CPU_ONLY "Set to true if your caffe was built with CPU only" OFF )
  if( CAFFE_CPU_ONLY )
    add_definitions( -DCPU_ONLY )
  endif()
endif()

# Add options for other misc things
option( ENABLE_BENCHMARKING "Output timing statistics to a text file" OFF )

if( ENABLE_BENCHMARKING )
  add_definitions( -DENABLE_BENCHMARKING )
endif()

option( ENABLE_VISUAL_DEBUGGER "Enable visual debugger, requires OpenGL" OFF )

if( ENABLE_VISUAL_DEBUGGER )
  find_package( OpenGL REQUIRED )
  find_package( GLUT REQUIRED )

  add_definitions( -DENABLE_VISUAL_DEBUGGER )
  include_directories( ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} )
endif()

# Add subdirs to project
add_subdirectory( ScallopTK )

option( BUILD_TOOLS "Build and install ScallopTK binary tools" ON )
option( BUILD_TESTS "Build and install ScallopTK tests" ON )
option( VC_TOOLNAMES "Install ScallopTK tools with viper_case names" OFF )

if( BUILD_TOOLS )
  add_subdirectory( Tools )
endif()

# Add configure files for find_package
set( ScallopTK_EXPORT_FILE "${ScallopTK_BINARY_DIR}/ScallopTKConfig.cmake" )
configure_file(
  "${CMAKE_SOURCE_DIR}/CMake/ScallopTK-config-install.cmake.in"
  "${ScallopTK_EXPORT_FILE}"
)

# Set up the install export
set( ScallopTK_EXPORT_INSTALL_FILE "${ScallopTK_BINARY_DIR}/CMakeFiles/ScallopTKConfig.cmake" )
configure_file(
  "${CMAKE_SOURCE_DIR}/CMake/ScallopTK-config-install.cmake.in"
  "${ScallopTK_EXPORT_INSTALL_FILE}"
  @ONLY
)

# On windows include dependency dlls in installer
#if( WIN32 )
#  install( FILES ${Caffe_LIBRARIES} ${OpenCV_LIBS}
#    DESTINATION bin FILES_MATCHING PATTERN "*.dll" )
#endif()

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
