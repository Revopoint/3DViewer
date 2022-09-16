/*******************************************************************************
* This file is part of the 3DViewer
*
* Copyright 2022-2026 (C) Revopoint3D AS
* All rights reserved.
*
* Revopoint3D Software License, v1.0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistribution in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of Revopoint3D AS nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* 4. This software, with or without modification, must not be used with any
* other 3D camera than from Revopoint3D AS.
*
* 5. Any software provided in binary form under this license must not be
* reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY REVOPOINT3D AS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL REVOPOINT3D AS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Info:  https://www.revopoint3d.com
******************************************************************************/

#ifndef _CS_CAMERAPARAID_H
#define _CS_CAMERAPARAID_H

#include "cscameraapi.h"
#include <QMetaType>

namespace cs{
    //Q_NAMESPACE;
    namespace parameter
    {
        Q_NAMESPACE
        enum CAMERA_PARA_ID
        {
            PARA_HAS_DEPTH = 0,
            PARA_HAS_RGB,
            PARA_EXTRINSICS,
            PARA_DEPTH_HAS_LR,
            PARA_TRIGGER_MODE,

            //depth
            PARA_DEPTH_STREAM_FORMAT,
            PARA_DEPTH_RESOLUTION,
            PARA_DEPTH_RANGE,
            PARA_DEPTH_GAIN,
            PARA_DEPTH_EXPOSURE,
            PARA_DEPTH_FRAMETIME,
            PARA_DEPTH_THRESHOLD,
            PARA_DEPTH_FILTER_TYPE,
            PARA_DEPTH_FILTER,
            PARA_DEPTH_AUTO_EXPOSURE,
            PARA_DEPTH_FILL_HOLE,
            PARA_DEPTH_HDR_MODE,
            PARA_DEPTH_HDR_LEVEL,
            PARA_DEPTH_SCALE,
            PARA_DEPTH_HDR_SETTINGS,
            PARA_DEPTH_ROI,
            PARA_DEPTH_INTRINSICS,

            // rgb 
            PARA_RGB_STREAM_FORMAT,
            PARA_RGB_RESOLUTION,
            PARA_RGB_GAIN,
            PARA_RGB_EXPOSURE,
            PARA_RGB_AUTO_EXPOSURE,
            PARA_RGB_WHITE_BALANCE,
            PARA_RGB_AUTO_WHITE_BALANCE,
            PARA_RGB_INTRINSICS,

            PARA_COUNT
        };

       Q_ENUM_NS(CAMERA_PARA_ID);
    };
}

#endif // _CS_CAMERAPARAID_H