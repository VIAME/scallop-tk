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
##
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
function( AddDownloadedFile URL OutputLoc MD5 )
  file( DOWNLOAD ${URL} ${OutputLoc} EXPECTED_MD5 ${MD5} )
endfunction()

# Install files contained in some data directory
function( AddDataDir DirName OutputDirname TargetExec )
  install( DIRECTORY ${DirName}/
    DESTINATION ${OutputDirname}
    FILES_MATCHING PATTERN "*" )

  add_custom_command(TARGET ${TargetExec} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/${DirName}
    ${CMAKE_BINARY_DIR}/${OutputDirname} )
endfunction()

# =================================================================
# Add All Tools and Data Directories
# =================================================================

if( VC_TOOLNAMES )

  AddTool( scallop_tk_detector ScallopDetector.cpp ScallopTK )

  if( ENABLE_CAFFE )
  
    AddDownloadedFile(
      https://github.com/mattdawkins/scallop-tk-models/raw/master/CNN/DefaultCNN.caffemodel 
      ${CMAKE_CURRENT_SOURCE_DIR}/Models/Classifiers/CNN/DefaultCNN.caffemodel
      5f184c3c55b1cd738e37245982ee4bc5 )

    AddDownloadedFile(
      https://github.com/mattdawkins/scallop-tk-models/raw/master/CNN/Seperator.caffemodel
      ${CMAKE_CURRENT_SOURCE_DIR}/Models/Classifiers/Combo/Seperator.caffemodel
      ae5fa68b0848fb56bd16598b8b53639d )
  
  endif()

  if( MODEL_INSTALL_DIR )
    AddDataDir( Models ${MODEL_INSTALL_DIR} ScallopDetector )
  else()
    AddDataDir( Models bin/scallop_tk_models scallop_tk_detector )
  endif()

else()

  AddTool( ScallopDetector ScallopDetector.cpp ScallopTK )

  if( ENABLE_CAFFE )
  
    AddDownloadedFile(
      https://github.com/mattdawkins/scallop-tk-models/raw/master/CNN/DefaultCNN.caffemodel
      ${CMAKE_CURRENT_SOURCE_DIR}/Models/Classifiers/CNN/DefaultCNN.caffemodel
      5f184c3c55b1cd738e37245982ee4bc5 )

    AddDownloadedFile(
      https://github.com/mattdawkins/scallop-tk-models/raw/master/CNN/Seperator.caffemodel
      ${CMAKE_CURRENT_SOURCE_DIR}/Models/Classifiers/Combo/Seperator.caffemodel
      ae5fa68b0848fb56bd16598b8b53639d )
  
  endif()

  if( MODEL_INSTALL_DIR )
    AddDataDir( Models ${MODEL_INSTALL_DIR} ScallopDetector )
  else()
    AddDataDir( Models bin/Models ScallopDetector )
  endif()

endif()
