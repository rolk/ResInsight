
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicAddWellLogToPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellLogCurveExtractionFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellLogFileCurveFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellLogPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicNewWellLogPlotFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicNewWellLogPlotTrackFeature.h
${CEE_CURRENT_LIST_DIR}RicWellLogPlotCurveFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicWellLogsImportFileFeature.h
${CEE_CURRENT_LIST_DIR}RicDeleteWellLogPlotTrackFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicAddWellLogToPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellLogCurveExtractionFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellLogFileCurveFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellLogPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellLogPlotFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicNewWellLogPlotTrackFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellLogPlotCurveFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicWellLogsImportFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicDeleteWellLogPlotTrackFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\WellLog" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
