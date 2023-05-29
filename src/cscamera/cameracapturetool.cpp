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

#include "cameracapturetool.h"
#include <QMutexLocker>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QVariant>

#include <icscamera.h>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <JlCompress.h>

#include "outputsaver.h"

using namespace cs;

#define MAX_CACHED_COUNT 20
#define MAX_SAVING_COUNT 50

CameraCaptureTool::CameraCaptureTool()
{
    qInfo() << "CameraCaptureTool";
}

CameraCaptureTool::~CameraCaptureTool()
{
    qInfo() << "~CameraCaptureTool";
}

void CameraCaptureTool::process(const OutputDataPort& outputDataPort)
{
    QMutexLocker locker(&m_mutex);
    if (outputDataPort.isEmpty())
    {
        qWarning() << "CameraCaptureTool, process, outputDataPort is empty";
        return;
    }

    m_cachedOutputData = outputDataPort;

    if (m_cameraCapture)
    {
        m_cameraCapture->addOutputData(outputDataPort);
    }
}

void CameraCaptureTool::startCapture(CameraCaptureConfig config, bool autoNaming)
{
    QMutexLocker locker(&m_mutex);
    if (m_cameraCapture && m_cameraCapture->getCaptureType() != CAPTURE_TYPE_SINGLE)
    {
        qInfo() << "captruing, please wait...";
        emit captureStateChanged(config.captureType, CAPTURE_WARNING, tr("captruing, please wait..."));
        return;
    }

    if(!m_cameraCapture || (config.captureType != m_cameraCapture->getCaptureType()))
    {
        if (m_cameraCapture)
        {
            delete m_cameraCapture;
        }

        if (config.captureType == CAPTURE_TYPE_MULTIPLE)
        {
            m_cameraCapture = new CameraCaptureMultiple(config);
        }
        else 
        {
            m_cameraCapture = new CameraCaptureSingle(config);
        }
    }
    else 
    {
        m_cameraCapture->setCameraCaptureConfig(config);
    }

    m_cameraCapture->setCamera(m_camera);

    bool suc = true;

    suc &= (bool)connect(m_cameraCapture, &CameraCaptureBase::captureStateChanged, this, &CameraCaptureTool::captureStateChanged);
    suc &= (bool)connect(m_cameraCapture, &CameraCaptureBase::captureNumberUpdated, this, &CameraCaptureTool::captureNumberUpdated);
    suc &= (bool)connect(m_cameraCapture, &CameraCaptureBase::finished, this, [=]() 
        {
            m_cameraCapture->deleteLater();
            m_cameraCapture = nullptr;
        });

    Q_ASSERT(suc);

    //start capture
    m_cameraCapture->start();
}

void CameraCaptureTool::stopCapture()
{
    QMutexLocker locker(&m_mutex);
    if (!m_cameraCapture)
    {
        qWarning() << "cameraCapture is nullptr";
        return;
    }

    m_cameraCapture->requestInterruption();

    //delete m_cameraCapture later
    m_cameraCapture = nullptr;
}

void CameraCaptureTool::setCamera(std::shared_ptr<ICSCamera>& camera)
{
    this->m_camera = camera;
}

void CameraCaptureTool::setCurOutputData(const CameraCaptureConfig& config)
{
    if (config.captureType != CAPTURE_TYPE_SINGLE)
    {
        qWarning() << "";
        return;
    }

    if (!m_cameraCapture)
    {
        m_cameraCapture = new CameraCaptureSingle(config);
    }
    
    m_cameraCapture->setOutputData(m_cachedOutputData);
}

CameraCaptureBase::CameraCaptureBase(const CameraCaptureConfig& config, CAPTURE_TYPE captureType)
    : m_captureConfig(config)
    , m_captureType(captureType)
    , m_maxCachedCount(MAX_CACHED_COUNT)
    , m_maxSavingCount(MAX_SAVING_COUNT)
{
    int maxThread = m_threadPool.maxThreadCount() > 4 ? 4 : m_threadPool.maxThreadCount();
    m_threadPool.setMaxThreadCount(maxThread);
}

CAPTURE_TYPE CameraCaptureBase::getCaptureType() const
{
    return m_captureType;
}

void CameraCaptureBase::run()
{
    qInfo() << "start capturing";
    QTime time;
    time.start();

    emit captureStateChanged(m_captureConfig.captureType, CAPTURING, tr("Start capturing"));

    int captured = getCapturedCount();
    int skip = getSkipCount();

    emit captureNumberUpdated(captured, skip);

    while (!isInterruptionRequested() && (skip + captured) < m_captureConfig.captureNumber)
    {
        m_saverMutex.lock();
        if (m_outputDatas.isEmpty() || m_outputSaverList.size() >= m_maxSavingCount)
        {
            QThread::msleep(2);
            m_saverMutex.unlock();
        }
        else 
        {   
            OutputSaver* outputSaver = nullptr;

            outputSaver = m_outputDatas.dequeue();
            m_outputSaverList.push_back(outputSaver);
            m_saverMutex.unlock();

            outputSaver->updateSaveIndex();
            outputSaver->updateCaptureConfig(m_captureConfig);

            // save a frame in separate thread
            m_threadPool.start(outputSaver, QThread::NormalPriority);
        }

        captured = getCapturedCount();
        skip = getSkipCount();
    }
    
    m_captureFinished = true;

    qInfo() << "wait all thread finished";
    //wait all thread finished
    m_threadPool.clear();
    m_threadPool.waitForDone();

    onCaptureDataDone();

    int timeMs = time.elapsed();
    qInfo("captured %d frames (%d dropped), spend time : %d ms", m_capturedDataCount, m_skipDataCount, timeMs);

    QString msg = QString(tr("End capture, captured %1 frames (%2 dropped)")).arg(m_capturedDataCount).arg(m_skipDataCount);
    emit captureStateChanged(m_captureConfig.captureType, CAPTURE_FINISHED, msg);
}

// called by OutputSaver
void CameraCaptureBase::saveFinished(OutputSaver* saver)
{
    QMutexLocker locker(&m_saverMutex);

    m_capturedDataCount++;
    m_outputSaverList.removeAll(saver);

    emit captureNumberUpdated(m_capturedDataCount, m_skipDataCount);
}

void CameraCaptureBase::setCamera(std::shared_ptr<ICSCamera>& m_camera)
{
    this->m_camera = m_camera;
}

void CameraCaptureBase::setCameraCaptureConfig(const CameraCaptureConfig& config)
{
    m_captureConfig = config;
}

void CameraCaptureBase::setOutputData(const OutputDataPort& outputDataPort)
{
    QMutexLocker locker(&m_saverMutex);
    while(!m_outputDatas.isEmpty())
    {
        auto output = m_outputDatas.dequeue();
        delete output;
    }

    m_outputDatas.enqueue(genOutputSaver(outputDataPort));
    m_cachedDataCount++;
}

OutputSaver* CameraCaptureBase::genOutputSaver(const OutputDataPort& outputData)
{
    OutputSaver* outputSaver = nullptr;
    if (m_captureConfig.saveFormat == "raw")
    {
        outputSaver = new RawOutputSaver(this, m_captureConfig, outputData);
    }
    else
    {
        outputSaver = new ImageOutputSaver(this, m_captureConfig, outputData);
    }

    return outputSaver;
}

int CameraCaptureBase::getCapturedCount()
{
    QMutexLocker locker(&m_saverMutex);
    return m_capturedDataCount;
}

int CameraCaptureBase::getSkipCount()
{
    QMutexLocker locker(&m_saverMutex);
    return m_skipDataCount;
}

YAML::Node genYamlNodeFromIntrinsics(const Intrinsics& intrinsics)
{
    YAML::Node node;
    node["width"] = intrinsics.width;
    node["height"] = intrinsics.height;

    YAML::Node nodeMatrix;
    node["matrix"] = nodeMatrix;

    nodeMatrix.SetTag("opencv-matrix");
    nodeMatrix["rows"] = 3;
    nodeMatrix["cols"] = 3;
    nodeMatrix["dt"] = "f";

    YAML::Node matrixData;
    matrixData.SetStyle(YAML::EmitterStyle::Flow);
    matrixData[0] = intrinsics.fx;
    matrixData[1] = intrinsics.zero01;
    matrixData[2] = intrinsics.cx;
    matrixData[3] = intrinsics.zeor10;
    matrixData[4] = intrinsics.fy;
    matrixData[5] = intrinsics.cy;
    matrixData[6] = intrinsics.zeor20;
    matrixData[7] = intrinsics.zero21;
    matrixData[8] = intrinsics.one22;

    nodeMatrix["data"] = matrixData;

    return node;
}

YAML::Node genYamlNodeFromExtrinsics(const Extrinsics& extrinsics)
{
    YAML::Node node;
    
    {
        YAML::Node nodeMatrix;
        node["rotation"] = nodeMatrix;

        nodeMatrix.SetTag("opencv-matrix");
        nodeMatrix["rows"] = 3;
        nodeMatrix["cols"] = 3;
        nodeMatrix["dt"] = "f";

        YAML::Node matrixData;
        matrixData.SetStyle(YAML::EmitterStyle::Flow);
        matrixData[0] = extrinsics.rotation[0];
        matrixData[1] = extrinsics.rotation[1];
        matrixData[2] = extrinsics.rotation[2];
        matrixData[3] = extrinsics.rotation[3];
        matrixData[4] = extrinsics.rotation[4];
        matrixData[5] = extrinsics.rotation[5];
        matrixData[6] = extrinsics.rotation[6];
        matrixData[7] = extrinsics.rotation[7];
        matrixData[8] = extrinsics.rotation[8];

        nodeMatrix["data"] = matrixData;
    }

    YAML::Node nodeMatrix2;
    node["translation"] = nodeMatrix2;
    nodeMatrix2.SetTag("opencv-matrix");
    nodeMatrix2["rows"] = 1;
    nodeMatrix2["cols"] = 3;
    nodeMatrix2["dt"] = "f";

    YAML::Node matrixData;
    matrixData.SetStyle(YAML::EmitterStyle::Flow);
    matrixData[0] = extrinsics.translation[0];
    matrixData[1] = extrinsics.translation[1];
    matrixData[2] = extrinsics.translation[2];

    nodeMatrix2["data"] = matrixData;

    return node;
}

YAML::Node genYamlNodeFromDataTypes(QVector<CS_CAMERA_DATA_TYPE> captureDataTypes)
{
    YAML::Node node;
    node.SetStyle(YAML::EmitterStyle::Flow);

    for (int i = 0; i < captureDataTypes.size(); i++)
    {
        CS_CAMERA_DATA_TYPE dataType = captureDataTypes.at(i);
        switch (dataType)
        {
        case CAMERA_DATA_L:
            node[i] = "IR(L)";
            break;
        case CAMERA_DATA_R:
            node[i] = "IR(R)";
            break;
        case CAMERA_DATA_DEPTH:
            node[i] = "Depth";
            break;
        case CAMERA_DATA_RGB:
            node[i] = "RGB";
            break;
        case CAMERA_DATA_POINT_CLOUD:
            node[i] = "Point Cloud";
            break;
        default:
            break;
        }
    }

    return node;
}

void CameraCaptureBase::saveCameraPara(QString filePath)
{
    qInfo() << "save camera parameters";
  
    auto bytesData = filePath.toLocal8Bit();
    std::ofstream fout(bytesData.data());

    if (!fout.is_open())
    {
        qWarning() << "open file failed, file:" << filePath;
        return;
    }

    fout << "%YAML:1.0\n";
    fout << "---\n";

    YAML::Node rootNode;

    // 
    rootNode["Version"] = "1.0.0";
    rootNode["Frame Number"] = m_capturedDataCount;
    rootNode["Data Types"] = genYamlNodeFromDataTypes(m_captureConfig.captureDataTypes);
    rootNode["Save Format"] = m_captureConfig.saveFormat.toStdString();
    rootNode["Name"] = m_captureConfig.saveName.toStdString();

    if (m_captureConfig.captureDataTypes.contains(CAMERA_DATA_POINT_CLOUD) && m_captureConfig.savePointCloudWithTexture)
    {
        rootNode["With Texture"] = true;
    }

    // save depth resolution
    {
        QVariant value;
        m_camera->getCameraPara(cs::parameter::PARA_DEPTH_RESOLUTION, value);
        QSize res = value.toSize();

        YAML::Node nodeRes;
        nodeRes["width"] = res.width();
        nodeRes["height"] = res.height();
        rootNode["Depth resolution"] = nodeRes;
    }

    {
        // Depth intrinsics
        Intrinsics depthIntrinsics;
        QVariant intrinsics;
        m_camera->getCameraPara(cs::parameter::PARA_DEPTH_INTRINSICS, intrinsics);
        if (intrinsics.isValid())
        {
            depthIntrinsics = intrinsics.value<Intrinsics>();
            YAML::Node node = genYamlNodeFromIntrinsics(depthIntrinsics);

            rootNode["Depth intrinsics"] = node;
        }
    }
    
    {
        // save depth scale
        QVariant value;
        float depthScale;
        m_camera->getCameraPara(cs::parameter::PARA_DEPTH_SCALE, value);
        if (value.isValid())
        {
            depthScale = value.toFloat();
            rootNode["Depth Scale"] = depthScale;
        }
    }

    {
        // save depth range
        QVariant value;
        m_camera->getCameraPara(cs::parameter::PARA_DEPTH_RANGE, value);
        if (value.isValid())
        {
            auto range = value.value<QPair<float, float>>();
            rootNode["Depth Min"] = range.first;
            rootNode["Depth Max"] = range.second;
        }
    }

    {
        // Depth exposure
        QVariant value;
        m_camera->getCameraPara(cs::parameter::PARA_DEPTH_EXPOSURE, value);
        if (value.isValid())
        {
            rootNode["Depth Exposure Time"] = value.toFloat();
        }
    }

    {
        // Depth gain
        QVariant value;
        m_camera->getCameraPara(cs::parameter::PARA_DEPTH_GAIN, value);
        if (value.isValid())
        {
            rootNode["Depth Gain"] = value.toFloat();
        }
    }

    // RGB intrinsics
    QVariant hasRgbV;
    m_camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);

    if (hasRgbV.isValid() && hasRgbV.toBool())
    {
        // save RGB resolution
        QVariant value;
        m_camera->getCameraPara(cs::parameter::PARA_RGB_RESOLUTION, value);
        QSize res = value.toSize();
        {
            YAML::Node nodeRes;
            nodeRes["width"] = res.width();
            nodeRes["height"] = res.height();
            rootNode["RGB resolution"] = nodeRes;
        }

        Intrinsics rgbIntrinsics;
        QVariant intrinsics;
        m_camera->getCameraPara(cs::parameter::PARA_RGB_INTRINSICS, intrinsics);

        if (intrinsics.isValid())
        {
            rgbIntrinsics = intrinsics.value<Intrinsics>();
            YAML::Node node = genYamlNodeFromIntrinsics(rgbIntrinsics);

            rootNode["RGB intrinsics"] = node;
        }
        else
        {
            qWarning() << "get rgb intrinsics failed";
        }

        {
            // RGB exposure
            QVariant value;
            m_camera->getCameraPara(cs::parameter::PARA_RGB_EXPOSURE, value);
            if (value.isValid())
            {
                rootNode["RGB Exposure Time"] = value.toFloat();
            }
        }

        {
            // RGB gain
            QVariant value;
            m_camera->getCameraPara(cs::parameter::PARA_RGB_GAIN, value);
            if (value.isValid())
            {
                rootNode["RGB Gain"] = value.toFloat();
            }
        }
    }

    {
        // save extrinsics
        QVariant value;
        Extrinsics extrinsics;
        m_camera->getCameraPara(cs::parameter::PARA_EXTRINSICS, value);
        if (value.isValid())
        {
            extrinsics = value.value<Extrinsics>();
            rootNode["Extrinsics"] = genYamlNodeFromExtrinsics(extrinsics);
        }
    }

    fout << rootNode;
}

void CameraCaptureBase::onCaptureDataDone()
{
    // save camera parameters to file
    QString savePath = m_captureConfig.saveDir + QDir::separator() + m_captureConfig.saveName + "-CaptureParameters.yaml";
    saveCameraPara(savePath);
}

CameraCaptureSingle::CameraCaptureSingle(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_SINGLE)
{
    m_realSaveFolder = config.saveDir;
}

void CameraCaptureSingle::getCaptureIndex(const OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex)
{
    rgbFrameIndex = depthFrameIndex = pointCloudIndex = -1;
}

CameraCaptureMultiple::CameraCaptureMultiple(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_MULTIPLE)
{
    m_realSaveFolder = m_captureConfig.saveDir;
    m_captureConfig.saveDir = genTmpSaveDir(config.saveDir);
}

QString CameraCaptureMultiple::genTmpSaveDir(QString saveDir)
{
    QDir dir(saveDir);
    
    // make tmp dir 
    QDateTime timeCurrent = QDateTime::currentDateTime();
    QString timeStr = timeCurrent.toString("yyMMddhhmmss");

    QString tmpDir = saveDir;

    QString tmpName = QString("~capture-%1").arg(timeStr);
    if (!dir.mkdir(tmpName))
    {
        qWarning() << "mkdir failed, dir:" << saveDir << "/" << tmpName;
        Q_ASSERT(false);
    }

    return saveDir + QDir::separator() + tmpName;
}

void CameraCaptureMultiple::addOutputData(const OutputDataPort& outputDataPort)
{
    if (m_captureFinished)
    {
        return;
    }

    QMutexLocker locker(&m_saverMutex);
    if (m_cachedDataCount + m_skipDataCount  >= m_captureConfig.captureNumber)
    {
        qInfo() << "output data count >= capture count, skip the output data";
        return;
    }

    if (m_outputDatas.size() >= m_maxCachedCount)
    {
        m_skipDataCount++;
        qWarning() << "skip one frame, skipDataCount=" << m_skipDataCount << ", (skipDataCount + cachedDataCount) = " << m_cachedDataCount + m_skipDataCount;

        emit captureStateChanged(m_captureConfig.captureType, CAPTURE_WARNING, tr("a frame dropped"));
    }
    else 
    {
        m_outputDatas.enqueue(genOutputSaver(outputDataPort));
        m_cachedDataCount++;
    }
}

void CameraCaptureMultiple::getCaptureIndex(const OutputDataPort& output, int& rgbFrameIdx, int& depthFrameIdx, int& pointCloudIdx)
{
    rgbFrameIdx = m_capturedRgbCount;
    depthFrameIdx = m_capturedDepthCount;
    pointCloudIdx = m_capturePointCloudCount;

    FrameData frameData = output.getFrameData();
    
    for (auto& streamData : frameData.data)
    {
        if (streamData.dataInfo.streamDataType == TYPE_DEPTH)
        {
            m_capturedDepthCount++;
            m_capturePointCloudCount++;

            m_depthTimeStamps.push_back(streamData.dataInfo.timeStamp);
        }
        else if (streamData.dataInfo.streamDataType == TYPE_RGB)
        {
            m_capturedRgbCount++;
            m_rgbTimeStamps.push_back(streamData.dataInfo.timeStamp);
        }
    }
}

void CameraCaptureMultiple::onCaptureDataDone()
{
    // save time stamps to file
    saveTimeStamps();

    // save camera parameters and capture information
    QString savePath = m_captureConfig.saveDir + QDir::separator() + "CaptureParameters.yaml";
    saveCameraPara(savePath);

    // compress to zip file
    compressToZip();
}

void CameraCaptureMultiple::saveTimeStamps()
{
    qInfo() << "save time stamps";

    QString savePath = m_captureConfig.saveDir + QDir::separator() + "TimeStamps.txt";

    if (m_depthTimeStamps.isEmpty() && m_rgbTimeStamps.isEmpty())
    {
        return;
    }

    QFile file(savePath);
    file.open(QFile::WriteOnly);
    if (!file.isOpen())
    {
        qWarning() << "open file failed, file:" << file;
        return;
    }

    QTextStream ts(&file);

    // save depth time stamps
    if (!m_depthTimeStamps.isEmpty())
    {
        ts << "[Depth Time Stamps]\n";
        const int size = m_depthTimeStamps.size();
        for(int i = 0; i < size; i++)
        {
            QString time = QString().setNum(qRound64(m_depthTimeStamps.at(i)));
            QString s = QString("%1 = %2\n").arg(i, 4, 10,QChar('0')).arg(time);
            ts << s;
        }

        ts << "\n";
    }

    // save RGB time stamps
    if (!m_rgbTimeStamps.isEmpty())
    {
        ts << "[RGB Time Stamps]\n";
        const int size = m_rgbTimeStamps.size();
        for (int i = 0; i < size; i++)
        {
            QString time = QString().setNum(qRound64(m_rgbTimeStamps.at(i)));
            QString s = QString("%1 = %2\n").arg(i, 4, 10, QChar('0')).arg(time);
            ts << s;
        }
    }
}

void CameraCaptureMultiple::compressToZip()
{
    emit captureStateChanged(m_captureConfig.captureType, CAPTURING, tr("Please wait for the file to be compressed to zip"));

    QString m_zipFile = m_realSaveFolder + QDir::separator() + m_captureConfig.saveName + ".zip";

    int result = JlCompress::compressDir(m_zipFile, m_captureConfig.saveDir, false, QDir::Files);
    if (result)
    {
        qInfo() << "Compress zip file success.";
    }
    else 
    {
        qWarning() << "Compress zip file failed, zip file : " << m_zipFile;
        emit captureStateChanged(m_captureConfig.captureType, CAPTURE_ERROR, tr("Failed to compress zip file"));
    }

    // delete tmp folder
    qInfo() << "delete tmp dir:" << m_captureConfig.saveDir;
    QDir dir(m_captureConfig.saveDir);
    dir.removeRecursively();
}