#include "cameracapturetool.h"
#include <QMutexLocker>
#include <QDebug>
#include <QTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QVariant>

#include <icscamera.h>
#include <fstream>
#include <yaml-cpp/yaml.h>

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

void CameraCaptureTool::startCapture(CameraCaptureConfig config)
{
    QMutexLocker locker(&mutex);
    if (cameraCapture)
    {
        qInfo() << "captruing, please wait...";
        emit captureStateChanged(CameraCaptureBase::CAPTURE_WARNING, tr("captruing, please wait..."));
        return;
    }

    if (config.captureNumber > 1)
    {
        cameraCapture = new CameraCaptureMutiple(config);
    }
    else 
    {
        cameraCapture = new CameraCaptureSingle(config);
    }

    cameraCapture->setCamera(camera);
    cameraCapture->setOutputData(cachedOutputData);

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
    cameraCapture = nullptr;
}

void CameraCaptureTool::setCamera(std::shared_ptr<ICSCamera>& camera)
{
    this->camera = camera;
}

CameraCaptureBase::CameraCaptureBase(const CameraCaptureConfig& config, CAPTURE_TYPE captureType)
    : captureConfig(config)
    , captureType(captureType)
{
    //threadPool.setMaxThreadCount(1);
}

CameraCaptureBase::CAPTURE_TYPE CameraCaptureBase::getCaptureType() const
{
    return captureType;
}

void CameraCaptureBase::run()
{
    qInfo() << "start capturing";
    QTime time;
    time.start();

    emit captureStateChanged(CAPTURING, tr("Start capturing"));

    int captured = 0;
    {
        QMutexLocker locker(&saverMutex);
        captured = capturedDataCount;
    }

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
                emit captureStateChanged(CAPTURE_ERROR, tr("save frame data is empty"));
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

            threadPool.start(outputSaver, QThread::LowPriority);
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
    emit captureStateChanged(CAPTURE_FINISHED, msg);
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

void CameraCaptureBase::saveIntrinsics()
{
    qInfo() << "save camera intrinsics";

    QVariant hasRgbV;
    camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);

    QString savePath = captureConfig.saveDir + QDir::separator() + captureConfig.saveName + "-intrinsics.yaml";

    std::ofstream fout(savePath.toStdString());
    if (!fout.is_open())
    {
        qWarning() << "open file failed, file:" << savePath;
        return;
    }

    fout << "%YAML:1.0\n";
    fout << "---\n";

    YAML::Node rootNode;
    // RGB intrinsics
    if (hasRgbV.isValid() && hasRgbV.toBool())
    {
        Intrinsics rgbIntrinsics;
        QVariant intrinsics;
        camera->getCameraPara(cs::parameter::PARA_DEPTH_INTRINSICS, intrinsics);

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
    }

    {
        // Depth intrinsics
        Intrinsics depthIntrinsics;
        QVariant intrinsics;
        camera->getCameraPara(cs::parameter::PARA_RGB_INTRINSICS, intrinsics);
        if (intrinsics.isValid())
        {
            depthIntrinsics = intrinsics.value<Intrinsics>();
            YAML::Node node = genYamlNodeFromIntrinsics(depthIntrinsics);

            rootNode["Depth intrinsics"] = node;
        }
    }
    
    {
        // depth scale
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
        // extrinsics
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
    fout.close();
}

void CameraCaptureBase::onCaptureDataDone()
{
    // save camera intrinsics to file
    saveIntrinsics();
}

CameraCaptureSingle::CameraCaptureSingle(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_SINGLE)
{

}

void CameraCaptureSingle::getCaptureIndex(OutputDataPort& output, int& rgbFrameIndex, int& depthFrameIndex, int& pointCloudIndex)
{
    rgbFrameIndex = depthFrameIndex = pointCloudIndex = -1;
}

CameraCaptureMutiple::CameraCaptureMutiple(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_MULTIPLE)
{

}

void CameraCaptureMutiple::addOutputData(const OutputDataPort& outputDataPort)
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

        emit captureStateChanged(CAPTURE_WARNING, tr("a frame dropped"));
    }
    else 
    {
        outputDatas.enqueue(outputDataPort);
        cachedDataCount++;
    }
}

void CameraCaptureMutiple::getCaptureIndex(OutputDataPort& output, int& rgbFrameIdx, int& depthFrameIdx, int& pointCloudIdx)
{
    rgbFrameIdx = capturedRgbCount;
    depthFrameIdx = capturedDepthCount;
    pointCloudIdx = capturePointCloudCount;

    FrameData frameData = output.getFrameData();
    
    for (auto& streamData : frameData.data)
    {
        if (streamData.dataInfo.streamDataType == TYPE_RGB)
        {
            capturedDepthCount++;
            capturePointCloudCount++;

            depthTimeStamps.push_back(streamData.dataInfo.timeStamp);
        }
        else if (streamData.dataInfo.streamDataType == TYPE_DEPTH)
        {
            capturedRgbCount++;
            rgbTimeStamps.push_back(streamData.dataInfo.timeStamp);
        }
    }
}

void CameraCaptureMutiple::onCaptureDataDone()
{
    // save time stamps to file
    saveTimeStamps();

    CameraCaptureBase::onCaptureDataDone();
}

void CameraCaptureMutiple::saveTimeStamps()
{
    qInfo() << "save time stamps";

    QString savePath = captureConfig.saveDir + QDir::separator() + captureConfig.saveName + "-TimeStamps.txt";

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
