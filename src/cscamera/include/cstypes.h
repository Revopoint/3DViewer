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

#ifndef _CS_TYPES_H
#define _CS_TYPES_H

#include <QByteArray>
#include <QVector>
#include <QImage>
#include <QVector3D>
#include <QVariant>
#include <QMetaType>
#include <QMetaEnum>
#include <hpp/Types.hpp>

enum CAMERA_STATE
{
    CAMERA_CONNECTING                           = 0,
    CAMERA_CONNECTED                            = 1,
    CAMERA_CONNECTFAILED                        = 2,   
    CAMERA_DISCONNECTING                        = 3,
    CAMERA_DISCONNECTED                         = 4,
    CAMERA_DISCONNECTFAILED                     = 5,
    CAMERA_STARTING_STREAM                      = 6,
    CAMERA_STARTED_STREAM                       = 7,
    CAMERA_START_STREAM_FAILED                  = 8,
    CAMERA_PAUSING_STREAM                       = 9,
    CAMERA_PAUSED_STREAM                        = 10,
    CAMERA_STOPPING_STREAM                      = 11,
    CAMERA_STOPPED_STREAM                       = 12,
    CAMERA_RESTARTING_CAMERA                    = 13,
};

enum STREAM_DATA_TYPE
{
    TYPE_UNKNOW            = 0,
    TYPE_DEPTH             = (1 << 0),
    TYPE_RGB               = (1 << 1),
};

struct StreamDataInfo 
{ 
    STREAM_DATA_TYPE streamDataType;
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
    Intrinsics rgbIntrinsics;
    Intrinsics depthIntrinsics;
    Extrinsics extrinsics;
    float depthScale;

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
    CAMERA_DATA_POINT_CLOUD = (1 << 4)
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

    bool isEmpty() const 
    {
        return info.cameraDataType == CAMERA_DATA_UNKNOW;
    }
};

Q_DECLARE_METATYPE(OutputInfo2D)
Q_DECLARE_METATYPE(OutputData2D)
Q_DECLARE_METATYPE(Extrinsics)
Q_DECLARE_METATYPE(Intrinsics)
Q_DECLARE_METATYPE(HdrExposureParam)
Q_DECLARE_METATYPE(HdrExposureSetting)
Q_DECLARE_METATYPE(CameraIpSetting)

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

enum CAMERA_CAPTURE_TYPE
{
    CAPTURE_TYPE_SINGLE,
    CAPTURE_TYPE_MULTIPLE
};
struct CameraCaptureConfig
{
    CAMERA_CAPTURE_TYPE captureType;
    int captureNumber = 1;
    QVector<CS_CAMERA_DATA_TYPE> captureDataTypes;
    bool savePointCloudWithTexture = false;
    QString saveFormat;
    QString saveDir;
    QString saveName;
};

enum WINDOWLAYOUT_MODE
{
    LAYOUT_TILE,
    LAYOUT_TAB
};

#endif // _CS_TYPES_H