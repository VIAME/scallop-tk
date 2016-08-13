# Locate the system installed Caffe
# The following variables will be set:
#
# Caffe_FOUND        - Set to true if Caffe can be found
# Caffe_INCLUDE_DIRS - The path to the Caffe header files
# Caffe_LIBRARIES    - The full path to the Caffe library

set( Caffe_DIR "" CACHE PATH "Path to root caffe CMake build" )

if( Caffe_DIR )

  find_package( Caffe NO_MODULE )

elseif( NOT Caffe_FOUND )

  include( FindMacros )
  configure_find_paths( Caffe )

  find_path( Caffe_INCLUDE_DIR NAMES caffe/caffe.hpp HINTS ${Caffe_INC_FIND_PATH} )
  if( NOT EXISTS ${Caffe_INCLUDE_DIR} )
    message( FATAL_ERROR "Caffe include directory not valid, must contain caffe/caffe.hpp" )
  endif()

  find_library( Caffe_LIBRARY NAMES caffe HINTS ${Caffe_LIB_FIND_PATH} )
  if( NOT EXISTS ${Caffe_LIBRARY} )
    message( FATAL_ERROR "Caffe library path not valid, must contain caffe library .lib/.dll/.so/.a" )
  endif()

  include( FindPackageHandleStandardArgs )
  find_package_handle_standard_args( Caffe Caffe_INCLUDE_DIR Caffe_LIBRARY )

  set( Caffe_INCLUDE_DIRS ${Caffe_INCLUDE_DIR} )
  set( Caffe_LIBRARIES ${Caffe_LIBRARY} )

endif()
