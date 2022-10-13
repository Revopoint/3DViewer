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

    if (show3D)
    {
        processStrategys[cs::STRATEGY_CLOUD_POINT] = new PointCloudProcessStrategy();
    }
    else
    {
        processStrategys[cs::STRATEGY_DEPTH] = new DepthProcessStrategy();

        QVariant hasRgbV;
        cameraThread->getCamera()->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
        if (hasRgbV.toBool())
        {
            processStrategys[cs::STRATEGY_RGB] = new RgbProcessStrategy();
        }
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

void CSApplication::onShow3DUpdated(bool show)
{
    show3D = show;
    updateProcessStrategys();
}

void CSApplication::onShow3DTextureChanged(bool texture)
{
    if (processStrategys[STRATEGY_CLOUD_POINT])
    {
        processStrategys[STRATEGY_CLOUD_POINT]->setProperty("withTexture", texture);
    }   
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

void CSApplication::startCapture(CameraCaptureConfig config)
{
    cameraCaptureTool->startCapture(config);
}

void CSApplication::stopCapture()
{
    cameraCaptureTool->stopCapture();
}
