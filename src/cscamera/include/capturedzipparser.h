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
    QString m_zipFile = "";
    // name of capture 
    QString m_captureName = "";
    // captured data types
    QVector<int> m_dataTypes;
    // captured data format: images or raw
    QString m_dataFormat = "";
    // resolution
    QSize m_depthResolution = QSize(0, 0);
    QSize m_rgbResolution = QSize(0, 0);
    // the frame count of capture
    int m_capturedFrameCount = 0;
    // the time stamps in zip file is valid
    bool m_isTimeStampsValid = false;
    // the time stamps of frames
    QVector<int> m_depthTimeStamps;
    QVector<int> m_rgbTimeStamps;
    bool m_enableTexture = false;

    // camera parameter
    Intrinsics m_depthIntrinsics;
    Intrinsics m_rgbIntrinsics;
    Extrinsics m_extrinsics;
    float m_depthScale = 0.0f;
    float m_depthMin = 0.0f;
    float m_depthMax = 0.0f;
};

}
#endif // _CS_CAPTUREDZIPPARSER_H