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

#ifndef _CS_TYPES_H
#define _CS_TYPES_H

#include <QByteArray>
#include <QVector>
#include <QImage>
#include <QVector3D>
#include <QVariant>
#include <QMetaType>
#include <hpp/Types.hpp>

enum CAMERA_STATE
{
    CAMERA_CONNECTING  = 0,
    CAMERA_CONNECTED,
    CAMERA_CONNECTFAILED,
    CAMERA_DISCONNECTING,
    CAMERA_DISCONNECTED,
    CAMERA_DISCONNECTFAILED,
    CAMERA_STARTING_STREAM,
    CAMERA_STARTED_STREAM,
    CAMERA_PAUSING_STREAM,
    CAMERA_PAUSED_STREAM,
    CAMERA_STOPPING_STREAM,
    CAMERA_STOPPED_STREAM,
    CAMERA_RESTARTING_CAMERA
};

enum FRAME_DATA_TYPE
{
    TYPE_UNKNOW            = 0,
    TYPE_DEPTH             = (1 << 0),
    TYPE_RGB               = (1 << 1),
    TYPE_DEPTH_RGB         = (TYPE_DEPTH | TYPE_RGB)
};

struct StreamDataInfo 
{ 
    STREAM_FORMAT format;
    int width;
    int height;
    double timeStamp;
}; 

struct StreamData
{
    StreamDataInfo dataInfo;
    QByteArray data;
}; 
 
struct FrameData
{ 
    FRAME_DATA_TYPE frameDataType;
    QVector<StreamData> data; 
};

Q_DECLARE_METATYPE(FrameData);

enum CS_CAMERA_DATA_TYPE
{
    CAMERA_DATA_UNKNOW     = 0,
    CAMERA_DATA_L          = (1 << 0),
    CAMERA_DATA_R          = (1 << 1),
    CAMERA_DATA_DEPTH      = (1 << 2),
    CAMERA_DATA_RGB        = (1 << 3),
    CAMERA_DTA_POINT_CLOUD = (1 << 4)
};

struct OutputInfo2D
{
    int cameraDataType = CAMERA_DATA_UNKNOW;
    QVector3D vertex = { 1.0f, 1.0f, 1.0f };
    float depthScale = 0.0f;
};

struct OutputData2D
{
    QImage image;
    OutputInfo2D info;
};

Q_DECLARE_METATYPE(OutputInfo2D)
Q_DECLARE_METATYPE(OutputData2D)
Q_DECLARE_METATYPE(Extrinsics)
Q_DECLARE_METATYPE(Intrinsics)
Q_DECLARE_METATYPE(HdrExposureParam)
Q_DECLARE_METATYPE(HdrExposureSetting)

enum FILTER_TYPE
{
    FILTER_CLOSE = 0,
    FILTER_SMOOTH,
    FILTER_MEDIAN,
    FILTER_TDSMOOTH
};

struct CSRange
{
    int min;
    int max;
};

enum CAMERA_HDR_MODE
{
    HDR_MODE_CLOSE = 0,
    HDR_MODE_SHINE,
    HDR_MODE_DARK,
    HDR_MODE_BOTH,
    HDR_MODE_MANUAL,
};

enum CSConnectType
{
    CONNECT_TYPE_NET = 0,
    CONNECT_TYPE_USB,
    CONNECT_TYPE_COUNT
};

struct CSCameraInfo 
{
    QString model;
    int connectType;
    CameraInfo cameraInfo;
    QString sdkVersion;
};

struct CameraCaptureConfig
{
    int captureNumber = 1;
    QVector<CS_CAMERA_DATA_TYPE> captureDataTypes;
    QString saveFormat;
    QString saveDir;
    QString saveName;
};

#endif // _CS_TYPES_H