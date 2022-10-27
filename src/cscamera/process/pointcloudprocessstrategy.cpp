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

            texImage = image;
        }
        else {
            qDebug() << "invalid stream format.";
            Q_ASSERT(false);
        }
    }
}