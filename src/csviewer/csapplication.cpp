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

#include "csapplication.h"

#include <QMetaType>
#include <QDebug>
#include <QDir>

#include "camerathread.h"
#include "cameraproxy.h"
#include "process/processthread.h" 
#include "process/processor.h"
#include "app_version.h"
#include "cameracapturetool.h"
#include "appconfig.h"

#include "process/pointcloudprocessstrategy.h"
#include "process/depthprocessstrategy.h"
#include "process/rgbprocessstrategy.h"

using namespace cs;

Q_DECLARE_METATYPE(cs::Pointcloud);

CSApplication* CSApplication::getInstance()
{
    static CSApplication app;
    return &app;
}

CSApplication::CSApplication()
    : processor(std::make_shared<Processor>())
    , processThread(std::make_shared<ProcessThread>(processor))
    , cameraCaptureTool(std::make_shared<CameraCaptureTool>())
    , appConfig(std::make_shared<AppConfig>())
{
    CameraThread::setSdkLogPath(LOG_ROOT_DIR + "/sdk.log");
    cameraThread = std::make_shared<CameraThread>();

    qRegisterMetaType<StreamData>("StreamData");
    qRegisterMetaType<FrameData>("FrameData");
    qRegisterMetaType<OutputData2D>("OutputData2D");
    qRegisterMetaType<cs::Pointcloud>("cs::Pointcloud");

    processStrategys[cs::STRATEGY_DEPTH] = nullptr;
    processStrategys[cs::STRATEGY_RGB] = nullptr;
    processStrategys[cs::STRATEGY_CLOUD_POINT] = nullptr;
}

CSApplication::~CSApplication()
{
    disconnections();

    processor->removeProcessEndLisener(cameraCaptureTool.get());

    for (auto straType : processStrategys.keys())
    {
        auto stra = processStrategys[straType];
        if (stra)
        {
            processor->removeProcessStrategy(stra);
            processStrategys[straType] = nullptr;
            delete stra;
        }
    }

    qDebug() << "~CSApplication";
}

void CSApplication::start()
{
    initConnections();
    cameraThread->start();

    auto camera = cameraThread->getCamera();
    cameraCaptureTool->setCamera(camera);

    processor->addProcessEndLisener(cameraCaptureTool.get());
}

std::shared_ptr<ICSCamera> CSApplication::getCamera() const
{
    return cameraThread->getCamera();
}

void CSApplication::initConnections()
{
    auto cameraProxy = cameraThread->getCamera();

    bool suc = true;
    suc &= (bool)connect(cameraThread.get(), &CameraThread::cameraListUpdated,    this,  &CSApplication::cameraListUpdated);
    suc &= (bool)connect(cameraThread.get(), &CameraThread::cameraStateChanged,   this,  &CSApplication::cameraStateChanged);
    suc &= (bool)connect(cameraThread.get(), &CameraThread::cameraStateChanged,   this,  &CSApplication::onCameraStateChanged);
    suc &= (bool)connect(cameraThread.get(), &CameraThread::removedCurrentCamera, this,  &CSApplication::removedCurrentCamera);
    suc &= (bool)connect(cameraProxy.get(),  &ICSCamera::cameraParaUpdated,       this, &CSApplication::onCameraParaUpdated);

    suc &= (bool)connect(cameraCaptureTool.get(), &CameraCaptureTool::captureNumberUpdated, this, &CSApplication::captureNumberUpdated);
    suc &= (bool)connect(cameraCaptureTool.get(), &CameraCaptureTool::captureStateChanged,  this, &CSApplication::captureStateChanged);

    suc &= (bool)connect(this,  &CSApplication::connectCamera,      cameraThread.get(),  &CameraThread::onConnectCamera);
    suc &= (bool)connect(this,  &CSApplication::disconnectCamera,   cameraThread.get(),  &CameraThread::onDisconnectCamera);
    suc &= (bool)connect(this,  &CSApplication::restartCamera,      cameraThread.get(),  &CameraThread::onRestartCamera);
    suc &= (bool)connect(this,  &CSApplication::startStream,        cameraThread.get(),  &CameraThread::onStartStream);
    suc &= (bool)connect(this,  &CSApplication::stopStream,         cameraThread.get(),  &CameraThread::onStopStream);
    suc &= (bool)connect(this,  &CSApplication::pausedStream,       cameraThread.get(),  &CameraThread::onPausedStream);
    suc &= (bool)connect(this,  &CSApplication::resumeStream,       cameraThread.get(),  &CameraThread::onResumeStream);
    suc &= (bool)connect(this,  &CSApplication::queryCameras,       cameraThread.get(),  &CameraThread::onQueryCameras);
   
    suc &= (bool)connect(cameraProxy.get(),  &ICSCamera::framedDataUpdated,          processThread.get(), &ProcessThread::onFrameDataUpdated, Qt::DirectConnection);
    
    Q_ASSERT(suc);
}

void CSApplication::disconnections()
{
    auto cameraProxy = cameraThread->getCamera();
    disconnect(cameraProxy.get(), &ICSCamera::framedDataUpdated, processThread.get(), &ProcessThread::onFrameDataUpdated);
}

void CSApplication::onCameraStateChanged(int state)
{
    CAMERA_STATE cameraState = (CAMERA_STATE)state;
    switch (cameraState)
    {
    case CAMERA_CONNECTED:
        updateProcessStrategys();
        break;
    default:
        break;
    }
}

void CSApplication::onCameraParaUpdated(int paraId, QVariant value)
{
    for (auto straType : processStrategys.keys())
    {
        auto stra = processStrategys[straType];
        if (stra)
        {
            stra->setCameraParaState(true);
        }
    }
}

void CSApplication::updateProcessStrategys()
{
    // remove all process strategys
    for (auto straType : processStrategys.keys())
    {
        auto stra = processStrategys[straType];
        if (stra)
        {
            processor->removeProcessStrategy(stra);
            processStrategys[straType] = nullptr;
            delete stra;
        }
    }

    processStrategys[cs::STRATEGY_CLOUD_POINT] = new PointCloudProcessStrategy();
    processStrategys[cs::STRATEGY_DEPTH] = new DepthProcessStrategy();

    QVariant hasRgbV;
    cameraThread->getCamera()->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    if (hasRgbV.toBool())
    {
        processStrategys[cs::STRATEGY_RGB] = new RgbProcessStrategy();
    }

    // add process strategys
    for (auto straType : processStrategys.keys())
    {
        auto stra = processStrategys[straType];
        if (stra)
        {
            // init connections
            bool suc = true;
            suc &= (bool)connect(stra, &ProcessStrategy::output2DUpdated, this, &CSApplication::output2DUpdated);
            suc &= (bool)connect(stra, &ProcessStrategy::output3DUpdated, this, &CSApplication::output3DUpdated);

            Q_ASSERT(suc);

            stra->setCamera(cameraThread->getCamera());
            processor->addProcessStrategy(stra);
        }
    }
}

void CSApplication::onWindowLayoutChanged(QVector<int> windows)
{
    for (auto straType : processStrategys.keys())
    {
        auto stra = processStrategys[straType];
        if (stra)
        {
            if (straType == STRATEGY_DEPTH)
            {
                bool enable = windows.contains(CAMERA_DATA_L) || windows.contains(CAMERA_DATA_R) || windows.contains(CAMERA_DATA_DEPTH);
                stra->setStrategyEnable(enable);
            }
            else if (straType == STRATEGY_CLOUD_POINT)
            {
                bool enable = windows.contains(CAMERA_DATA_POINT_CLOUD);
                stra->setStrategyEnable(enable);
            }
            else if (straType == STRATEGY_RGB)
            {
                stra->setStrategyEnable(windows.contains(CAMERA_DATA_RGB));
            }
        }
    }
}

void CSApplication::onShow3DTextureChanged(bool texture)
{
    show3DTexture = texture;

    emit show3DTextureChanged(show3DTexture);
}

void CSApplication::onShowCoordChanged(bool show, QPointF pos)
{
    auto stra = processStrategys[STRATEGY_DEPTH];

    if (stra)
    {
        stra->setProperty("calcDepthCoord", show);
        stra->setProperty("depthCoordCalcPos", pos);
    }
}

void CSApplication::startCapture(CameraCaptureConfig config, bool autoName)
{
    cameraCaptureTool->startCapture(config, autoName);
}

void CSApplication::setCurOutputData(const CameraCaptureConfig& config)
{
    cameraCaptureTool->setCurOutputData(config);
}

void CSApplication::stopCapture()
{
    cameraCaptureTool->stopCapture();
}

std::shared_ptr<AppConfig> CSApplication::getAppConfig()
{
    return appConfig;
}

bool CSApplication::getShow3DTexture() const
{
    return show3DTexture;
}