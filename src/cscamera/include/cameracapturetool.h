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

#ifndef _CS_CAMERA_CAPTURETOOL_H
#define _CS_CAMERA_CAPTURETOOL_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QString>
#include <QThreadPool>

#include "cstypes.h"
#include "cscameraapi.h"
#include "process/processor.h"

namespace cs 
{
class ICSCamera;

enum CAPTURE_TYPE
{
    CAPTURE_TYPE_SINGLE,
    CAPTURE_TYPE_MULTIPLE,
    CAPTURE_TYPE_COUNT
};

enum CAPTURE_STATE
{
    CAPTURING,
    CAPTURE_FINISHED,
    CAPTURE_WARNING,
    CAPTURE_ERROR
};
class CameraCaptureBase : public QThread
{
    Q_OBJECT
public:
    CameraCaptureBase(const CameraCaptureConfig& config, CAPTURE_TYPE captureType);

    CAPTURE_TYPE getCaptureType() const;
    virtual void addOutputData(const OutputDataPort& outputDataPort) {}
    virtual void setOutputData(const OutputDataPort& outputDataPort);
    
    virtual void getCaptureIndex(OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex) {}

    void run() override;
    void saveFinished();

    void setCamera(std::shared_ptr<ICSCamera>& camera);
signals:
    void captureStateChanged(int state, QString message);
    void captureNumberUpdated(int, int);
protected:
    virtual void onCaptureDataDone();
    void saveCameraPara();

protected:
    CameraCaptureConfig captureConfig;
    CAPTURE_TYPE captureType;
    // cached data count
    int cachedDataCount = 0;
    // not cached data count: skipDataCount + cachedDataCount <= captureConfig.captureNumber
    int skipDataCount = 0;

    // captured data count
    int capturedDataCount = 0;

    // cached data
    QQueue<OutputDataPort> outputDatas;
    QMutex mutex;
    QMutex saverMutex;

    const int maxCachedCount = 10;
    bool captureFinished = false;

    QThreadPool threadPool;

    std::shared_ptr<ICSCamera> camera;

    // real save folder
    QString realSaveFolder;
};

// save a frame of data
class CameraCaptureSingle : public CameraCaptureBase
{
    Q_OBJECT
public:
    CameraCaptureSingle(const CameraCaptureConfig& config);
    void getCaptureIndex(OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex) override;
protected:

};

// save multi-frame data
class CameraCaptureMultiple : public CameraCaptureBase
{
    Q_OBJECT
public:
    CameraCaptureMultiple(const CameraCaptureConfig& config);
    void addOutputData(const OutputDataPort& outputDataPort) override;
    void getCaptureIndex(OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex) override;

protected:
    void onCaptureDataDone() override;
    void saveTimeStamps();
    void compressToZip();
    QString genTmpSaveDir(QString saveDir);
private:
    int capturedRgbCount = 0;
    int capturedDepthCount = 0;
    int capturePointCloudCount = 0;
    
    // RGB frame time stamps
    QVector<double> rgbTimeStamps;
    // depth frame time stamps
    QVector<double> depthTimeStamps;
};

class CS_CAMERA_EXPORT CameraCaptureTool : public Processor::ProcessEndListener
{
    Q_OBJECT
public:
    CameraCaptureTool();
    ~CameraCaptureTool();
    void process(const OutputDataPort& outputDataPort) override;
    
    void startCapture(CameraCaptureConfig config);
    void setCamera(std::shared_ptr<ICSCamera>& camera);
public slots:
    void stopCapture();
signals:
    void captureNumberUpdated(int captured, int dropped);
    void captureStateChanged(int state, QString message);
private:
    QMutex mutex;
    // for saving data
    OutputDataPort cachedOutputData;
    CameraCaptureBase* cameraCapture = nullptr;
    std::shared_ptr<ICSCamera> camera;
};
}

#endif // _CS_CAMERA_CAPTURETOOL_H