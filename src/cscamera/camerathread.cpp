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

#include "camerathread.h"

#include <QDebug>
#include <QReadLocker>
#include <QWriteLocker>

#include "cameraproxy.h"
#include "cscamera.h"

using namespace cs;

#define RESTART_CAMERA_TIME_OUT (120 * 1000) //ms 

CameraThread::CameraThread()
    : cameraProxy(std::make_shared<CameraProxy>())
{
    setObjectName("CameraThread");
    moveToThread(this);
    cameraProxy->moveToThread(this);

    initConnections();

    // restart camera timer
    cameraRestartTimer.moveToThread(this);
    cameraRestartTimer.setSingleShot(true);
    cameraRestartTimer.setInterval(RESTART_CAMERA_TIME_OUT);

    bool suc = true;
    suc &= (bool)connect(&cameraRestartTimer, &QTimer::timeout, this, &CameraThread::onRestartCameraTimeout);
    Q_ASSERT(suc);
}

CameraThread::~CameraThread()
{
    CSCamera::setCameraChangeCallback(nullptr, nullptr);
    CSCamera::setCameraAlarmCallback(nullptr, nullptr);

    disconnect(cameraProxy.get(), &CameraProxy::cameraStateChanged, this, &CameraThread::cameraStateChanged);
    disconnect(this, &CameraThread::cameraStateChanged, this, &CameraThread::onCameraStateChanged);
    disconnect(this, &CameraThread::reconnectCamera, this, &CameraThread::onReconnectCamera);

    quit();
    wait();
    qDebug() << "~CameraThread";
}

void CameraThread::onCameraChanged(std::vector<CameraInfo>& added, std::vector<CameraInfo>& removed, void* user)
{
    CameraThread* cameraThread = static_cast<CameraThread*>(user);
    cameraThread->updateCameraInfoList(added, removed);
}

void CameraThread::onCameraAlarm(const char* jsonData, int iDataLen, void* userData)
{
    qDebug("on alarm %d : %s\n", iDataLen, jsonData);
}

std::shared_ptr<ICSCamera> CameraThread::getCamera() const
{
    return cameraProxy;
}

void CameraThread::updateCameraInfoList(const std::vector<CameraInfo>& added, const std::vector<CameraInfo>& removed)
{
    lock.lockForWrite();

    if (removed.size() > 0)
    {
        onRemoveCameras(removed);
    }

    if (added.size() > 0)
    {
        onAddCameras(added);
    }

    lock.unlock();

    notifyCameraListUpdated();
}

void CameraThread::onAddCameras(const std::vector<CameraInfo>& added)
{
    for (const auto& info : added)
    {
        auto it = cameraInfoList.begin();
        bool contains = false;
        while (it != cameraInfoList.end())
        {
            if (std::string(info.serial) == std::string((*it).serial))
            {
                contains = true;
            }
            it++;
        }

        if (!contains)
        {
            cameraInfoList.push_back(info);

            //connect camera
            const int state = cameraProxy->getCameraState();
            if (state == CAMERA_RESTARTING_CAMERA) 
            {
                auto curInfo = cameraProxy->getCameraInfo().cameraInfo;
                if (std::string(info.serial) == std::string(curInfo.serial))
                {
                    qInfo() << "onAddCameras, try to connect camera, serial = " << info.serial;
                    emit reconnectCamera();
                }
            }     
        }  
    }
}

void CameraThread::onRemoveCameras(const std::vector<CameraInfo>& removed)
{
    for (const auto& info : removed)
    {
        auto it = cameraInfoList.begin();
        while (it != cameraInfoList.end())
        {
            if (std::string(info.serial) == std::string((*it).serial))
            {
                it = cameraInfoList.erase(it);
            }
            else
            {
                ++it;
            }

            CameraInfo curInfo = cameraProxy->getCameraInfo().cameraInfo;
            if (std::string(info.serial) == std::string(curInfo.serial))
            {
                const int state = cameraProxy->getCameraState();

                // do not unbind the camera when restart a camera manually
                if (state != CAMERA_RESTARTING_CAMERA)
                {
                    unBindCamera();
                    emit removedCurrentCamera(curInfo.serial);
                }
            }
        }
    }
}

void CameraThread::initConnections()
{
    bool suc = true;
    suc &= (bool)connect(cameraProxy.get(), &CameraProxy::cameraStateChanged, this, &CameraThread::cameraStateChanged);
    suc &= (bool)connect(this, &CameraThread::cameraStateChanged, this, &CameraThread::onCameraStateChanged, Qt::QueuedConnection);
    suc &= (bool)connect(this, &CameraThread::reconnectCamera,    this, &CameraThread::onReconnectCamera,      Qt::QueuedConnection);

    Q_ASSERT(suc);
}

void CameraThread::initialize()
{
    bool result = true;
    result &= (CSCamera::setCameraChangeCallback(onCameraChanged, this) == 0);
    result &= (CSCamera::setCameraAlarmCallback(onCameraAlarm, this) == 0);

    if (!result)
    {
        qDebug() << "set camera callback failed.";
        Q_ASSERT(result);
    }
}

void CameraThread::setSdkLogPath(QString path)
{
    CSCamera::setSdkLogPath(path);
}

void CameraThread::run()
{
    initialize();
    exec();
}

void CameraThread::unBindCamera()
{
    qInfo() << "try to unbind camera";
    qobject_cast<CameraProxy*>(cameraProxy.get())->unBindCamera();
    qInfo() << "unbind camera end";

    emit cameraStateChanged(CAMERA_DISCONNECTED);
}

void CameraThread::bindCamera(CSCamera* camera)
{
    //bind thread
    camera->setCameraThread(this);
    //bind the camera to proxy
    qobject_cast<CameraProxy*>(cameraProxy.get())->bindCamera(camera);
}

void CameraThread::onDisconnectCamera()
{
    qInfo() << "try to disconnect camera";
    unBindCamera();
}

void CameraThread::onReconnectCamera()
{
    qInfo() << "try to reconnect camera";

    if (cameraProxy->reconnectCamera())
    {
        qInfo() << "reconnect camera success";
    }

    cameraRestartTimer.stop();
}

// notify connect failed when time out
void CameraThread::onRestartCameraTimeout()
{
    qInfo() << "restart camera time out";
    unBindCamera();
}

// do not connect a camera in UI thread
void CameraThread::onConnectCamera(QString info)
{
    qInfo() << "try connect to camera, serial : " << info;
    // 1. notify connection failure if info is empty
    if (info.isEmpty())
    {
        qInfo() << "try to connect camera failed, serial is empty";
        emit cameraStateChanged(CAMERA_CONNECTFAILED);
        return;
    }

    // 2. remove ip information
    QString realSerial = splitCameraInfo(info);
    CameraInfo cameraInfo;
    bool result =  findCameraInfo(realSerial, cameraInfo);
    if (!result) {
        qDebug() << "connect camera failed, do not find camera info, serial : " << realSerial;
        emit cameraStateChanged(CAMERA_CONNECTFAILED);
        return;
    }

    // 3. not to connect if already connected
    QString curSerial = QString(cameraProxy->getCameraInfo().cameraInfo.serial);
    if (curSerial == realSerial)
    {
        qDebug() << "already connected, serial = " << realSerial;
        return;
    }
    
    // 4. connecting
    emit cameraStateChanged(CAMERA_CONNECTING);
    qInfo() << "begin connect camera";
    CSCamera* camera = new CSCamera();
    if (!camera->connectCamera(cameraInfo))
    {
        camera->deleteLater();
        emit cameraStateChanged(CAMERA_CONNECTFAILED);
        return;
    }

    // 5. connect success, then bind the camera 
    bindCamera(camera);
    emit cameraStateChanged(CAMERA_CONNECTED);
    qInfo() << "connect camera end";
}

void CameraThread::onRestartCamera()
{
    qInfo() << "try to restart camera";
    cameraProxy->restartCamera();
    qInfo() << "restart camera end";
}

void CameraThread::onStartStream()
{
    qInfo() << "begin start stream";
    cameraProxy->startStream();
    qInfo() << "start stream end";
}

void CameraThread::onStopStream()
{
    qInfo() << "begin stop stream";
    cameraProxy->stopStream();
    qInfo() << "start stop end";
}

void CameraThread::onPausedStream()
{
    qInfo() << "begin pause stream";
    cameraProxy->pauseStream();
    qInfo() << "pause stream end";
}

void CameraThread::onResumeStream()
{
    qInfo() << "begin resume stream";
    cameraProxy->resumeStream();
    qInfo() << "resume stream end";
}

void CameraThread::onCameraStateChanged(int state)
{
   CAMERA_STATE cameraState = (CAMERA_STATE)state;
   qInfo() << "camera state changed, state = " << cameraState;

   switch (cameraState)
   {
   case CAMERA_DISCONNECTFAILED:
       unBindCamera();
       break;
   case CAMERA_RESTARTING_CAMERA:
       qInfo() << "start restart camera timer.";
       cameraRestartTimer.start();
       break;
   default:
       break;
   }
}

void CameraThread::onQueryCameras()
{
    // 1. query cameras
    std::vector<CameraInfo> cameras;
    CSCamera::queryCameras(cameras);

    lock.lockForWrite();

    // 2. if not connecting a camera, set find is true
    auto curCameraSerial = QString(cameraProxy->getCameraInfo().cameraInfo.serial);
    bool find = curCameraSerial.isEmpty();

    // 3. update cameraInfoList
    cameraInfoList.clear();
    for (auto info : cameras)
    {
        cameraInfoList.push_back(info);
        // find connected camera
        if (!find && curCameraSerial == QString(info.serial))
        {
            find = true;
        }
    }

    // 4. unbind the camera if not find 
    if (!find)
    {
        unBindCamera();
    }
    lock.unlock();

    // 5. notify the camera list updated
    notifyCameraListUpdated();
}

bool CameraThread::findCameraInfo(const QString& serial, CameraInfo& cameraInfo)
{
    QReadLocker locker(&lock);
    for (const auto& info : cameraInfoList)
    {
        if (QString(info.serial) == serial) {
            cameraInfo = info;
            return true;
        }
    }
    return false;
}

void CameraThread::notifyCameraListUpdated()
{
    lock.lockForRead();
    // notify camera list updated
    QStringList infoList;
    for (const auto& info : cameraInfoList)
    {
        QString str = isNetConnect(info.uniqueId) ? (QString("%1(%2)").arg(info.serial).arg(info.uniqueId)): info.serial;
        infoList << str;
    }

    lock.unlock();
    emit cameraListUpdated(infoList);
}

bool CameraThread::isNetConnect(QString uuid)
{
    return uuid.contains(".");
}

QString CameraThread::splitCameraInfo(QString info)
{
    return info.split("(").first();
}