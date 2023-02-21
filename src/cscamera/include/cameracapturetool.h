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

#ifndef _CS_CAMERA_CAPTURETOOL_H
#define _CS_CAMERA_CAPTURETOOL_H

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QString>
#include <QThreadPool>
#include <QList>

#include "cstypes.h"
#include "cscameraapi.h"
#include "process/processor.h"

namespace cs 
{
class ICSCamera;
class OutputSaver;

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
    
    virtual void getCaptureIndex(const OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex) {}

    void run() override;
    void saveFinished(OutputSaver* saver);

    void setCamera(std::shared_ptr<ICSCamera>& camera);
    void setCameraCaptureConfig(const CameraCaptureConfig& config);
signals:
    void captureStateChanged(int captureType, int state, QString message);
    void captureNumberUpdated(int, int);
protected:
    virtual void onCaptureDataDone();
    void saveCameraPara(QString filePath);
    int getCapturedCount();
    int getSkipCount();

    OutputSaver* genOutputSaver(const OutputDataPort& outputData);
protected:
    CameraCaptureConfig m_captureConfig;
    CAPTURE_TYPE m_captureType;
    // cached data count
    int m_cachedDataCount = 0;
    // not cached data count: skipDataCount + cachedDataCount <= captureConfig.captureNumber
    int m_skipDataCount = 0;

    // captured data count
    int m_capturedDataCount = 0;

    // cached data
    QQueue<OutputSaver*> m_outputDatas;
    QMutex m_mutex;
    QMutex m_saverMutex;
    QList<OutputSaver*> m_outputSaverList;

    const int m_maxCachedCount;
    const int m_maxSavingCount;

    bool m_captureFinished = false;

    QThreadPool m_threadPool;

    std::shared_ptr<ICSCamera> m_camera;

    // real save folder
    QString m_realSaveFolder;
};

// save a frame of data
class CameraCaptureSingle : public CameraCaptureBase
{
    Q_OBJECT
public:
    CameraCaptureSingle(const CameraCaptureConfig& config);
    void getCaptureIndex(const OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex) override;
protected:

};

// save multi-frame data
class CameraCaptureMultiple : public CameraCaptureBase
{
    Q_OBJECT
public:
    CameraCaptureMultiple(const CameraCaptureConfig& config);
    void addOutputData(const OutputDataPort& outputDataPort) override;
    void getCaptureIndex(const OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex) override;

protected:
    void onCaptureDataDone() override;
    void saveTimeStamps();
    void compressToZip();
    QString genTmpSaveDir(QString saveDir);
private:
    int m_capturedRgbCount = 0;
    int m_capturedDepthCount = 0;
    int m_capturePointCloudCount = 0;
    
    // RGB frame time stamps
    QVector<double> m_rgbTimeStamps;
    // depth frame time stamps
    QVector<double> m_depthTimeStamps;
};

class CS_CAMERA_EXPORT CameraCaptureTool : public Processor::ProcessEndListener
{
    Q_OBJECT
public:
    CameraCaptureTool();
    ~CameraCaptureTool();
    void process(const OutputDataPort& outputDataPort) override;
    
    void startCapture(CameraCaptureConfig config, bool autoNaming = false);
    void setCamera(std::shared_ptr<ICSCamera>& m_camera);
    void setCurOutputData(const CameraCaptureConfig& config);
public slots:
    void stopCapture();
signals:
    void captureNumberUpdated(int captured, int dropped);
    void captureStateChanged(int captureType, int state, QString message);
private:
    QMutex m_mutex;
    // for saving data
    OutputDataPort m_cachedOutputData;
    CameraCaptureBase* m_cameraCapture = nullptr;
    std::shared_ptr<ICSCamera> m_camera;
};
}

#endif // _CS_CAMERA_CAPTURETOOL_H