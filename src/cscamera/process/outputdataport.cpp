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
    : frameData(frameData)
{

}

OutputDataPort::OutputDataPort(const OutputDataPort& other)
{
    outputData2DMap = other.outputData2DMap;
    pointCloud = other.pointCloud;
    frameData = other.frameData;
}

OutputDataPort::~OutputDataPort()
{
    outputData2DMap.clear();
}

bool OutputDataPort::isEmpty() const
{
    return !hasData(CAMERA_DATA_POINT_CLOUD) && outputData2DMap.isEmpty();
}

bool OutputDataPort::hasData(CS_CAMERA_DATA_TYPE dataType) const
{
    if (dataType == CAMERA_DATA_POINT_CLOUD)
    {
        return pointCloud.size() > 0;
    }
    else 
    {
        return outputData2DMap.contains(dataType);
    }
}

cs::Pointcloud OutputDataPort::getPointCloud()
{
    return pointCloud;
}

OutputData2D OutputDataPort::getOutputData2D(CS_CAMERA_DATA_TYPE dataType)
{
    if (!hasData(dataType))
    {
        return OutputData2D();
    }

    return outputData2DMap[dataType];
}

QMap<CS_CAMERA_DATA_TYPE, OutputData2D> OutputDataPort::getOutputData2Ds()
{
    return outputData2DMap;
}

FrameData OutputDataPort::getFrameData() const
{
    return frameData;
}

void OutputDataPort::setPointCloud(const cs::Pointcloud& pointCloud)
{
    this->pointCloud = pointCloud;
}

void OutputDataPort::addOutputData2D(const OutputData2D& outputData2D)
{
    CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)outputData2D.info.cameraDataType;
    outputData2DMap[dataType] = outputData2D;
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
    this->frameData = frameData;
}