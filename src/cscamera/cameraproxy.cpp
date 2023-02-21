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
    if (m_csCamera) {
        delete m_csCamera;
    }

    qDebug() << "~CameraProxy";
}

bool CameraProxy::startStream()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->startStream();
    }
    m_lock.unlock();

    return result;
}

bool CameraProxy::stopStream()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->stopStream();
    }
    m_lock.unlock();

    return result;
}

bool CameraProxy::restartStream()
{
    bool result = true;
    m_lock.lockForRead();

    if (m_csCamera)
    {
        result =  m_csCamera->restartStream();
    }
    m_lock.unlock();

    return result;
}

bool CameraProxy::restartCamera()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->restartCamera();
    }
    m_lock.unlock();

    return result;
}

bool CameraProxy::disconnectCamera()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->disconnectCamera();
    }
    m_lock.unlock();

    return true;
}

bool CameraProxy::reconnectCamera()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->reconnectCamera();
    }
    m_lock.unlock();

    return true;
}

bool CameraProxy::pauseStream()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->pauseStream();
    }
    m_lock.unlock();

    return result;
}

bool CameraProxy::resumeStream()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->resumeStream();
    }
    m_lock.unlock();

    return result;
}

bool CameraProxy::softTrigger()
{
    m_lock.lockForRead();
    bool result = false;
    if (m_csCamera)
    {
        result = m_csCamera->softTrigger();
    }
    m_lock.unlock();

    return result;
}

void CameraProxy::bindCamera(ICSCamera* camera)
{
    unBindCamera();

    m_lock.lockForWrite();
    m_csCamera = camera;

    bool suc = true;
    suc &= (bool)connect(m_csCamera, &ICSCamera::cameraStateChanged,         this, &ICSCamera::cameraStateChanged);
    suc &= (bool)connect(m_csCamera, &ICSCamera::framedDataUpdated,          this, &ICSCamera::framedDataUpdated,          Qt::DirectConnection);
    suc &= (bool)connect(m_csCamera, &ICSCamera::cameraParaUpdated,          this, &ICSCamera::cameraParaUpdated,          Qt::QueuedConnection);
    suc &= (bool)connect(m_csCamera, &ICSCamera::cameraParaRangeUpdated,     this, &ICSCamera::cameraParaRangeUpdated,     Qt::QueuedConnection);
    suc &= (bool)connect(m_csCamera, &ICSCamera::cameraParaItemsUpdated,     this, &ICSCamera::cameraParaItemsUpdated,     Qt::QueuedConnection);

    Q_ASSERT(suc);
    m_lock.unlock();
}

void CameraProxy::unBindCamera()
{
    m_lock.lockForWrite();
    if (m_csCamera)
    {
        delete m_csCamera;
        m_csCamera = nullptr;
    }
    m_lock.unlock();
}

CSCameraInfo CameraProxy::getCameraInfo() const
{
    CSCameraInfo info = { "", 0, {"", "", "", "", ""}, "" };
    m_lock.lockForRead();
    if (m_csCamera)
    {
        info = m_csCamera->getCameraInfo();
    }
    m_lock.unlock();

    return info;
}

int CameraProxy::getCameraState() const
{
    int result = -1;

    m_lock.lockForRead();
    if (m_csCamera)
    {
        result = m_csCamera->getCameraState();
    }
    m_lock.unlock();

    return result;
}

void CameraProxy::getCameraPara(CAMERA_PARA_ID paraId, QVariant& value)
{
    m_lock.lockForRead();
    if (m_csCamera)
    {
        m_csCamera->getCameraPara(paraId, value);
    }
    m_lock.unlock();
}

void CameraProxy::setCameraPara(CAMERA_PARA_ID paraId, QVariant value)
{
    m_lock.lockForRead();
    if (m_csCamera)
    {
        m_csCamera->setCameraPara(paraId, value);
    }
    m_lock.unlock();
}

void CameraProxy::getCameraParaRange(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step)
{
    m_lock.lockForRead();
    if (m_csCamera)
    {
        m_csCamera->getCameraParaRange(paraId, min, max, step);
    }
    m_lock.unlock();
}

void CameraProxy::getCameraParaItems(CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list)
{
    m_lock.lockForRead();
    if (m_csCamera)
    {
        m_csCamera->getCameraParaItems(paraId, list);
    }
    m_lock.unlock();
}