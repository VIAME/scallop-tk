
set( ScallopTK_Library_Source

  Classifiers/AdaClassifier.h            Classifiers/AdaClassifier.cpp
  Classifiers/CNNClassifier.h            Classifiers/CNNClassifier.cpp
  Classifiers/TrainingUtils.h            Classifiers/TrainingUtils.cpp

  EdgeDetection/EdgeLinking.h            EdgeDetection/EdgeLinking.cpp
  EdgeDetection/ExpensiveSearch.h        EdgeDetection/ExpensiveSearch.cpp
  EdgeDetection/GaussianEdges.h          EdgeDetection/GaussianEdges.cpp
  EdgeDetection/StableSearch.h           EdgeDetection/StableSearch.cpp
  EdgeDetection/WatershedEdges.h         EdgeDetection/WatershedEdges.cpp

  FeatureExtraction/Clustering.h         FeatureExtraction/Clustering.cpp
  FeatureExtraction/ColorID.h            FeatureExtraction/ColorID.cpp
  FeatureExtraction/FFT.h                FeatureExtraction/FFT.cpp
  FeatureExtraction/Gabor.h              FeatureExtraction/Gabor.cpp
  FeatureExtraction/HoG.h                FeatureExtraction/HoG.cpp
  FeatureExtraction/ShapeID.h            FeatureExtraction/ShapeID.cpp
  FeatureExtraction/StemID.h             FeatureExtraction/StemID.cpp

  ObjectProposals/AdaptiveThresholding.h ObjectProposals/AdaptiveThresholding.cpp
  ObjectProposals/CannyPoints.h          ObjectProposals/CannyPoints.cpp
  ObjectProposals/Consolidator.h         ObjectProposals/Consolidator.cpp
  ObjectProposals/DoG.h                  ObjectProposals/DoG.cpp
  ObjectProposals/HistogramFiltering.h   ObjectProposals/HistogramFiltering.cpp
  ObjectProposals/PriorStatistics.h      ObjectProposals/PriorStatistics.cpp
  ObjectProposals/TemplateApproximator.h ObjectProposals/TemplateApproximator.cpp

  Pipelines/CoreDetector.h               Pipelines/CoreDetector.cpp

  ScaleDetection/ImageProperties.h       ScaleDetection/ImageProperties.cpp

  TPL/AdaBoost/BoostedCommittee.h        TPL/AdaBoost/BoostedCommittee.cpp
  TPL/AdaBoost/IWeakHypothesis.h
  TPL/AdaBoost/SPHypothesis.h            TPL/AdaBoost/SPHypothesis.cpp
  TPL/Config/ConvertUTF.h                TPL/Config/ConvertUTF.c
  TPL/Config/SimpleIni.h
  TPL/Homography/ScottCamera.h           TPL/Homography/ScottCamera.cpp
  TPL/KDTree/kdtree.h                    TPL/KDTree/kdtree.c

  Utilities/Benchmarking.h
  Utilities/ConfigParsing.h
  Utilities/Definitions.h
  Utilities/Display.cpp
  Utilities/Display.h
  Utilities/FilesystemUnix.h
  Utilities/FilesystemWin32.h
  Utilities/HelperFunctions.h            Utilities/HelperFunctions.cpp
  Utilities/Threads.h
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

install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)
