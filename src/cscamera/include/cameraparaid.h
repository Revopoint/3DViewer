/*******************************************************************************
* This file is part of the 3DViewer                                            *
*                                                                              *
* Copyright (C) 2022 Revopoint3D Company Ltd.                                  *
* All rights reserved.                                                         *
*                                                                              *
* This program is free software: you can redistribute it and/or modify         *
* it under the terms of the GNU General Public License as published by         *
* the Free Software Foundation, either version 3 of the License, or            *
* (at your option) any later version.                                          *
*                                                                              *
* This program is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)             *
* for more details.                                                            *
*                                                                              *
********************************************************************************/

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
            PARA_DEPTH_HAS_IR,
            PARA_TRIGGER_MODE,
            PARA_CAMERA_IP,

            // depth
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