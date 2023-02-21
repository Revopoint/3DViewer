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

#include "process/outputdataport.h"

OutputDataPort::OutputDataPort()
{

}

OutputDataPort::OutputDataPort(const FrameData& frameData)
    : m_frameData(frameData)
{

}

OutputDataPort::OutputDataPort(const OutputDataPort& other)
{
    m_outputData2DMap = other.m_outputData2DMap;
    m_pointCloud = other.m_pointCloud;
    m_frameData = other.m_frameData;
}

OutputDataPort::~OutputDataPort()
{
    m_outputData2DMap.clear();
}

bool OutputDataPort::isEmpty() const
{
    return !hasData(CAMERA_DATA_POINT_CLOUD) && m_outputData2DMap.isEmpty();
}

bool OutputDataPort::hasData(CS_CAMERA_DATA_TYPE dataType) const
{
    if (dataType == CAMERA_DATA_POINT_CLOUD)
    {
        return m_pointCloud.size() > 0;
    }
    else 
    {
        return m_outputData2DMap.contains(dataType);
    }
}

cs::Pointcloud OutputDataPort::getPointCloud()
{
    return m_pointCloud;
}

OutputData2D OutputDataPort::getOutputData2D(CS_CAMERA_DATA_TYPE dataType)
{
    if (!hasData(dataType))
    {
        return OutputData2D();
    }

    return m_outputData2DMap[dataType];
}

QMap<CS_CAMERA_DATA_TYPE, OutputData2D> OutputDataPort::getOutputData2Ds()
{
    return m_outputData2DMap;
}

FrameData OutputDataPort::getFrameData() const
{
    return m_frameData;
}

void OutputDataPort::setPointCloud(const cs::Pointcloud& pointCloud)
{
    this->m_pointCloud = pointCloud;
}

void OutputDataPort::addOutputData2D(const OutputData2D& outputData2D)
{
    CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)outputData2D.info.cameraDataType;
    m_outputData2DMap[dataType] = outputData2D;
}

void OutputDataPort::addOutputData2D(const QVector<OutputData2D>& outputData2Ds)
{
    for (const auto& data : outputData2Ds)
    {
        addOutputData2D(data);
    }
}

void OutputDataPort::setFrameData(const FrameData& frameData)
{
    this->m_frameData = frameData;
}