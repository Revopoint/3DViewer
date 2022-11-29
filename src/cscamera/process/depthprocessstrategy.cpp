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

#include "process/depthprocessstrategy.h"

#include <QImage>
#include <QVariant>
#include <QFile>
#include <QDebug>
#include <hpp/Processing.hpp>

#include "icscamera.h"
#include "cameraparaid.h"
#include "process/fillhole.h"

using namespace cs;

DepthProcessStrategy::DepthProcessStrategy()
    : DepthProcessStrategy(STRATEGY_DEPTH)
{

}

DepthProcessStrategy::DepthProcessStrategy(PROCESS_STRA_TYPE type)
    : ProcessStrategy(type)
    , calcDepthCoord(false)
    , depthCoordCalcPos(QPointF(-1.0f, -1.0f))
    , fillHole(false)
    , filterValue(0)
    , filterType(0)
{

}

void DepthProcessStrategy::doProcess(const FrameData& frameData, OutputDataPort& outputDataPort)
{
    const auto& streamDatas = frameData.data;

    for (const auto& data : streamDatas)
    {
        STREAM_FORMAT format = data.dataInfo.format;
        QVector<OutputData2D> outputData2Ds;

        switch (format)
        {
        case STREAM_FORMAT_Z16:
            outputData2Ds = onProcessZ16(data);
            break;
        case STREAM_FORMAT_Z16Y8Y8:
            outputData2Ds = onProcessZ16Y8Y8(data);
            break;
        case STREAM_FORMAT_PAIR:
            outputData2Ds = onProcessPAIR(data);
            break;
        default:
            break;
        }

        outputDataPort.addOutputData2D(outputData2Ds);
    } 
}

OutputData2D DepthProcessStrategy::processDepthData(const ushort* dataPtr, int length, int width, int height)
{
    QByteArray output;
    QImage image;

    onProcessDepthData(dataPtr, length, width, height, output, image);

    OutputData2D outputData;
    outputData.image = image;

    // calc point info
    if (calcDepthCoord)
    {
        int x = depthCoordCalcPos.x() * width;
        int y = depthCoordCalcPos.y() * height;

        if (!(x < 0 || y < 0 || x >= width || y >= height))
        {
            ushort d = dataPtr[y * width + x];

            cs::Pointcloud pc;
            cs::float3 point;
            cs::float2 texcoord;

            pc.generatePoint(point, texcoord, x, y, d, width, height, depthScale, &depthIntrinsics);

            outputData.info.vertex = QVector3D(point.x, point.y, point.z);
            outputData.info.depthScale = depthScale;
        }
    }

    return outputData;
}

void DepthProcessStrategy::onProcessDepthData(const ushort* dataPtr, int length, int width, int height, QByteArray& output, QImage& depthImage)
{
    //tran to float
    output.resize(length * sizeof(float));
    float* floatPtr = (float*)output.data();
    copyData<const ushort, float>(dataPtr, floatPtr, length);

    //fill hole
    if (fillHole)
    {
        depthFillHole(floatPtr, width, height);
    }

    // filter
    FILTER_TYPE type = (FILTER_TYPE)filterType;
    switch (type)
    {
    case FILTER_SMOOTH:
        filter::AverageBlur<float>(floatPtr, width, height, filterValue);
        break;
    case FILTER_MEDIAN:
        filter::MedianBlur<float>(floatPtr, width, height, filterValue);
        break;
    case FILTER_TDSMOOTH:
        timeDomainSmooth(dataPtr, length, width, height, floatPtr);
        break;
    default:
        break;
    }
   
    // colorize
    cs::colorizer color;
    color.setRange(depthRange.first, depthRange.second);

    depthImage =  QImage(width, height, QImage::Format_RGB888);
    color.process<float>(floatPtr, depthScale, depthImage.bits(), length);
}

void DepthProcessStrategy::timeDomainSmooth(const ushort* dataPtr, int length, int width, int height, float* output)
{
    if (filterCachedData.size() >= filterValue)
    {
        filterCachedData.pop_front();
    }

    //copy data
    QByteArray curFrame;
    curFrame.resize(length * sizeof(ushort));
    memcpy(curFrame.data(), dataPtr, curFrame.size());

    filterCachedData.push_back(curFrame);

    // smooth
    const int size = width * height;
    QList<ushort*> list;
    for (auto& data : filterCachedData)
    {
        if(data.size() == size * sizeof(ushort))
        {
            list.push_back((ushort*)data.data());
        }
    }

    const int frameSize = list.size();
#pragma omp parallel for
    for (int i = 0; i < size; i++)
    {
        float sum = 0;
        int count = 0;

        for (int j = 0; j < frameSize; j++)
        {
            const auto& v = (list[j])[i];
            if (v > 0)
            {
                sum += v;
                count++;
            }
        }
        output[i] = count > 0 ? (sum / count) : 0;
    }
}

OutputData2D DepthProcessStrategy::onProcessLData(const char* dataPtr, int length, int width, int height)
{
    QImage imageL = QImage(width, height, QImage::Format_Grayscale8);
    memcpy(imageL.bits(), dataPtr, length);

    OutputData2D outputData;
    outputData.image = imageL;
    outputData.info.cameraDataType = CAMERA_DATA_L;
    emit output2DUpdated(outputData);

    return outputData;
}

OutputData2D DepthProcessStrategy::onProcessRData(const char* dataPtr, int length, int width, int height)
{
    QImage imageR = QImage(width, height, QImage::Format_Grayscale8);
    memcpy(imageR.bits(), dataPtr, length);

    OutputData2D outputData;
    outputData.image = imageR;
    outputData.info.cameraDataType = CAMERA_DATA_R;
    emit output2DUpdated(outputData);
    
    return outputData;
}

QVector<OutputData2D> DepthProcessStrategy::onProcessZ16(const StreamData& streamData)
{
    const int& width = streamData.dataInfo.width;
    const int& height = streamData.dataInfo.height;
    Q_ASSERT(streamData.data.size() == width * height * sizeof(ushort));

    OutputData2D outputData = processDepthData((const ushort*)streamData.data.data(), width * height, width, height);
    outputData.info.cameraDataType = CAMERA_DATA_DEPTH;

    emit output2DUpdated(outputData);

    QVector<OutputData2D> outputDatas;
    outputDatas.push_back(outputData);

    return outputDatas;
}

QVector<OutputData2D> DepthProcessStrategy::onProcessZ16Y8Y8(const StreamData& streamData)
{
    const int& width = streamData.dataInfo.width;
    const int& height = streamData.dataInfo.height;
    const int& dataSize = streamData.data.size();
    int dataOffset = 0;

    Q_ASSERT(dataSize == (width * height * sizeof(ushort) + width * height * 2));

    QVector<OutputData2D> outputDatas;
    //depth
    OutputData2D outputData = processDepthData((const ushort*)streamData.data.data() + dataOffset, width * height, width, height);
    outputData.info.cameraDataType = CAMERA_DATA_DEPTH;
    outputDatas.push_back(outputData);

    dataOffset += width * height * sizeof(ushort);
    emit output2DUpdated(outputData);

    //L
    outputData = onProcessLData((const char*)streamData.data.data() + dataOffset, width * height, width, height);
    outputDatas.push_back(outputData);
    dataOffset += width * height;

    //R
    outputData = onProcessRData((const char*)streamData.data.data() + dataOffset, width * height, width, height);
    outputDatas.push_back(outputData);
    dataOffset += width * height;

    return outputDatas;
}

QVector<OutputData2D> DepthProcessStrategy::onProcessPAIR(const StreamData& streamData)
{
    const int width = streamData.dataInfo.width;
    const int height = streamData.dataInfo.height;
    Q_ASSERT(streamData.data.size() == width * height * 2);

    int dataOffset = 0;

    QVector<OutputData2D> outputDatas;
    //L
    OutputData2D outputData = onProcessLData((const char*)streamData.data.data() + dataOffset, width * height, width, height);
    outputDatas.push_back(outputData);
    dataOffset += width * height;

    //R
    outputData = onProcessRData((const char*)streamData.data.data() + dataOffset, width * height, width, height);
    outputDatas.push_back(outputData);
    dataOffset += width * height;

    return outputDatas;
}

void DepthProcessStrategy::onLoadCameraPara()
{
    const auto parameters =
    {
        PARA_DEPTH_RANGE,
        PARA_DEPTH_SCALE,
        PARA_DEPTH_INTRINSICS,
        PARA_DEPTH_FILL_HOLE,
        PARA_DEPTH_FILTER_TYPE,
        PARA_DEPTH_FILTER
    };

    for (auto para : parameters)
    {
        QVariant value;
        cameraPtr->getCameraPara(para, value);
        switch (para)
        {
        case PARA_DEPTH_RANGE:
            depthRange = value.value<QPair<float, float> >();
            break;
        case PARA_DEPTH_SCALE:
            depthScale = value.toFloat();
            break;
        case PARA_DEPTH_INTRINSICS:
            depthIntrinsics = value.value<Intrinsics>();
            break;
        case PARA_DEPTH_FILL_HOLE:
            fillHole = value.toBool();
            break;
        case PARA_DEPTH_FILTER:
            filterValue = value.toInt();
            break;
        case PARA_DEPTH_FILTER_TYPE:
            filterType = value.toInt();
            if (filterType != FILTER_TDSMOOTH)
            {
                filterCachedData.clear();
            }
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    //add log
    if (qAbs(depthScale) < 0.0000001)
    {
        qWarning() << "invalid depth scale, depthScale = " << depthScale;
        Q_ASSERT(false);
    }
}

bool DepthProcessStrategy::getCalcDepthCoord() const
{
    return calcDepthCoord;
}

void DepthProcessStrategy::setCalcDepthCoord(bool calc)
{
    calcDepthCoord = calc;
}

QPointF DepthProcessStrategy::getDepthCoordCalcPos() const
{
    return depthCoordCalcPos;
}

void  DepthProcessStrategy::setDepthCoordCalcPos(QPointF pos)
{
    depthCoordCalcPos = pos;
}