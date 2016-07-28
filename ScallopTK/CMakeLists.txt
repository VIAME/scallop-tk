
set( ScallopTK_Library_Source
  Utilities/sadsa.h
)

add_library( ScallopTK ${ScallopTK_Library_Source} )
target_link_libraries( ScallopTK ${OpenCV_LIBS} )

if( USE_CAFFE )
  target_link_libraries( ScallopTK ${Caffe_LIBRARIES} )
endif()

set_target_properties( ScallopTK PROPERTIES
  VERSION ${ScallopTK_VERSION} SOVERSION ${ScallopTK_VERSION}
)

install( TARGETS ScallopTK EXPORT ScallopTK
  RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib
)

install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/ScallopTK
  FILES_MATCHING PATTERN "*.h"
)
