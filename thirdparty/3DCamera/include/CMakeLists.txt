# License: Apache 2.0. See LICENSE file in root directory.
# Copyright(c) 2019 Intel Corporation. All Rights Reserved.
target_sources(${LRS_TARGET}
    PRIVATE
         "${CMAKE_CURRENT_LIST_DIR}/3DCamera.hpp"

        "${CMAKE_CURRENT_LIST_DIR}/hpp/System.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/hpp/Camera.hpp"
        "${CMAKE_CURRENT_LIST_DIR}/hpp/Frame.hpp"
		"${CMAKE_CURRENT_LIST_DIR}/hpp/Types.hpp"
		"${CMAKE_CURRENT_LIST_DIR}/hpp/Processing.hpp"
		
		"${CMAKE_CURRENT_LIST_DIR}/hpp/HandEye/HandEye.hpp"
		"${CMAKE_CURRENT_LIST_DIR}/hpp/HandEye/HandEyeInput.hpp"
)

target_link_libraries(${LRS_TARGET} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../3DCamera" )
