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

#ifndef _CS_CAMERATHREAD_H
#define _CS_CAMERATHREAD_H

#include <QThread>
#include <QList>
#include <QReadWriteLock>
#include <memory>
#include <QTimer>

#include "cscameraapi.h"
#include "cscamera.h"

namespace cs {

class CameraProxy;
class ICSCamera;

class CS_CAMERA_EXPORT CameraThread : public QThread
{
    Q_OBJECT
public:
    CameraThread();
    ~CameraThread();
    static void onCameraChanged(std::vector<CameraInfo>& added, std::vector<CameraInfo>& removed, void* user);
    static void onCameraAlarm(const char* jsonData, int iDataLen, void* userData);
    void updateCameraInfoList(const std::vector<CameraInfo>& added, const std::vector<CameraInfo>& removed);
    std::shared_ptr<ICSCamera> getCamera() const;
    
    static void enableSdkLog(QString logDir);
    static void initialize(void* user);
    static void deInitialize();

public slots:
    void onConnectCamera(QString serial);
    void onDisconnectCamera();
    void onReconnectCamera();
    void onRestartCamera();
    void onStartStream();
    void onPausedStream();
    void onResumeStream();
    void onStopStream();

    void onCameraStateChanged(int state);
    void onRestartCameraTimeout();
    void onQueryCameras();
signals:
    void cameraListUpdated(const QStringList cameralist);
    void cameraStateChanged(int state);
    void removedCurrentCamera(QString serial);
    void reconnectCamera();
private:
    void run() override;

    void initConnections();
    
    void onAddCameras(const std::vector<CameraInfo>& added);
    void onRemoveCameras(const std::vector<CameraInfo>& removed);
    void notifyCameraListUpdated();
    
    void unBindCamera();
    void bindCamera(CSCamera* camera);
    
    bool findCameraInfo(const QString& serial, CameraInfo& cameraInfo);
    bool isNetConnect(QString uuid);
    QString splitCameraInfo(QString info);
private:
    std::shared_ptr<ICSCamera> m_cameraProxy;
    QList<CameraInfo> m_cameraInfoList;
    QReadWriteLock m_lock;
    QTimer m_cameraRestartTimer;
};

}
#endif //_CS_CAMERATHREAD_H