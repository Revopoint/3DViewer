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

#ifndef _CS_DEPTH_PROCESSSTRATEGY_H
#define _CS_DEPTH_PROCESSSTRATEGY_H

#include <QObject>
#include <QPointF>
#include <QPair>
#include <QList>
#include "processstrategy.h"
#include "cscameraapi.h"

namespace cs
{

class CS_CAMERA_EXPORT DepthProcessStrategy : public ProcessStrategy
{
    Q_OBJECT
    Q_PROPERTY(bool calcDepthCoord READ getCalcDepthCoord WRITE setCalcDepthCoord)
    Q_PROPERTY(QPointF depthCoordCalcPos READ getDepthCoordCalcPos WRITE setDepthCoordCalcPos)
public:
    DepthProcessStrategy();
    DepthProcessStrategy(PROCESS_STRA_TYPE type);
    void doProcess(const FrameData& frameData, OutputDataPort& outputDataPort) override;
    void onLoadCameraPara() override;

    bool getCalcDepthCoord() const;
    void setCalcDepthCoord(bool calc);
    QPointF getDepthCoordCalcPos() const;
    void  setDepthCoordCalcPos(QPointF pos);

protected:
    void onProcessDepthData(const ushort* dataPtr, int length, int width, int height, QByteArray& output, QImage& depthImage);
protected:
    float depthScale;
    Intrinsics depthIntrinsics;

private:
    QVector<OutputData2D> onProcessZ16(const StreamData& frameData);
    QVector<OutputData2D> onProcessZ16Y8Y8(const StreamData& frameData);
    QVector<OutputData2D> onProcessPAIR(const StreamData& frameData);
   
    OutputData2D processDepthData(const ushort* dataPtr, int length, int width, int height);
    OutputData2D onProcessLData(const char* dataPtr, int length, int width, int height);
    OutputData2D onProcessRData(const char* dataPtr, int length, int width, int height);

    void timeDomainSmooth(const ushort* dataPtr, int length, int width, int height, float* output);

protected:
    bool calcDepthCoord;
    QPointF depthCoordCalcPos;
    
    // for time domain smooth
    QList<QByteArray> filterCachedData;
    QPair<float, float> depthRange;

    bool fillHole;
    int filterValue;
    int filterType;
};

}

#endif //_CS_DEPTH_PROCESSSTRATEGY_H