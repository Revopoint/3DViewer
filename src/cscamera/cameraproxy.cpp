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