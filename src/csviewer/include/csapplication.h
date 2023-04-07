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
    void stop();
    std::shared_ptr<ICSCamera> getCamera() const;

    void setCurOutputData(const CameraCaptureConfig& config);
    void startCapture(CameraCaptureConfig config, bool autoName = false);
    void stopCapture();

    std::shared_ptr<AppConfig> getAppConfig();
    bool getShow3DTexture() const;
public slots:
    void onWindowLayoutChanged(QVector<int> windows);
    void onShowCoordChanged(bool show, QPointF pos);
    void onShow3DTextureChanged(bool texture);
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
    void captureStateChanged(int captureType, int state, QString message);

    void show3DTextureChanged(bool texture);
private slots:
    void onCameraStateChanged(int state);
    void onCameraParaUpdated(int paraId, QVariant value);
private:
    CSApplication();    
    void initConnections();
    void disconnections();
    void updateProcessStrategys();
private:
    std::shared_ptr<CameraThread> m_cameraThread;
    std::shared_ptr<Processor> m_processor;
    std::shared_ptr<ProcessThread> m_processThread;
    
    QMap<int, cs::ProcessStrategy*> m_processStrategys;
    std::shared_ptr<CameraCaptureTool> m_cameraCaptureTool;

    std::shared_ptr<AppConfig> m_appConfig;

    bool m_show3DTexture = false;
};
}

#endif // _CS_CSAPPLICATION_H
