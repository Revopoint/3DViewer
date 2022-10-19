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
    
    static void setSdkLogPath(QString path);
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
    void initialize();
    
    void onAddCameras(const std::vector<CameraInfo>& added);
    void onRemoveCameras(const std::vector<CameraInfo>& removed);
    void notifyCameraListUpdated();
    
    void unBindCamera();
    void bindCamera(CSCamera* camera);
    
    bool findCameraInfo(const QString& serial, CameraInfo& cameraInfo);
    bool isNetConnect(QString uuid);
    QString splitCameraInfo(QString info);
private:
    std::shared_ptr<ICSCamera> cameraProxy;
    QList<CameraInfo> cameraInfoList;
    QReadWriteLock lock;
    QTimer cameraRestartTimer;
};

}
#endif //_CS_CAMERATHREAD_H