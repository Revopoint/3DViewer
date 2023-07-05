# cscamera
set(TARGET_NAME cscamera)

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# process 
file(GLOB PROCESS_INC "${CMAKE_CURRENT_SOURCE_DIR}/include/process/*.h")
file(GLOB PROCESS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/process/*.cpp")

source_group("Process" FILES ${PROCESS_SRC} )
source_group("Process\\include" FILES ${PROCESS_INC} )

include_directories(include)
include_directories(../csutil/include)
include_directories(${CAMERA_SDK_INC}) 
include_directories(${YAML_CPP_INC}) 
include_directories(${QUAZIP_CPP_INC})

link_directories(${CAMERA_SDK_LIB})
link_directories(${YAML_CPP_LIB})
link_directories(${QUAZIP_CPP_LIB})

add_definitions(-DCS_CAMERA_API)

if(WIN32)
    add_definitions(-DYAML_CPP_DLL)
endif()

#qt5_wrap_cpp(MOC_HDS ${HEADERS})
#source_group("Generated Files\\moc" FILES ${MOC_HDS})

add_library(${TARGET_NAME} SHARED
    ${SOURCES}
    ${HEADERS}
    #${MOC_HDS}
    ${PROCESS_SRC}
    ${PROCESS_INC}
)

if(MSVC)
    set(3DCAMERA_LIBRARY optimized 3DCamera debug 3DCamerad)
    set(YAMLCPP_LIBRARY optimized yaml-cpp debug yaml-cppd)
    set(QUAZIP_LIBRARY optimized quazip debug quazipd)
else()
    set(3DCAMERA_LIBRARY 3DCamera)
    set(YAMLCPP_LIBRARY yaml-cpp)
    set(QUAZIP_LIBRARY quazip)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE
    ${3DCAMERA_LIBRARY}
    ${YAMLCPP_LIBRARY}
    ${QUAZIP_LIBRARY}
    csutil
    Qt5::Core
    Qt5::Gui)

if(USE_OPENMP)
    target_link_libraries(${TARGET_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()

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
