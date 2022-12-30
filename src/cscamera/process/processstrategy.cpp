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
    , isCameraParaDirty(true)
    , strategyType(type)
{

}

void ProcessStrategy::setCamera(std::shared_ptr<ICSCamera> camera)
{
    cameraPtr = camera;
}

void ProcessStrategy::setCameraParaState(bool dirty)
{
    QMutexLocker locker(&mutex);
    isCameraParaDirty = dirty;
}

void ProcessStrategy::process(const FrameData& frameData, OutputDataPort& outputDataPort)
{
    bool dirty = false;
    
    mutex.lock();
    dirty = isCameraParaDirty;
    isCameraParaDirty = false;
    mutex.unlock();

    if (dirty)
    {
        onLoadCameraPara();
    }

    doProcess(frameData, outputDataPort);
}

PROCESS_STRA_TYPE ProcessStrategy::getProcessStraType()
{
    return strategyType;
}

void ProcessStrategy::setStrategyEnable(bool enable)
{
    strategyEnable = enable;
}

int ProcessStrategy::isStrategyEnable()
{
    return strategyEnable;
}