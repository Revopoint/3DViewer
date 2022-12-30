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
    QMutexLocker locker(&mutex);
    if (outputDataPort.isEmpty())
    {
        qWarning() << "CameraCaptureTool, process, outputDataPort is empty";
        return;
    }

    cachedOutputData = outputDataPort;

    if (cameraCapture)
    {
        cameraCapture->addOutputData(outputDataPort);
    }
}

void CameraCaptureTool::startCapture(CameraCaptureConfig config, bool autoNaming)
{
    QMutexLocker locker(&mutex);
    if (cameraCapture && cameraCapture->getCaptureType() != CAPTURE_TYPE_SINGLE)
    {
        qInfo() << "captruing, please wait...";
        emit captureStateChanged(config.captureType, CAPTURE_WARNING, tr("captruing, please wait..."));
        return;
    }

    if(!cameraCapture || (config.captureType != cameraCapture->getCaptureType()))
    {
        if (cameraCapture)
        {
            delete cameraCapture;
        }

        if (config.captureType == CAPTURE_TYPE_MULTIPLE)
        {
            cameraCapture = new CameraCaptureMultiple(config);
        }
        else 
        {
            cameraCapture = new CameraCaptureSingle(config);
        }
    }
    else 
    {
        cameraCapture->setCameraCaptureConfig(config);
    }

    cameraCapture->setCamera(camera);

    bool suc = true;

    suc &= (bool)connect(cameraCapture, &CameraCaptureBase::captureStateChanged, this, &CameraCaptureTool::captureStateChanged);
    suc &= (bool)connect(cameraCapture, &CameraCaptureBase::captureNumberUpdated, this, &CameraCaptureTool::captureNumberUpdated);
    suc &= (bool)connect(cameraCapture, &CameraCaptureBase::finished, this, &CameraCaptureTool::stopCapture);

    Q_ASSERT(suc);

    //start capture
    cameraCapture->start();
}

void CameraCaptureTool::stopCapture()
{
    QMutexLocker locker(&mutex);
    if (!cameraCapture)
    {
        qWarning() << "cameraCapture is nullptr";
        return;
    }

    cameraCapture->requestInterruption();
    cameraCapture->wait();

    delete cameraCapture;
    cameraCapture = nullptr;
}

void CameraCaptureTool::setCamera(std::shared_ptr<ICSCamera>& camera)
{
    this->camera = camera;
}

void CameraCaptureTool::setCurOutputData(const CameraCaptureConfig& config)
{
    if (config.captureType != CAPTURE_TYPE_SINGLE)
    {
        qWarning() << "";
        return;
    }

    if (!cameraCapture)
    {
        cameraCapture = new CameraCaptureSingle(config);
    }
    
    cameraCapture->setOutputData(cachedOutputData);
}

CameraCaptureBase::CameraCaptureBase(const CameraCaptureConfig& config, CAPTURE_TYPE captureType)
    : captureConfig(config)
    , captureType(captureType)
{
    //threadPool.setMaxThreadCount(1);
}

CAPTURE_TYPE CameraCaptureBase::getCaptureType() const
{
    return captureType;
}

void CameraCaptureBase::run()
{
    qInfo() << "start capturing";
    QTime time;
    time.start();

    emit captureStateChanged(captureConfig.captureType, CAPTURING, tr("Start capturing"));

    int captured = 0;
    {
        QMutexLocker locker(&saverMutex);
        captured = capturedDataCount;
    }

    emit captureNumberUpdated(capturedDataCount, skipDataCount);

    while (!isInterruptionRequested() && (skipDataCount + captured) < captureConfig.captureNumber)
    {
        mutex.lock();
        if (outputDatas.isEmpty())
        {
            QThread::msleep(2);
            mutex.unlock();
        }
        else 
        {
            OutputDataPort outputData;
            outputData = outputDatas.dequeue();
            mutex.unlock();

            if (outputData.isEmpty())
            {
                emit captureStateChanged(captureConfig.captureType, CAPTURE_ERROR, tr("save frame data is empty"));
                continue;
            }

            int rgbFrameIndex = -1, depthFrameIndex = -1, pointCloudIndex = -1;
            getCaptureIndex(outputData, rgbFrameIndex, depthFrameIndex, pointCloudIndex);

            // save a frame in separate thread
            OutputSaver* outputSaver = nullptr;
            if (captureConfig.saveFormat == "raw")
            {
                outputSaver = new RawOutputSaver(this, captureConfig, outputData);
            }
            else 
            {
                outputSaver = new ImageOutputSaver(this, captureConfig, outputData);
            }

            outputSaver->setSaveIndex(rgbFrameIndex, depthFrameIndex, pointCloudIndex);

            threadPool.start(outputSaver, QThread::NormalPriority);
        }

        {
            QMutexLocker locker(&saverMutex);
            captured = capturedDataCount;
        }
    }
    
    captureFinished = true;

    qInfo() << "wait all thread finished";

    //wait all thread finished
    threadPool.waitForDone();

    int timeMs = time.elapsed();

    onCaptureDataDone();

    qInfo("captured %d frames (%d dropped), spend time : %d ms", capturedDataCount, skipDataCount, timeMs);

    QString msg = QString(tr("End capture, captured %1 frames (%2 dropped)")).arg(capturedDataCount).arg(skipDataCount);
    emit captureStateChanged(captureConfig.captureType, CAPTURE_FINISHED, msg);
}

void CameraCaptureBase::saveFinished()
{
    QMutexLocker locker(&saverMutex);
    capturedDataCount++;

    emit captureNumberUpdated(capturedDataCount, skipDataCount);
}

void CameraCaptureBase::setCamera(std::shared_ptr<ICSCamera>& camera)
{
    this->camera = camera;
}

void CameraCaptureBase::setCameraCaptureConfig(const CameraCaptureConfig& config)
{
    captureConfig = config;
}

void CameraCaptureBase::setOutputData(const OutputDataPort& outputDataPort)
{
    QMutexLocker locker(&mutex);
    outputDatas.clear();
    outputDatas.enqueue(outputDataPort);

    cachedDataCount++;
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
    rootNode["Frame Number"] = capturedDataCount;
    rootNode["Data Types"] = genYamlNodeFromDataTypes(captureConfig.captureDataTypes);
    rootNode["Save Format"] = captureConfig.saveFormat.toStdString();
    rootNode["Name"] = captureConfig.saveName.toStdString();

    if (captureConfig.captureDataTypes.contains(CAMERA_DATA_POINT_CLOUD) && captureConfig.savePointCloudWithTexture)
    {
        rootNode["With Texture"] = true;
    }

    // save depth resolution
    {
        QVariant value;
        camera->getCameraPara(cs::parameter::PARA_DEPTH_RESOLUTION, value);
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
        camera->getCameraPara(cs::parameter::PARA_DEPTH_INTRINSICS, intrinsics);
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
        camera->getCameraPara(cs::parameter::PARA_DEPTH_SCALE, value);
        if (value.isValid())
        {
            depthScale = value.toFloat();
            rootNode["Depth Scale"] = depthScale;
        }
    }

    {
        // save depth range
        QVariant value;
        camera->getCameraPara(cs::parameter::PARA_DEPTH_RANGE, value);
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
        camera->getCameraPara(cs::parameter::PARA_DEPTH_EXPOSURE, value);
        if (value.isValid())
        {
            rootNode["Depth Exposure Time"] = value.toFloat();
        }
    }

    {
        // Depth gain
        QVariant value;
        camera->getCameraPara(cs::parameter::PARA_DEPTH_GAIN, value);
        if (value.isValid())
        {
            rootNode["Depth Gain"] = value.toFloat();
        }
    }

    // RGB intrinsics
    QVariant hasRgbV;
    camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);

    if (hasRgbV.isValid() && hasRgbV.toBool())
    {
        // save RGB resolution
        QVariant value;
        camera->getCameraPara(cs::parameter::PARA_RGB_RESOLUTION, value);
        QSize res = value.toSize();
        {
            YAML::Node nodeRes;
            nodeRes["width"] = res.width();
            nodeRes["height"] = res.height();
            rootNode["RGB resolution"] = nodeRes;
        }

        Intrinsics rgbIntrinsics;
        QVariant intrinsics;
        camera->getCameraPara(cs::parameter::PARA_RGB_INTRINSICS, intrinsics);

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
            camera->getCameraPara(cs::parameter::PARA_RGB_EXPOSURE, value);
            if (value.isValid())
            {
                rootNode["RGB Exposure Time"] = value.toFloat();
            }
        }

        {
            // RGB gain
            QVariant value;
            camera->getCameraPara(cs::parameter::PARA_RGB_GAIN, value);
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
        camera->getCameraPara(cs::parameter::PARA_EXTRINSICS, value);
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
    QString savePath = captureConfig.saveDir + QDir::separator() + captureConfig.saveName + "-CaptureParameters.yaml";
    saveCameraPara(savePath);
}

CameraCaptureSingle::CameraCaptureSingle(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_SINGLE)
{
    realSaveFolder = config.saveDir;
}

void CameraCaptureSingle::getCaptureIndex(OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex)
{
    rgbFrameIndex = depthFrameIndex = pointCloudIndex = -1;
}

CameraCaptureMultiple::CameraCaptureMultiple(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_MULTIPLE)
{
    realSaveFolder = captureConfig.saveDir;
    captureConfig.saveDir = genTmpSaveDir(config.saveDir);
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
    if (captureFinished)
    {
        return;
    }

    QMutexLocker locker(&mutex);
    if (cachedDataCount + skipDataCount  >= captureConfig.captureNumber)
    {
        qInfo() << "output data count >= capture count, skip the output data";
        return;
    }

    if (outputDatas.size() >= maxCachedCount)
    {
        skipDataCount++;
        qWarning() << "skip one frame, skipDataCount=" << skipDataCount << ", (skipDataCount + cachedDataCount) = " << cachedDataCount + skipDataCount;

        emit captureStateChanged(captureConfig.captureType, CAPTURE_WARNING, tr("a frame dropped"));
    }
    else 
    {
        outputDatas.enqueue(outputDataPort);
        cachedDataCount++;
    }
}

void CameraCaptureMultiple::getCaptureIndex(OutputDataPort& output, int& rgbFrameIdx, int& depthFrameIdx, int& pointCloudIdx)
{
    rgbFrameIdx = capturedRgbCount;
    depthFrameIdx = capturedDepthCount;
    pointCloudIdx = capturePointCloudCount;

    FrameData frameData = output.getFrameData();
    
    for (auto& streamData : frameData.data)
    {
        if (streamData.dataInfo.streamDataType == TYPE_DEPTH)
        {
            capturedDepthCount++;
            capturePointCloudCount++;

            depthTimeStamps.push_back(streamData.dataInfo.timeStamp);
        }
        else if (streamData.dataInfo.streamDataType == TYPE_RGB)
        {
            capturedRgbCount++;
            rgbTimeStamps.push_back(streamData.dataInfo.timeStamp);
        }
    }
}

void CameraCaptureMultiple::onCaptureDataDone()
{
    // save time stamps to file
    saveTimeStamps();

    // save camera parameters and capture information
    QString savePath = captureConfig.saveDir + QDir::separator() + "CaptureParameters.yaml";
    saveCameraPara(savePath);

    // compress to zip file
    compressToZip();
}

void CameraCaptureMultiple::saveTimeStamps()
{
    qInfo() << "save time stamps";

    QString savePath = captureConfig.saveDir + QDir::separator() + "TimeStamps.txt";

    if (depthTimeStamps.isEmpty() && rgbTimeStamps.isEmpty())
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
    if (!depthTimeStamps.isEmpty())
    {
        ts << "[Depth Time Stamps]\n";
        const int size = depthTimeStamps.size();
        for(int i = 0; i < size; i++)
        {
            QString time = QString().setNum(qRound64(depthTimeStamps.at(i)));
            QString s = QString("%1 = %2\n").arg(i, 4, 10,QChar('0')).arg(time);
            ts << s;
        }

        ts << "\n";
    }

    // save RGB time stamps
    if (!rgbTimeStamps.isEmpty())
    {
        ts << "[RGB Time Stamps]\n";
        const int size = rgbTimeStamps.size();
        for (int i = 0; i < size; i++)
        {
            QString time = QString().setNum(qRound64(rgbTimeStamps.at(i)));
            QString s = QString("%1 = %2\n").arg(i, 4, 10, QChar('0')).arg(time);
            ts << s;
        }
    }
}

void CameraCaptureMultiple::compressToZip()
{
    emit captureStateChanged(captureConfig.captureType, CAPTURING, tr("Please wait for the file to be compressed to zip"));

    QString zipFile = realSaveFolder + QDir::separator() + captureConfig.saveName + ".zip";

    int result = JlCompress::compressDir(zipFile, captureConfig.saveDir, false, QDir::Files);
    if (result)
    {
        qInfo() << "Compress zip file success.";
    }
    else 
    {
        qWarning() << "Compress zip file failed, zip file : " << zipFile;
        emit captureStateChanged(captureConfig.captureType, CAPTURE_ERROR, tr("Failed to compress zip file"));
    }

    // delete tmp folder
    qInfo() << "delete tmp dir:" << captureConfig.saveDir;
    QDir dir(captureConfig.saveDir);
    dir.removeRecursively();
}