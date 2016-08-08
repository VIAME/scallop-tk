# =================================================================
# Helper Function Definitions
# =================================================================

# Default command used to build and install a Scallop-TK tool.
#
# This command should be used by default, unless a non-standard
# CMake install option is required. This function automatically
# adds the custom binary prefix to the generated executable name in
# addition to building, linking, and installing the executable in
# a single CMake function call. Optionally, the third argument
# can contain any variable definitions to be passed to the tool.
#
# Example usages:
#
# AddTool( ExampleTool             <-- Binary name
#          ExampleTool.cpp         <-- Source file
#          ScallopTK )             <-- Linked libraries
#
function( AddTool BinName SourceFile InstallProp )
  add_executable( ${ScallopTK_BINARY_PREFIX}${BinName}
    ${SourceFile} )

  if( InstallProp MATCHES "-D*" )
    set_target_properties( ${ScallopTK_BINARY_PREFIX}${BinName}
      PROPERTIES COMPILE_FLAGS ${InstallProp} )

    target_link_libraries( ${ScallopTK_BINARY_PREFIX}${BinName}
      ${ARGN} )
  else()
    target_link_libraries( ${ScallopTK_BINARY_PREFIX}${BinName}
      ${InstallProp} ${ARGN} )
  endif()

  install( TARGETS ${ScallopTK_BINARY_PREFIX}${BinName}
    RUNTIME DESTINATION bin )
endfunction()

# Install files contained in some data directory
function( AddDataDir DirName )
  install( DIRECTORY ${DirName} ${ScallopTK_BINARY_PREFIX}
    DESTINATION bin FILES_MATCHING PATTERN "*" )
endfunction()

# =================================================================
# Add All Tools and Data Directories
# =================================================================

AddTool( ScallopDetector ScallopDetector.cpp ScallopTK )

AddDataDir( Models )

AddTool( FishOrNoFish FishOrNoFish.cpp ScallopTK )

