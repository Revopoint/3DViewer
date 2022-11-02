
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was QuaZipConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

find_dependency(ZLIB REQUIRED)

include("${CMAKE_CURRENT_LIST_DIR}/QuaZip-Qt5_SharedTargets.cmake")

if(5 EQUAL 6)
	find_dependency(Qt6 REQUIRED COMPONENTS Core Core5Compat)
elseif(5 EQUAL 5)
	find_dependency(Qt5 REQUIRED COMPONENTS Core)
elseif(5 EQUAL 4)
	find_dependency(Qt4 4.5.0 REQUIRED COMPONENTS QtCore)
else()
    message(FATAL_ERROR "Qt version QUAZIP_QT_MAJOR_VERSION=5 is unsupported")
endif()
set_target_properties(QuaZip::QuaZip PROPERTIES IMPORTED_GLOBAL TRUE)

check_required_components(QuaZip-Qt5)
