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
    , m_withTexture(false)
{
    m_dependentParameters.push_back(PARA_RGB_INTRINSICS);
    m_dependentParameters.push_back(PARA_EXTRINSICS);
    m_dependentParameters.push_back(PARA_HAS_RGB);
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

    // If in single-trigger mode and time-domain filtering is enabled, 
    // the cached data of the time-domain filter needs to be cleared.
    if (m_trigger != TRIGGER_MODE_OFF && m_filterType == FILTER_TDSMOOTH)
    {
        m_filterCachedData.clear();
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
    
    if (pc.getVertices().size() > 0)
    {
        emit output3DUpdated(pc, texImage);
        outputDataPort.setPointCloud(pc);
    }
}

void PointCloudProcessStrategy::onLoadCameraPara()
{
    DepthProcessStrategy::onLoadCameraPara();

    for (auto para : m_dependentParameters)
    {
        auto emPara = (CAMERA_PARA_ID)para;
        QVariant value;

        m_cameraPtr->getCameraPara(emPara, value);
        switch (para)
        {
        case PARA_RGB_INTRINSICS:
            m_rgbIntrinsics = value.value<Intrinsics>();
            break;
        case PARA_EXTRINSICS:
            m_extrinsics = value.value<Extrinsics>();
            break;
        case PARA_HAS_RGB:
            m_withTexture = value.toBool();
            break;
        default:
            break;
        }
    }
}

bool PointCloudProcessStrategy::getWithTexture() const
{
    return m_withTexture;
}

void PointCloudProcessStrategy::setWithTexture(bool with)
{
    m_withTexture = with;
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
    if (!onProcessDepthData(dataPtr, width * height, width, height, floatData))
    {
        return;
    }
    
    float* floatPtr = (float*)floatData.data();
    bool hasTex = m_withTexture && depthData.data.size() > 1;

    switch (depthData.dataInfo.format)
    {
    case STREAM_FORMAT_Z16:
    case STREAM_FORMAT_Z16Y8Y8:
        if (hasTex) {
            pc.generatePoints<float>(floatPtr, width, height, m_depthScale, &m_depthIntrinsics, &m_rgbIntrinsics, &m_extrinsics, true);
        }
        else
        {
            pc.generatePoints<float>(floatPtr, width, height, m_depthScale, &m_depthIntrinsics, nullptr, nullptr, true);
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
    if (m_withTexture)
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