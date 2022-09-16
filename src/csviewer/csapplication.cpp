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
#include "dataexporter.h"

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
    , dataExporter(std::make_shared<DataExporter>())
{
    CameraThread::setSdkLogPath(LOG_ROOT_DIR + "/sdk.log");
    cameraThread = std::make_shared<CameraThread>();

    qRegisterMetaType<StreamData>("StreamData");
    qRegisterMetaType<FrameData>("FrameData");
    qRegisterMetaType<OutputData2D>("OutputData2D");
    qRegisterMetaType<cs::Pointcloud>("cs::Pointcloud");
}

CSApplication::~CSApplication()
{
    disconnections();
    qDebug() << "~CSApplication";
}

void CSApplication::start()
{
    initConnections();
    processor->setCamera(getCamera());

    cameraThread->start();
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
    suc &= (bool)connect(cameraThread.get(), &CameraThread::removedCurrentCamera, this, &CSApplication::removedCurrentCamera);

    suc &= (bool)connect(processor.get(),    &Processor::output2DUpdated,        this,  &CSApplication::onOutput2DUpdated);
    suc &= (bool)connect(processor.get(),    &Processor::output3DUpdated,        this,  &CSApplication::onOutput3DUpdated);

    suc &= (bool)connect(this,  &CSApplication::connectCamera,      cameraThread.get(),  &CameraThread::onConnectCamera);
    suc &= (bool)connect(this,  &CSApplication::disconnectCamera,   cameraThread.get(),  &CameraThread::onDisconnectCamera);
    suc &= (bool)connect(this,  &CSApplication::restartCamera,      cameraThread.get(),  &CameraThread::onRestartCamera);
    suc &= (bool)connect(this,  &CSApplication::startStream,        cameraThread.get(),  &CameraThread::onStartStream);
    suc &= (bool)connect(this,  &CSApplication::pausedStream,       cameraThread.get(),  &CameraThread::onPausedStream);
    suc &= (bool)connect(this,  &CSApplication::resumeStream,       cameraThread.get(),  &CameraThread::onResumeStream);
    suc &= (bool)connect(this,  &CSApplication::queryCameras,       cameraThread.get(),  &CameraThread::onQueryCameras);
   
    suc &= (bool)connect(cameraProxy.get(),  &ICSCamera::framedDataUpdated,              processThread.get(), &ProcessThread::onFrameDataUpdated, Qt::DirectConnection);
    suc &= (bool)connect(cameraProxy.get(),  SIGNAL(showDepthCoordChanged(bool)),        processor.get(),     SLOT(onShowDepthCoordChanged(bool)));
    suc &= (bool)connect(cameraProxy.get(),  SIGNAL(showDepthCoordPosChanged(QPointF)),  processor.get(),     SLOT(onShowDepthCoordPosChanged(QPointF)));
    suc &= (bool)connect(cameraProxy.get(),  SIGNAL(showRender3DChanged(bool)),          processor.get(),     SLOT(onShowRender3DChanged(bool)));
    suc &= (bool)connect(cameraProxy.get(),  SIGNAL(show3DWithTextureChanged(bool)),     processor.get(),     SLOT(onShow3DWithTextureChanged(bool)));

    suc &= (bool)connect(cameraProxy.get(),  &ICSCamera::cameraParaUpdated,              processor.get(),     &Processor::onCameraParaUpdated);

    suc &= (bool)connect(this, &CSApplication::exportStreamData, dataExporter.get(), &DataExporter::onExportStreamData);
    suc &= (bool)connect(this, &CSApplication::exportPointCloud, dataExporter.get(), &DataExporter::onExportPointCloud);
    suc &= (bool)connect(dataExporter.get(), &DataExporter::exportFinished, this,    &CSApplication::exportFinished);

    Q_ASSERT(suc);
}

void CSApplication::disconnections()
{
    auto cameraProxy = cameraThread->getCamera();
    disconnect(cameraProxy.get(), &ICSCamera::framedDataUpdated, processThread.get(), &ProcessThread::onFrameDataUpdated);
}

void CSApplication::onOutput2DUpdated(OutputData2D outputData, StreamData streamData)
{
    // cache data, only depth and rgb
    const int cameraDataType = outputData.info.cameraDataType;
    if (cameraDataType == CAMERA_DATA_DEPTH || cameraDataType == (CAMERA_DATA_R | CAMERA_DATA_L))
    {
        cachedDepthData = streamData;
        cachedDepthImage = outputData.image;
    }
    else if(cameraDataType == CAMERA_DATA_RGB)
    {
        cachedRgbData = streamData;
        cachedRgbImage = outputData.image;
    }

    emit output2DUpdated(outputData);
}

void CSApplication::onOutput3DUpdated(cs::Pointcloud pointCloud, const QImage& image, StreamData streamData)
{
    cachedPointcloud = pointCloud;
    cachedTextureImage = image;
    emit output3DUpdated(pointCloud, image);
}

void CSApplication::onExportPointCloud(QString filePath)
{
    emit exportPointCloud(cachedPointcloud, cachedTextureImage, filePath);
}

void CSApplication::onExportDepthData(QString filePath)
{
    emit exportStreamData(cachedDepthData, cachedDepthImage, filePath);
}

void CSApplication::onExportRgbData(QString filePath)
{
    emit exportStreamData(cachedRgbData, cachedRgbImage, filePath);
}
