
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicAddEclipseInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicAddOpmInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicComputeStatisticsFeature.h
${CEE_CURRENT_LIST_DIR}RicCreateGridCaseGroupFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.h
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertFeature.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.h
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewFeature.h
${CEE_CURRENT_LIST_DIR}RicImportEclipseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicImportInputEclipseCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicImportInputEclipseCaseOpmFeature.h
${CEE_CURRENT_LIST_DIR}RicNewStatisticsCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseInputPropertyFeature.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.h
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicAddEclipseInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicAddOpmInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicComputeStatisticsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicCreateGridCaseGroupFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipseCaseNewGroupFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterInsertFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewExec.cpp
${CEE_CURRENT_LIST_DIR}RicEclipsePropertyFilterNewFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportEclipseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportInputEclipseCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicImportInputEclipseCaseOpmFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewStatisticsCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseInputPropertyFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyExec.cpp
${CEE_CURRENT_LIST_DIR}RicSaveEclipseResultAsInputPropertyFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Eclipse" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
