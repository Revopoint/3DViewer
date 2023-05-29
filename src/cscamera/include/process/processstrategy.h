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

#ifndef _CS_PROCESSSTRATEGY_H
#define _CS_PROCESSSTRATEGY_H

#include <QObject>
#include <memory>
#include <QMutex>
#include <QVector>

#include "cstypes.h"
#include "cscameraapi.h"
#include "process/outputdataport.h"

#include <hpp/Processing.hpp>

namespace cs 
{

enum  PROCESS_STRA_TYPE
{
    STRATEGY_DEPTH,
    STRATEGY_RGB,
    STRATEGY_CLOUD_POINT
};

class ICSCamera;
class CS_CAMERA_EXPORT ProcessStrategy : public QObject
{
    Q_OBJECT
public:
    ProcessStrategy(PROCESS_STRA_TYPE type);
    void setCamera(std::shared_ptr<ICSCamera> camera);
    void setCameraParaState(int paraId, bool dirty);

    PROCESS_STRA_TYPE getProcessStraType();
    void process(const FrameData& frameData, OutputDataPort& outputDataPort);
    virtual void onLoadCameraPara() {}
    void setStrategyEnable(bool enable);
    int isStrategyEnable();
signals:
    void output2DUpdated(OutputData2D outputData);
    void output3DUpdated(cs::Pointcloud pointCloud, const QImage& image);

protected:
    virtual void doProcess(const FrameData& frameData, OutputDataPort& outputDataPort) = 0;

    template<typename S, typename T>
    void copyData(S* src, T* dst, int size)
    {
#pragma omp parallel for
        for (int i = 0; i < size; i++)
        {
            dst[i] = src[i];
        }
    }
protected:
    std::shared_ptr<ICSCamera> m_cameraPtr;
    bool m_isCameraParaDirty;
    QMutex m_mutex;
    PROCESS_STRA_TYPE m_strategyType;
    bool m_strategyEnable = true;

    QVector<int> m_dependentParameters;
};

}

#endif //_CS_PROCESSSTRATEGY_H