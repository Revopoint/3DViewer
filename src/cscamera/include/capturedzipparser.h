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

#ifndef _CS_CAPTUREDZIPPARSER_H
#define _CS_CAPTUREDZIPPARSER_H
#include <QObject>
#include <QVector>
#include <QString>
#include <QImage>

#include "cstypes.h"
#include "cscameraapi.h"
#include <hpp/Processing.hpp>

namespace cs
{

class CS_CAMERA_EXPORT CapturedZipParser : public QObject
{
    Q_OBJECT
public:
    CapturedZipParser(QString filePath = "");
    ~CapturedZipParser();

    void setZipFile(QString filePath);
    bool checkFileValid();
    bool parseCaptureInfo();
    bool parseTimeStamps();
    bool checkTimeStampsValid();

    QByteArray getFrameData(int frameIndex, int dataType);
    QImage getImageOfFrame(int frameIndex, int dataType);
    bool getPointCloud(int frameIndex, Pointcloud& pc, QImage& texImage);

    bool generatePointCloud(int depthIndex, int rgbIndex, bool withTexture, Pointcloud& pc, QImage& tex);
    bool saveFrameToLocal(int frameIndex, bool withTexture, QString filePath);
    int getRgbFrameIndexByTimeStamp(int depthIndex);

    bool enablePointCloudTexture();
    // get parameters
    QVector<int> getDataTypes();
    int getFrameCount();
    bool isRawFormat();
    QString getCaptureName();
    bool getIsTimeStampsValid();
private:
    QString getFileName(int frameIndex, int dataType);
    QString getFileName(int dataType, QString name);

    QImage convertPng2QImage(QByteArray data, int dataType);
    QImage convertPixels2QImage(QByteArray data, int dataType);
    QImage convertData2QImage(int width, int height, QByteArray data);

    bool saveImageData(int frameIndex, int dataType, QString filePath);
    bool savePointCloud(int frameIndex, bool withTexture, QString filePath);

    // time stamp
    int getTimeStampOfFrame(int frameIndex, int dataType);
    int getNearestRgbFrame(int depthIndex, int timeStamp);
private:
    // zip file path
    QString zipFile = "";
    // name of capture 
    QString captureName = "";
    // captured data types
    QVector<int> dataTypes;
    // captured data format: images or raw
    QString dataFormat = "";
    // resolution
    QSize depthResolution = QSize(0, 0);
    QSize rgbResolution = QSize(0, 0);
    // the frame count of capture
    int capturedFrameCount = 0;
    // the time stamps in zip file is valid
    bool isTimeStampsValid = false;
    // the time stamps of frames
    QVector<int> depthTimeStamps;
    QVector<int> rgbTimeStamps;
    bool enableTexture = false;

    // camera parameter
    Intrinsics depthIntrinsics;
    Intrinsics rgbIntrinsics;
    Extrinsics extrinsics;
    float depthScale = 0.0f;
    float depthMin = 0.0f;
    float depthMax = 0.0f;
};

}
#endif // _CS_CAPTUREDZIPPARSER_H