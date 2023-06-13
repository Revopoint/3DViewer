# 3DViewer
set(TARGET_NAME ${APP_NAME})
include_directories(include)
include_directories(include/cswidgets)
include_directories(../cscamera/include)
include_directories(../csutil/include)
include_directories(${CAMERA_SDK_INC} ${OSG_INC} ${PROJECT_BINARY_DIR})

# OSG_LIB
link_directories(${OSG_LIB})

if(WIN32)
    set(OSG_LIBRARY 
        optimized osg debug osgd
        optimized osgViewer debug osgViewerd
        optimized osgQOpenGL debug osgQOpenGLd
        optimized osgGA debug osgGAd
        optimized osgDB debug osgDBd
        optimized OpenThreads debug OpenThreadsd
        optimized osgText debug osgTextd         
    )
else(WIN32)
    set(OSG_LIBRARY 
        osg
        osgGA
        osgViewer
        osgText
        osgQOpenGL
        OpenThreads
    )
endif(WIN32)

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB FORMS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")

file(GLOB CSWIDGET_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/cswidgets/*.h")
file(GLOB CSWIDGET_SRC "${CMAKE_CURRENT_SOURCE_DIR}/cswidgets/*.cpp")

source_group("CSWidgets" FILES ${CSWIDGET_SRC} )
source_group("CSWidgets\\include" FILES ${CSWIDGET_INC} )
source_group("Form Files" FILES ${FORMS} )

qt5_wrap_ui(UI_HDS ${FORMS})
qt5_add_resources(QRC_FILE 3dviewer.qrc)
#qt5_wrap_cpp(MOC_HDS ${HEADERS})

# others
string(TIMESTAMP COMPILE_TIME "%Y/%m/%d %H:%M:%S")
set(APP_BUILD_TIME ${COMPILE_TIME})

set(APP_VERSION_H ${PROJECT_BINARY_DIR}/app_version.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app_version.h.in ${APP_VERSION_H})

# theme file
set(THEMES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/themes/global.css)

if(WIN32)
    file(GLOB RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/*.rc")
endif(WIN32)

source_group("Generated Files\\ui" FILES ${UI_HDS})
source_group("Generated Files" FILES ${QRC_FILE})
#source_group("Generated Files\\moc" FILES ${MOC_HDS})
source_group("Other Files" FILES ${THEMES_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/3dviewer.qrc ${RC_FILE})

if(APPLE)
    set(APP_ICNS ${CMAKE_CURRENT_SOURCE_DIR}/resources/3DViewer.icns)
    set_source_files_properties(${APP_ICNS}
        PROPERTIES MACOSX_PACKAGE_LOCATION Resources
    )
endif()

#set(CMAKE_AUTOUIC_SEARCH_PATH ${CMAKE_AUTOUIC_SEARCH_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(${TARGET_NAME} MACOSX_BUNDLE
    ${APP_ICNS}
    #${MOC_HDS}
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
    ${UI_HDS}
    ${RESOURCES}
    ${CSWIDGET_SRC}
    ${CSWIDGET_INC}
    ${THEMES_FILE}
    ${APP_VERSION_H}
    ${RC_FILE}
    ${QRC_FILE}
)

target_link_libraries(${TARGET_NAME} 
    PRIVATE
    ${OSG_LIBRARY}
    Qt5::Widgets
    Qt5::OpenGL
    csutil
    cscamera
    )

if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE 3DViewer.icns
        MACOSX_BUNDLE_BUNDLE_NAME ${TARGET_NAME}
    )
endif()

# translate
set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/lang_en.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/lang_zh.ts
)

execute_process(
    COMMAND ${LUPDATE} ${LUPDATE_OPTIONS} ${CMAKE_SOURCE_DIR} -ts ${TS_FILES}
    COMMAND ${LRELEASE} ${TS_FILES}
)

if(APPLE)
    set(CP_RES_DIR ${BIN_DIR}/${TARGET_NAME}.app/Contents/MacOS)
else()
    set(CP_RES_DIR ${BIN_DIR})
endif()

file(GLOB QM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.qm")
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/fonts "${CP_RES_DIR}/fonts/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/themes "${CP_RES_DIR}/themes/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CP_RES_DIR}/translations/"
    COMMAND ${CMAKE_COMMAND} -E copy ${QM_FILES} "${CP_RES_DIR}/translations/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CP_RES_DIR}/document/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/../document/manual "${CP_RES_DIR}/document/manual"
)

if(WIN32)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/resources/3DViewer.ico ${BIN_DIR}/
    )
endif(WIN32)

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${LIB_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_DIR})

    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${LIB_DIR}
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_DIR})
else()
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
    )
endif()

if (MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES VS_GLOBAL_LocalDebuggerEnvironment
        "PATH=${_qt5Core_install_prefix}/bin;${CAMERA_SDK_LIB};${YAML_CPP_BIN};${OSG_BIN};${LIBPNG_CPP_BIN};${QUAZIP_CPP_BIN};$(PATH)")

    set_target_properties(${TARGET_NAME} PROPERTIES
    LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif(MSVC)
