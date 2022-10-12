/*******************************************************************************
* This file is part of the 3DViewer
*
* Copyright 2022-2026 (C) Revopoint3D AS
* All rights reserved.
*
* Revopoint3D Software License, v1.0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistribution in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of Revopoint3D AS nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* 4. This software, with or without modification, must not be used with any
* other 3D camera than from Revopoint3D AS.
*
* 5. Any software provided in binary form under this license must not be
* reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY REVOPOINT3D AS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL REVOPOINT3D AS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Info:  https://www.revopoint3d.com
******************************************************************************/

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