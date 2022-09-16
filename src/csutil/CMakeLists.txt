# csutil
set(TARGET_NAME csutil)
include_directories(include)

file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_definitions(-DCS_UTILS_API)

#qt5_wrap_cpp(MOC_HDS ${HEADERS})
#source_group("Generated Files\\moc" FILES ${MOC_HDS})

add_library(${TARGET_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
    #${MOC_HDS}
)

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Widgets)

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIB_DIR})

    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIB_DIR})
else(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
        )
endif(WIN32)
