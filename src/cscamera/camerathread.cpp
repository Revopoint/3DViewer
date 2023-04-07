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

#include "camerathread.h"

#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QReadLocker>
#include <QWriteLocker>

#include "cameraproxy.h"
#include "cscamera.h"

using namespace cs;

#define RESTART_CAMERA_TIME_OUT (120 * 1000) //ms 

CameraThread::CameraThread()
    : m_cameraProxy(std::make_shared<CameraProxy>())
{
    setObjectName("CameraThread");
    moveToThread(this);
    m_cameraProxy->moveToThread(this);

    initConnections();

    // restart camera timer
    m_cameraRestartTimer.moveToThread(this);
    m_cameraRestartTimer.setSingleShot(true);
    m_cameraRestartTimer.setInterval(RESTART_CAMERA_TIME_OUT);

    bool suc = true;
    suc &= (bool)connect(&m_cameraRestartTimer, &QTimer::timeout, this, &CameraThread::onRestartCameraTimeout);
    Q_ASSERT(suc);
}

CameraThread::~CameraThread()
{
    disconnect(m_cameraProxy.get(), &CameraProxy::cameraStateChanged, this, &CameraThread::cameraStateChanged);
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
    return m_cameraProxy;
}

void CameraThread::updateCameraInfoList(const std::vector<CameraInfo>& added, const std::vector<CameraInfo>& removed)
{
    m_lock.lockForWrite();

    if (removed.size() > 0)
    {
        onRemoveCameras(removed);
    }

    if (added.size() > 0)
    {
        onAddCameras(added);
    }

    m_lock.unlock();

    notifyCameraListUpdated();
}

void CameraThread::onAddCameras(const std::vector<CameraInfo>& added)
{
    for (const auto& info : added)
    {
        auto it = m_cameraInfoList.begin();
        bool contains = false;
        while (it != m_cameraInfoList.end())
        {
            if (std::string(info.serial) == std::string((*it).serial))
            {
                contains = true;
            }
            it++;
        }

        if (!contains)
        {
            m_cameraInfoList.push_back(info);

            //connect camera
            const int state = m_cameraProxy->getCameraState();
            if (state == CAMERA_RESTARTING_CAMERA) 
            {
                auto curInfo = m_cameraProxy->getCameraInfo().cameraInfo;
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
        auto it = m_cameraInfoList.begin();
        while (it != m_cameraInfoList.end())
        {
            if (std::string(info.serial) == std::string((*it).serial))
            {
                it = m_cameraInfoList.erase(it);
            }
            else
            {
                ++it;
            }

            CameraInfo curInfo = m_cameraProxy->getCameraInfo().cameraInfo;
            if (std::string(info.serial) == std::string(curInfo.serial))
            {
                const int state = m_cameraProxy->getCameraState();

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
    suc &= (bool)connect(m_cameraProxy.get(), &CameraProxy::cameraStateChanged, this, &CameraThread::cameraStateChanged);
    suc &= (bool)connect(this, &CameraThread::cameraStateChanged, this, &CameraThread::onCameraStateChanged, Qt::QueuedConnection);
    suc &= (bool)connect(this, &CameraThread::reconnectCamera,    this, &CameraThread::onReconnectCamera,      Qt::QueuedConnection);

    Q_ASSERT(suc);
}

void CameraThread::initialize(void* user)
{
    bool result = true;
    result &= (CSCamera::setCameraChangeCallback(onCameraChanged, user) == 0);
    result &= (CSCamera::setCameraAlarmCallback(onCameraAlarm, user) == 0);

    if (!result)
    {
        qDebug() << "set camera callback failed.";
        Q_ASSERT(result);
    }
}

void CameraThread::deInitialize()
{
    bool result = true;
    result &= (CSCamera::setCameraChangeCallback(nullptr, nullptr) == 0);
    result &= (CSCamera::setCameraAlarmCallback(nullptr, nullptr) == 0);

    if (!result)
    {
        qDebug() << "set camera callback failed.";
        Q_ASSERT(result);
    }
}

void CameraThread::enableSdkLog(QString logDir)
{
    QString logPrefix = "3DCameraSDK";
    QString day = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString logFile = logPrefix + "." + day + ".log";

    QDir dir(logDir);
    // check the file exists or not
    if (dir.exists(logFile))
    {
        QStringList filters;
        filters << (logPrefix + ".*.log.*");
        dir.setNameFilters(filters);

        auto fileList = dir.entryList();

        // sort 
        qSort(fileList.begin(), fileList.end(), [](const QString& s1, const QString& s2)
            {
                QStringList ss1 = s1.split(".");
                QStringList ss2 = s2.split(".");
                if (ss1.size() == 4 && ss2.size() == 4)
                {
                    return ss1.at(3).toInt() > ss2.at(3).toInt();
                }

                return false;
            });
          
        foreach(QString file, fileList) {
            QStringList s = file.split(".");
            if (s.size() == 4)
            {
                int index = s.at(3).toInt();
                QString newName = logFile + "." + QString::number(index + 1);
                dir.rename(file, newName);
            }
        }

        QString newName = logFile + "." + QString::number(1);
        dir.rename(logFile, newName);
    }

    // delete logs from a week ago
    QString endDay = QDateTime::currentDateTime().addDays(-7).toString("yyyyMMdd");

    QStringList filters;
    filters << (logPrefix + ".*.log*");
    dir.setNameFilters(filters);

    auto fileList = dir.entryList();
    foreach(QString file, fileList) {
        QStringList s = file.split(".");
        if (s.size() >= 3 && s.at(1).length() == 8 && s.at(1) < endDay) {
            dir.remove(file);
        }
    }

    // set current 3DCamera log
    CSCamera::setSdkLogPath(logDir + QDir::separator() + logFile);
}

void CameraThread::run()
{
    initialize(this);
    exec();
}

void CameraThread::unBindCamera()
{
    qInfo() << "try to unbind camera";
    qobject_cast<CameraProxy*>(m_cameraProxy.get())->unBindCamera();
    qInfo() << "unbind camera end";

    emit cameraStateChanged(CAMERA_DISCONNECTED);
}

void CameraThread::bindCamera(CSCamera* camera)
{
    // bind thread
    camera->setCameraThread(this);
    // bind the camera to proxy
    qobject_cast<CameraProxy*>(m_cameraProxy.get())->bindCamera(camera);
}

void CameraThread::onDisconnectCamera()
{
    qInfo() << "try to disconnect camera";
    unBindCamera();
}

void CameraThread::onReconnectCamera()
{
    qInfo() << "try to reconnect camera";

    if (m_cameraProxy->reconnectCamera())
    {
        qInfo() << "reconnect camera success";
    }

    m_cameraRestartTimer.stop();
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
    QString curSerial = QString(m_cameraProxy->getCameraInfo().cameraInfo.serial);
    if (curSerial == realSerial)
    {
        qDebug() << "already connected, serial = " << realSerial;
        return;
    }
    
    // 4. connecting
    emit cameraStateChanged(CAMERA_CONNECTING);
    qInfo() << "begin connect camera";
    CSCamera* m_camera = new CSCamera();
    if (!m_camera->connectCamera(cameraInfo))
    {
        m_camera->deleteLater();
        emit cameraStateChanged(CAMERA_CONNECTFAILED);
        return;
    }

    // 5. connect success, then bind the camera 
    bindCamera(m_camera);
    emit cameraStateChanged(CAMERA_CONNECTED);
    qInfo() << "connect camera end";
}

void CameraThread::onRestartCamera()
{
    qInfo() << "try to restart camera";
    m_cameraProxy->restartCamera();
    qInfo() << "restart camera end";
}

void CameraThread::onStartStream()
{
    qInfo() << "begin start stream";
    m_cameraProxy->startStream();
    qInfo() << "start stream end";
}

void CameraThread::onStopStream()
{
    qInfo() << "begin stop stream";
    m_cameraProxy->stopStream();
    qInfo() << "start stop end";
}

void CameraThread::onPausedStream()
{
    qInfo() << "begin pause stream";
    m_cameraProxy->pauseStream();
    qInfo() << "pause stream end";
}

void CameraThread::onResumeStream()
{
    qInfo() << "begin resume stream";
    m_cameraProxy->resumeStream();
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
       m_cameraRestartTimer.start();
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

    m_lock.lockForWrite();

    // 2. if not connecting a camera, set find is true
    auto curCameraSerial = QString(m_cameraProxy->getCameraInfo().cameraInfo.serial);
    bool find = curCameraSerial.isEmpty();

    // 3. update cameraInfoList
    m_cameraInfoList.clear();
    for (auto info : cameras)
    {
        m_cameraInfoList.push_back(info);
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
    m_lock.unlock();

    // 5. notify the camera list updated
    notifyCameraListUpdated();
}

bool CameraThread::findCameraInfo(const QString& serial, CameraInfo& cameraInfo)
{
    QReadLocker locker(&m_lock);
    for (const auto& info : m_cameraInfoList)
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
    m_lock.lockForRead();
    // notify camera list updated
    QStringList infoList;
    for (const auto& info : m_cameraInfoList)
    {
        QString str = isNetConnect(info.uniqueId) ? (QString("%1(%2)").arg(info.serial).arg(info.uniqueId)): info.serial;
        infoList << str;
    }

    m_lock.unlock();
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