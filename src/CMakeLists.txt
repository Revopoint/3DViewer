cmake_minimum_required(VERSION 3.10)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

project(3DViewer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## qt settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

# set Qt5_ROOT in system environment variables
if(MSVC)
    cmake_policy(SET CMP0074 NEW)
endif()
find_package(Qt5 
    COMPONENTS 
    Widgets
    OpenGL
    LinguistTools
    REQUIRED)
    
if(Qt5_VERSION VERSION_LESS 5.10)
    message(FATAL_ERROR "Minimum supported Qt5 version is 5.10!")
endif()

set(LUPDATE ${_qt5_linguisttools_install_prefix}/bin/lupdate)
set(LUPDATE_OPTIONS -locations absolute -no-ui-lines -no-sort)
set(LRELEASE ${_qt5_linguisttools_install_prefix}/bin/lrelease)

# OpenMP
find_package(OpenMP REQUIRED)

## thirdparty
set(THIRDPARTY_DIR ${PROJECT_SOURCE_DIR}/../thirdparty)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(status "current platform: linux ")
    set(PLATFORM "linux")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(status "current platform: windows")
    set(PLATFORM "windows")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(status "current platform: mac")
    set(PLATFORM "mac")
else()
    message(status "other platform: ${CMAKE_SYSTEM_NAME}")
endif()

set(PLATFORM_BITS x64)

## 3DCamera
set(CAMERA_SDK_INC ${THIRDPARTY_DIR}/3DCamera/include)
set(CAMERA_SDK_LIB ${THIRDPARTY_DIR}/3DCamera/${PLATFORM}/${PLATFORM_BITS})

## OSG
set(OSG_LIB ${THIRDPARTY_DIR}/osg3.6.5/${PLATFORM}/lib)
set(OSG_INC ${THIRDPARTY_DIR}/osg3.6.5/${PLATFORM}/include)
set(OSG_BIN ${THIRDPARTY_DIR}/osg3.6.5/${PLATFORM}/bin)

## yaml-cpp
set(YAML_CPP_INC ${THIRDPARTY_DIR}/yaml-cpp0.6.0/include)
set(YAML_CPP_LIB ${THIRDPARTY_DIR}/yaml-cpp0.6.0/${PLATFORM}/lib)
set(YAML_CPP_BIN ${THIRDPARTY_DIR}/yaml-cpp0.6.0/${PLATFORM}/bin)

## libpng
set(LIBPNG_CPP_INC ${THIRDPARTY_DIR}/libpng/include)
set(LIBPNG_CPP_LIB ${THIRDPARTY_DIR}/libpng/${PLATFORM}/lib)
set(LIBPNG_CPP_BIN ${THIRDPARTY_DIR}/libpng/${PLATFORM}/bin)

## quazip
set(QUAZIP_CPP_INC ${THIRDPARTY_DIR}/quazip/include/quazip)
set(QUAZIP_CPP_LIB ${THIRDPARTY_DIR}/quazip/${PLATFORM}/lib)
set(QUAZIP_CPP_BIN ${THIRDPARTY_DIR}/quazip/${PLATFORM}/bin)

add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

## output settings
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_DIR ${CMAKE_BINARY_DIR}/lib)

# version
if("${APP_VERSION}" STREQUAL "")
    set(APP_VERSION "1.0.0-test")
endif()

if("${APP_NAME}" STREQUAL "")
    set(APP_NAME "3DViewer")
endif()

add_subdirectory(csutil)
add_subdirectory(cscamera)
add_subdirectory(csviewer)

set_target_properties(csutil cscamera PROPERTIES FOLDER libs)

if(MSVC)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${APP_NAME})
endif()

