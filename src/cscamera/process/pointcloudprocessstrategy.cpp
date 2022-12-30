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

#include "process/pointcloudprocessstrategy.h"
#include <QDebug>
#include <hpp/Processing.hpp>
#include "icscamera.h"
#include "cameraparaid.h"

using namespace cs;

PointCloudProcessStrategy::PointCloudProcessStrategy()
    : DepthProcessStrategy(STRATEGY_CLOUD_POINT)
    , withTexture(false)
{

}

void PointCloudProcessStrategy::doProcess(const FrameData& frameData, OutputDataPort& outputDataPort)
{
    const auto& streamDatas = frameData.data;
    
    if (streamDatas.size() <= 0)
    {
        return;
    }

    if (streamDatas.first().dataInfo.format == STREAM_FORMAT_PAIR)
    {
        return;
    }

    Pointcloud pc;
    QImage texImage;
    for (const StreamData& streamData : streamDatas)
    {
        switch (streamData.dataInfo.format)
        {
        case STREAM_FORMAT_Z16:
        case STREAM_FORMAT_Z16Y8Y8:
        case STREAM_FORMAT_XZ32:
            generatePointCloud(streamData, pc);
            break;
        case STREAM_FORMAT_RGB8:
        case STREAM_FORMAT_MJPG:
            generateTexture(streamData, texImage);
            break;
        default:
            qDebug() << "invalid stream format.";
            Q_ASSERT(false);
            return;
        }
    }

    emit output3DUpdated(pc, texImage);
    outputDataPort.setPointCloud(pc);
}

void PointCloudProcessStrategy::onLoadCameraPara()
{
    DepthProcessStrategy::onLoadCameraPara();

    const auto parameters =
    {
        PARA_RGB_INTRINSICS,
        PARA_EXTRINSICS,
        PARA_HAS_RGB
    };

    for (auto para : parameters)
    {
        QVariant value;
        cameraPtr->getCameraPara(para, value);
        switch (para)
        {
        case PARA_RGB_INTRINSICS:
            rgbIntrinsics = value.value<Intrinsics>();
            break;
        case PARA_EXTRINSICS:
            extrinsics = value.value<Extrinsics>();
            break;
        case PARA_HAS_RGB:
            withTexture = value.toBool();
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }
}

bool PointCloudProcessStrategy::getWithTexture() const
{
    return withTexture;
}

void PointCloudProcessStrategy::setWithTexture(bool with)
{
    withTexture = with;
}

void PointCloudProcessStrategy::generatePointCloud(const StreamData& depthData, Pointcloud& pc)
{
    // Point Cloud
    const int width = depthData.dataInfo.width;
    const int height = depthData.dataInfo.height;
    const ushort* dataPtr = (const ushort*)depthData.data.data();

    Q_ASSERT(depthData.data.size() >= width * height * sizeof(ushort));

    // depth process
    QByteArray floatData;
    QImage depthImage, texImage;
    onProcessDepthData(dataPtr, width * height, width, height, floatData, depthImage);

    float* floatPtr = (float*)floatData.data();

    bool hasTex = withTexture && depthData.data.size() > 1;
    switch (depthData.dataInfo.format)
    {
    case STREAM_FORMAT_Z16:
    case STREAM_FORMAT_Z16Y8Y8:
        if (hasTex) {
            pc.generatePoints<float>(floatPtr, width, height, depthScale, &depthIntrinsics, &rgbIntrinsics, &extrinsics, true);
        }
        else
        {
            pc.generatePoints<float>(floatPtr, width, height, depthScale, &depthIntrinsics, nullptr, nullptr, true);
        }
        break;
    case STREAM_FORMAT_XZ32:
        pc.generatePointsFromXZ(floatPtr, width);
        break;
    default:
        qDebug() << "invalid stream format.";
        Q_ASSERT(false);
        return;
    }
}

void PointCloudProcessStrategy::generateTexture(const StreamData& rgbData, QImage& texImage)
{
    // rgb process
    if (withTexture)
    {
        STREAM_FORMAT format = rgbData.dataInfo.format;

        if (format == STREAM_FORMAT_RGB8)
        {
            QImage image = QImage((uchar*)rgbData.data.data(), rgbData.dataInfo.width, rgbData.dataInfo.height, QImage::Format_RGB888);
            texImage = image.copy(image.rect());
        }
        else if (format == STREAM_FORMAT_MJPG)
        {
            QImage image;
            image.loadFromData(rgbData.data, "JPG");
            image = image.convertToFormat(QImage::Format_RGB888);

            texImage = image;
        }
        else {
            qDebug() << "invalid stream format.";
            Q_ASSERT(false);
        }
    }
}