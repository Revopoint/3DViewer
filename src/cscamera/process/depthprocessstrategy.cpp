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
    , m_calcDepthCoord(false)
    , m_depthCoordCalcPos(QPointF(-1.0f, -1.0f))
    , m_fillHole(false)
    , m_filterValue(0)
    , m_filterType(0)
{
    m_dependentParameters.push_back(PARA_DEPTH_RANGE);
    m_dependentParameters.push_back(PARA_DEPTH_SCALE);
    m_dependentParameters.push_back(PARA_DEPTH_INTRINSICS);
    m_dependentParameters.push_back(PARA_DEPTH_FILL_HOLE);
    m_dependentParameters.push_back(PARA_DEPTH_FILTER_TYPE);
    m_dependentParameters.push_back(PARA_DEPTH_FILTER);
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
    if (m_calcDepthCoord)
    {
        int x = m_depthCoordCalcPos.x() * width;
        int y = m_depthCoordCalcPos.y() * height;

        if (!(x < 0 || y < 0 || x >= width || y >= height))
        {
            ushort d = dataPtr[y * width + x];

            cs::Pointcloud pc;
            cs::float3 point;
            cs::float2 texcoord;

            pc.generatePoint(point, texcoord, x, y, d, width, height, m_depthScale, &m_depthIntrinsics);

            outputData.info.vertex = QVector3D(point.x, point.y, point.z);
            outputData.info.depthScale = m_depthScale;
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
    if (m_fillHole)
    {
        depthFillHole(floatPtr, width, height);
    }

    // filter
    FILTER_TYPE type = (FILTER_TYPE)m_filterType;
    switch (type)
    {
    case FILTER_SMOOTH:
        filter::AverageBlur<float>(floatPtr, width, height, m_filterValue);
        break;
    case FILTER_MEDIAN:
        filter::MedianBlur<float>(floatPtr, width, height, m_filterValue);
        break;
    case FILTER_TDSMOOTH:
        timeDomainSmooth(dataPtr, length, width, height, floatPtr);
        break;
    default:
        break;
    }
   
    // colorize
    cs::colorizer color;
    color.setRange(m_depthRange.first, m_depthRange.second);

    depthImage =  QImage(width, height, QImage::Format_RGB888);
    color.process<float>(floatPtr, m_depthScale, depthImage.bits(), length);
}

void DepthProcessStrategy::timeDomainSmooth(const ushort* dataPtr, int length, int width, int height, float* output)
{
    if (m_filterCachedData.size() >= m_filterValue)
    {
        m_filterCachedData.pop_front();
    }

    //copy data
    QByteArray curFrame;
    curFrame.resize(length * sizeof(ushort));
    memcpy(curFrame.data(), dataPtr, curFrame.size());

    m_filterCachedData.push_back(curFrame);

    // smooth
    const int size = width * height;
    QList<ushort*> list;
    for (auto& data : m_filterCachedData)
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
    for (auto para : m_dependentParameters)
    {
        auto emPara = (CAMERA_PARA_ID)para;

        QVariant value;
        m_cameraPtr->getCameraPara(emPara, value);
        switch (para)
        {
        case PARA_DEPTH_RANGE:
            m_depthRange = value.value<QPair<float, float> >();
            break;
        case PARA_DEPTH_SCALE:
            m_depthScale = value.toFloat();
            break;
        case PARA_DEPTH_INTRINSICS:
            m_depthIntrinsics = value.value<Intrinsics>();
            break;
        case PARA_DEPTH_FILL_HOLE:
            m_fillHole = value.toBool();
            break;
        case PARA_DEPTH_FILTER:
            m_filterValue = value.toInt();
            break;
        case PARA_DEPTH_FILTER_TYPE:
            m_filterType = value.toInt();
            if (m_filterType != FILTER_TDSMOOTH)
            {
                m_filterCachedData.clear();
            }
            break;
        default:
            break;
        }
    }

    //add log
    if (qAbs(m_depthScale) < 0.0000001)
    {
        qWarning() << "invalid depth scale, depthScale = " << m_depthScale;
        Q_ASSERT(false);
    }
}

bool DepthProcessStrategy::getCalcDepthCoord() const
{
    return m_calcDepthCoord;
}

void DepthProcessStrategy::setCalcDepthCoord(bool calc)
{
    m_calcDepthCoord = calc;
}

QPointF DepthProcessStrategy::getDepthCoordCalcPos() const
{
    return m_depthCoordCalcPos;
}

void  DepthProcessStrategy::setDepthCoordCalcPos(QPointF pos)
{
    m_depthCoordCalcPos = pos;
}