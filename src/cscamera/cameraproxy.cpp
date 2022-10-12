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

#include "cameraproxy.h"

#include <QDebug>

using namespace cs;
using namespace cs::parameter;

CameraProxy::CameraProxy()
{
}

CameraProxy::~CameraProxy()
{
    if (csCamera) {
        delete csCamera;
    }

    qDebug() << "~CameraProxy";
}

bool CameraProxy::startStream()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->startStream();
    }
    lock.unlock();

    return result;
}

bool CameraProxy::stopStream()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->stopStream();
    }
    lock.unlock();

    return result;
}

bool CameraProxy::restartStream()
{
    bool result = true;
    lock.lockForRead();

    if (csCamera)
    {
        result =  csCamera->restartStream();
    }
    lock.unlock();

    return result;
}

bool CameraProxy::restartCamera()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->restartCamera();
    }
    lock.unlock();

    return result;
}

bool CameraProxy::disconnectCamera()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->disconnectCamera();
    }
    lock.unlock();

    return true;
}

bool CameraProxy::reconnectCamera()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->reconnectCamera();
    }
    lock.unlock();

    return true;
}

bool CameraProxy::pauseStream()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->pauseStream();
    }
    lock.unlock();

    return result;
}

bool CameraProxy::resumeStream()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->resumeStream();
    }
    lock.unlock();

    return result;
}

bool CameraProxy::softTrigger()
{
    lock.lockForRead();
    bool result = false;
    if (csCamera)
    {
        result = csCamera->softTrigger();
    }
    lock.unlock();

    return result;
}

void CameraProxy::bindCamera(ICSCamera* camera)
{
    unBindCamera();

    lock.lockForWrite();
    csCamera = camera;

    bool suc = true;
    suc &= (bool)connect(csCamera, &ICSCamera::cameraStateChanged,         this, &ICSCamera::cameraStateChanged);
    suc &= (bool)connect(csCamera, &ICSCamera::framedDataUpdated,          this, &ICSCamera::framedDataUpdated,          Qt::DirectConnection);
    suc &= (bool)connect(csCamera, &ICSCamera::cameraParaUpdated,          this, &ICSCamera::cameraParaUpdated,          Qt::QueuedConnection);
    suc &= (bool)connect(csCamera, &ICSCamera::cameraParaRangeUpdated,     this, &ICSCamera::cameraParaRangeUpdated,     Qt::QueuedConnection);
    suc &= (bool)connect(csCamera, &ICSCamera::cameraParaItemsUpdated,     this, &ICSCamera::cameraParaItemsUpdated,     Qt::QueuedConnection);

    Q_ASSERT(suc);
    lock.unlock();
}

void CameraProxy::unBindCamera()
{
    lock.lockForWrite();
    if (csCamera)
    {
        delete csCamera;
        csCamera = nullptr;
    }
    lock.unlock();
}

CSCameraInfo CameraProxy::getCameraInfo() const
{
    CSCameraInfo info = { "", 0, {"", "", "", "", ""}, "" };
    lock.lockForRead();
    if (csCamera)
    {
        info = csCamera->getCameraInfo();
    }
    lock.unlock();

    return info;
}

int CameraProxy::getCameraState() const
{
    int result = -1;

    lock.lockForRead();
    if (csCamera)
    {
        result = csCamera->getCameraState();
    }
    lock.unlock();

    return result;
}

void CameraProxy::getCameraPara(CAMERA_PARA_ID paraId, QVariant& value)
{
    lock.lockForRead();
    if (csCamera)
    {
        csCamera->getCameraPara(paraId, value);
    }
    lock.unlock();
}

void CameraProxy::setCameraPara(CAMERA_PARA_ID paraId, QVariant value)
{
    lock.lockForRead();
    if (csCamera)
    {
        csCamera->setCameraPara(paraId, value);
    }
    lock.unlock();
}

void CameraProxy::getCameraParaRange(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step)
{
    lock.lockForRead();
    if (csCamera)
    {
        csCamera->getCameraParaRange(paraId, min, max, step);
    }
    lock.unlock();
}

void CameraProxy::getCameraParaItems(CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list)
{
    lock.lockForRead();
    if (csCamera)
    {
        csCamera->getCameraParaItems(paraId, list);
    }
    lock.unlock();
}