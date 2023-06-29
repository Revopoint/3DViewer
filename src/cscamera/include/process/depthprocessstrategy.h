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
    bool onProcessDepthData(const ushort* dataPtr, int length, int width, int height, QByteArray& output);
    void generateDepthImage(const QByteArray& output, int width, int height, QImage& depthImage);
protected:
    float m_depthScale;
    Intrinsics m_depthIntrinsics;

private:
    QVector<OutputData2D> onProcessZ16(const StreamData& frameData);
    QVector<OutputData2D> onProcessZ16Y8Y8(const StreamData& frameData);
    QVector<OutputData2D> onProcessPAIR(const StreamData& frameData);
   
    OutputData2D processDepthData(const ushort* dataPtr, int length, int width, int height);
    OutputData2D onProcessLData(const char* dataPtr, int length, int width, int height);
    OutputData2D onProcessRData(const char* dataPtr, int length, int width, int height);

    bool timeDomainSmooth(float* dataPtr, int length, int width, int height);

protected:
    bool m_calcDepthCoord;
    QPointF m_depthCoordCalcPos;
    
    // for time domain smooth
    QList<QByteArray> m_filterCachedData;
    QPair<float, float> m_depthRange;

    bool m_fillHole;
    int m_filterValue;
    int m_filterType;

    TRIGGER_MODE m_trigger = TRIGGER_MODE_OFF;
    cs::colorizer m_colorizer;
};

}

#endif //_CS_DEPTH_PROCESSSTRATEGY_H