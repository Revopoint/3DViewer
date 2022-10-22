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

#ifndef _CS_CSAPPLICATION_H
#define _CS_CSAPPLICATION_H
#include <QObject>
#include <QImage>
#include <memory>
#include <cstypes.h>

#include <hpp/Processing.hpp>

class AppConfig;

namespace cs {
    
class CameraThread;
class ProcessThread;
class Processor;
class ICSCamera;
class ProcessStrategy;
class CameraCaptureTool;

class CSApplication : public QObject
{
    Q_OBJECT
public:
    static CSApplication* getInstance();
    ~CSApplication();
    void start();
    std::shared_ptr<ICSCamera> getCamera() const;

    void startCapture(CameraCaptureConfig config);
    void stopCapture();

    std::shared_ptr<AppConfig> getAppConfig();
public slots:
    void onWindowLayoutChanged(QVector<int> windows);
    void onShow3DTextureChanged(bool texture);
    void onShowCoordChanged(bool show, QPointF pos);
signals:
    void cameraListUpdated(const QStringList infoList);
    void connectCamera(QString serial);
    void disconnectCamera();
    void restartCamera();
    void startStream();
    void pausedStream();
    void resumeStream();
    void stopStream();
    void queryCameras();

    void cameraStateChanged(int state);
    void output2DUpdated(OutputData2D outputData);
    void output3DUpdated(cs::Pointcloud pointCloud, const QImage& image);
    void removedCurrentCamera(QString serial);

    // save frame data
    void captureNumberUpdated(int captured, int dropped);
    void captureStateChanged(int state, QString message);

private slots:
    void onCameraStateChanged(int state);
    void onCameraParaUpdated(int paraId, QVariant value);
private:
    CSApplication();    
    void initConnections();
    void disconnections();
    void updateProcessStrategys();
private:
    std::shared_ptr<CameraThread> cameraThread;
    std::shared_ptr<Processor> processor;
    std::shared_ptr<ProcessThread> processThread;
    
    bool show3D = true;
    QMap<int, cs::ProcessStrategy*> processStrategys;
    std::shared_ptr<CameraCaptureTool> cameraCaptureTool;

    std::shared_ptr<AppConfig> appConfig;
};
}

#endif // _CS_CSAPPLICATION_H
