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

#include "process/processstrategy.h"
#include "icscamera.h"

#include <QMutexLocker>

using namespace cs;

ProcessStrategy::ProcessStrategy(PROCESS_STRA_TYPE type)
    : QObject()
    , m_isCameraParaDirty(true)
    , m_strategyType(type)
{

}

void ProcessStrategy::setCamera(std::shared_ptr<ICSCamera> camera)
{
    m_cameraPtr = camera;
}

void ProcessStrategy::setCameraParaState(int paraId, bool dirty)
{
    QMutexLocker locker(&m_mutex);
    if (m_dependentParameters.contains(paraId))
    {
        m_isCameraParaDirty = dirty;
    }
}

void ProcessStrategy::process(const FrameData& frameData, OutputDataPort& outputDataPort)
{
    bool dirty = false;
    
    m_mutex.lock();
    dirty = m_isCameraParaDirty;
    m_isCameraParaDirty = false;
    m_mutex.unlock();

    if (dirty)
    {
        onLoadCameraPara();
    }

    doProcess(frameData, outputDataPort);
}

PROCESS_STRA_TYPE ProcessStrategy::getProcessStraType()
{
    return m_strategyType;
}

void ProcessStrategy::setStrategyEnable(bool enable)
{
    m_strategyEnable = enable;
}

int ProcessStrategy::isStrategyEnable()
{
    return m_strategyEnable;
}