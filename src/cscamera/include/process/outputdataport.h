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

#ifndef _CS_OUTPUTDATAPORT_H
#define _CS_OUTPUTDATAPORT_H

#include <QMap>

#include "cstypes.h"
#include <hpp/Processing.hpp>

class OutputDataPort
{
public:
    OutputDataPort();
    OutputDataPort(const FrameData& frameData);
    OutputDataPort(const OutputDataPort& other);
    ~OutputDataPort();

    bool isEmpty() const;
    bool hasData(CS_CAMERA_DATA_TYPE dataType) const;

    cs::Pointcloud getPointCloud();
    OutputData2D getOutputData2D(CS_CAMERA_DATA_TYPE dataType);
    QMap<CS_CAMERA_DATA_TYPE, OutputData2D> getOutputData2Ds();
    FrameData getFrameData() const;

    void setFrameData(const FrameData& frameData);
    void setPointCloud(const cs::Pointcloud& pointCloud);
    void addOutputData2D(const OutputData2D& outputData2D);
    void addOutputData2D(const QVector<OutputData2D>& outputData2Ds);
private:
    FrameData m_frameData;
    QMap<CS_CAMERA_DATA_TYPE, OutputData2D> m_outputData2DMap;
    mutable cs::Pointcloud m_pointCloud;
};

#endif // _CS_OUTPUTDATAPORT_H