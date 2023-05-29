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
    : m_processor(std::make_shared<Processor>())
    , m_processThread(std::make_shared<ProcessThread>(m_processor))
    , m_cameraCaptureTool(std::make_shared<CameraCaptureTool>())
    , m_appConfig(std::make_shared<AppConfig>())
{
    CameraThread::enableSdkLog(LOG_ROOT_DIR);

    m_cameraThread = std::make_shared<CameraThread>();

    qRegisterMetaType<StreamData>("StreamData");
    qRegisterMetaType<FrameData>("FrameData");
    qRegisterMetaType<OutputData2D>("OutputData2D");
    qRegisterMetaType<cs::Pointcloud>("cs::Pointcloud");

    m_processStrategys[cs::STRATEGY_DEPTH] = nullptr;
    m_processStrategys[cs::STRATEGY_RGB] = nullptr;
    m_processStrategys[cs::STRATEGY_CLOUD_POINT] = nullptr;
}

CSApplication::~CSApplication()
{
    disconnections();

    m_processor->removeProcessEndLisener(m_cameraCaptureTool.get());

    for (auto straType : m_processStrategys.keys())
    {
        auto stra = m_processStrategys[straType];
        if (stra)
        {
            m_processor->removeProcessStrategy(stra);
            m_processStrategys[straType] = nullptr;
            delete stra;
        }
    }

    qDebug() << "~CSApplication";
}

void CSApplication::start()
{
    initConnections();

    m_cameraThread->start();

    auto camera = m_cameraThread->getCamera();
    m_cameraCaptureTool->setCamera(camera);

    m_processor->addProcessEndLisener(m_cameraCaptureTool.get());
}

void CSApplication::stop()
{
    getCamera()->disconnectCamera();
    CameraThread::deInitialize();
}

std::shared_ptr<ICSCamera> CSApplication::getCamera() const
{
    return m_cameraThread->getCamera();
}

void CSApplication::initConnections()
{
    auto cameraProxy = m_cameraThread->getCamera();

    bool suc = true;
    suc &= (bool)connect(m_cameraThread.get(), &CameraThread::cameraListUpdated,    this,  &CSApplication::cameraListUpdated);
    suc &= (bool)connect(m_cameraThread.get(), &CameraThread::cameraStateChanged,   this,  &CSApplication::cameraStateChanged);
    suc &= (bool)connect(m_cameraThread.get(), &CameraThread::cameraStateChanged,   this,  &CSApplication::onCameraStateChanged);
    suc &= (bool)connect(m_cameraThread.get(), &CameraThread::removedCurrentCamera, this,  &CSApplication::removedCurrentCamera);
    suc &= (bool)connect(cameraProxy.get(),  &ICSCamera::cameraParaUpdated,       this, &CSApplication::onCameraParaUpdated);

    suc &= (bool)connect(m_cameraCaptureTool.get(), &CameraCaptureTool::captureNumberUpdated, this, &CSApplication::captureNumberUpdated);
    suc &= (bool)connect(m_cameraCaptureTool.get(), &CameraCaptureTool::captureStateChanged,  this, &CSApplication::captureStateChanged);

    suc &= (bool)connect(this,  &CSApplication::connectCamera,      m_cameraThread.get(),  &CameraThread::onConnectCamera);
    suc &= (bool)connect(this,  &CSApplication::disconnectCamera,   m_cameraThread.get(),  &CameraThread::onDisconnectCamera);
    suc &= (bool)connect(this,  &CSApplication::restartCamera,      m_cameraThread.get(),  &CameraThread::onRestartCamera);
    suc &= (bool)connect(this,  &CSApplication::startStream,        m_cameraThread.get(),  &CameraThread::onStartStream);
    suc &= (bool)connect(this,  &CSApplication::stopStream,         m_cameraThread.get(),  &CameraThread::onStopStream);
    suc &= (bool)connect(this,  &CSApplication::pausedStream,       m_cameraThread.get(),  &CameraThread::onPausedStream);
    suc &= (bool)connect(this,  &CSApplication::resumeStream,       m_cameraThread.get(),  &CameraThread::onResumeStream);
    suc &= (bool)connect(this,  &CSApplication::queryCameras,       m_cameraThread.get(),  &CameraThread::onQueryCameras);
   
    suc &= (bool)connect(cameraProxy.get(),  &ICSCamera::framedDataUpdated,          m_processThread.get(), &ProcessThread::onFrameDataUpdated, Qt::DirectConnection);
    
    Q_ASSERT(suc);
}

void CSApplication::disconnections()
{
    auto cameraProxy = m_cameraThread->getCamera();
    disconnect(cameraProxy.get(), &ICSCamera::framedDataUpdated, m_processThread.get(), &ProcessThread::onFrameDataUpdated);
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
    for (auto straType : m_processStrategys.keys())
    {
        auto stra = m_processStrategys[straType];
        if (stra)
        {
            stra->setCameraParaState(paraId, true);
        }
    }
}

void CSApplication::updateProcessStrategys()
{
    // remove all process strategys
    for (auto straType : m_processStrategys.keys())
    {
        auto stra = m_processStrategys[straType];
        if (stra)
        {
            m_processor->removeProcessStrategy(stra);
            m_processStrategys[straType] = nullptr;
            delete stra;
        }
    }

    m_processStrategys[cs::STRATEGY_CLOUD_POINT] = new PointCloudProcessStrategy();
    m_processStrategys[cs::STRATEGY_DEPTH] = new DepthProcessStrategy();

    QVariant hasRgbV;
    m_cameraThread->getCamera()->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    if (hasRgbV.toBool())
    {
        m_processStrategys[cs::STRATEGY_RGB] = new RgbProcessStrategy();
    }

    // add process strategys
    for (auto straType : m_processStrategys.keys())
    {
        auto stra = m_processStrategys[straType];
        if (stra)
        {
            // init connections
            bool suc = true;
            suc &= (bool)connect(stra, &ProcessStrategy::output2DUpdated, this, &CSApplication::output2DUpdated);
            suc &= (bool)connect(stra, &ProcessStrategy::output3DUpdated, this, &CSApplication::output3DUpdated);

            Q_ASSERT(suc);

            stra->setCamera(m_cameraThread->getCamera());
            m_processor->addProcessStrategy(stra);
        }
    }
}

void CSApplication::onWindowLayoutChanged(QVector<int> windows)
{
    for (auto straType : m_processStrategys.keys())
    {
        auto stra = m_processStrategys[straType];
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
    m_show3DTexture = texture;

    emit show3DTextureChanged(m_show3DTexture);
}

void CSApplication::onShowCoordChanged(bool show, QPointF pos)
{
    auto stra = m_processStrategys[STRATEGY_DEPTH];

    if (stra)
    {
        stra->setProperty("calcDepthCoord", show);
        stra->setProperty("depthCoordCalcPos", pos);
    }
}

void CSApplication::startCapture(CameraCaptureConfig config, bool autoName)
{
    m_cameraCaptureTool->startCapture(config, autoName);
}

void CSApplication::setCurOutputData(const CameraCaptureConfig& config)
{
    m_cameraCaptureTool->setCurOutputData(config);
}

void CSApplication::stopCapture()
{
    m_cameraCaptureTool->stopCapture();
}

std::shared_ptr<AppConfig> CSApplication::getAppConfig()
{
    return m_appConfig;
}

bool CSApplication::getShow3DTexture() const
{
    return m_show3DTexture;
}