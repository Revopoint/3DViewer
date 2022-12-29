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

#include "process/processthread.h"

#include <QDebug>
#include <QMutexLocker>

#include "process/processor.h"

#define MAX_CACHED_FRAME 5

using namespace cs;

ProcessThread::ProcessThread(std::shared_ptr<Processor> processor)
    : processorPtr(processor)
{
    setObjectName("ProcessThread");
    moveToThread(this);
}

ProcessThread::~ProcessThread()
{
    requestInterruption();
    wait();
    qDebug() << "~ProcessThread";
}

void ProcessThread::run()
{
    while(!isInterruptionRequested())
    {
        bool hasData = false;
        FrameData data;

        mutex.lock();
        if (!cachedFrameData.isEmpty()) {
            data = cachedFrameData.dequeue();
            hasData = true;
        }
        mutex.unlock();

        if (hasData)
        {
            processorPtr->process(data);
        }
        else 
        {
            QThread::msleep(2);
        }
    }
}

// exec not on ProcessThread
void ProcessThread::onFrameDataUpdated(FrameData frameData)
{
    QMutexLocker locker(&mutex);
    if (!isRunning()) 
    {
        start();
    }

    if (cachedFrameData.size() >= MAX_CACHED_FRAME)
    {
        qWarning() << "dequeue, skip one frame";
        cachedFrameData.dequeue();
    }

    cachedFrameData.enqueue(frameData);
}