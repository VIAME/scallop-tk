# Locate the system installed Caffe
# The following variables will be set:
#
# Caffe_FOUND        - Set to true if Caffe can be found
# Caffe_INCLUDE_DIRS - The path to the Caffe header files
# Caffe_LIBRARIES    - The full path to the Caffe library

if( Caffe_DIR )

  find_package( Caffe NO_MODULE )

elseif( NOT Caffe_FOUND )

  find_path( Caffe_INCLUDE_DIR caffe/caffe.hpp ${Caffe_FIND_OPTS} )
  find_library( Caffe_LIBRARY caffe ${Caffe_FIND_OPTS} )

  include( FindPackageHandleStandardArgs )
  find_package_handle_standard_args( Caffe Caffe_INCLUDE_DIR Caffe_LIBRARY )
  mark_as_advanced( Caffe_INCLUDE_DIR Caffe_LIBRARY )

  set( Caffe_INCLUDE_DIRS ${Caffe_INCLUDE_DIR} )
  set( Caffe_LIBRARIES ${Caffe_LIBRARY} )

endif()
