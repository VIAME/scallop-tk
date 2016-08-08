# Setup paths
macro( configure_find_paths PKG )

  set( ${PKG}_INC_FIND_PATH "${${PKG}_INCLUDE_DIR}" )

  if( EXISTS "${${PKG}_LIBRARY}" )
    get_filename_component( ${PKG}_LIB_SEARCH_DIR "${${PKG}_LIBRARY}" DIRECTORY )
    set( ${PKG}_LIB_FIND_PATH "${${PKG}_LIB_SEARCH_DIR}" )
  endif()

  set( ${PKG}_INCLUDE_DIR NOTFOUND )
  set( ${PKG}_LIBRARY NOTFOUND )

endmacro()
