
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RimFlowDiagSolution.h
${CEE_CURRENT_LIST_DIR}RimFlowPlotCollection.h
${CEE_CURRENT_LIST_DIR}RimWellAllocationPlot.h
${CEE_CURRENT_LIST_DIR}RimTotalWellAllocationPlot.h
${CEE_CURRENT_LIST_DIR}RimWellFlowRateCurve.h
${CEE_CURRENT_LIST_DIR}RimWellAllocationPlotLegend.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RimFlowDiagSolution.cpp
${CEE_CURRENT_LIST_DIR}RimFlowPlotCollection.cpp
${CEE_CURRENT_LIST_DIR}RimWellAllocationPlot.cpp
${CEE_CURRENT_LIST_DIR}RimTotalWellAllocationPlot.cpp
${CEE_CURRENT_LIST_DIR}RimWellFlowRateCurve.cpp
${CEE_CURRENT_LIST_DIR}RimWellAllocationPlotLegend.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Flow" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
