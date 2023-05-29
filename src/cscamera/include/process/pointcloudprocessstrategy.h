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

#ifndef _CS_POINTCLOUD_PROCESSSTRATEGY_H
#define _CS_POINTCLOUD_PROCESSSTRATEGY_H

#include <QObject>
#include "processstrategy.h"
#include "depthprocessstrategy.h"
#include "cscameraapi.h"

namespace cs
{

class CS_CAMERA_EXPORT PointCloudProcessStrategy : public DepthProcessStrategy
{
    Q_OBJECT
    Q_PROPERTY(bool withTexture READ getWithTexture WRITE setWithTexture)
public:
    PointCloudProcessStrategy();
    void doProcess(const FrameData& frameData, OutputDataPort& outputDataPort) override;
    void onLoadCameraPara() override;

    bool getWithTexture() const;
    void setWithTexture(bool with);
private:
    void generatePointCloud(const StreamData& depthData, Pointcloud& pc);
    void generateTexture(const StreamData& rgbData, QImage& texImage);
private:
    Intrinsics m_rgbIntrinsics;
    Extrinsics m_extrinsics;
    bool m_withTexture;
};

}

#endif //_CS_POINTCLOUD_PROCESSSTRATEGY_H