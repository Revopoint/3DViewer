#include "cameracapturetool.h"
#include <QMutexLocker>
#include <QDebug>
#include <QTime>

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

    cameraCapture->setOutputData(cachedOutputData);

    bool suc = true;

    suc &= (bool)connect(cameraCapture, &CameraCaptureBase::captureStateChanged, this, &CameraCaptureTool::captureStateChanged);
    suc &= (bool)connect(cameraCapture, &CameraCaptureBase::captureNumberUpdated, this, &CameraCaptureTool::captureNumberUpdated);
    suc &= (bool)connect(cameraCapture, &CameraCaptureBase::finished, this, &CameraCaptureTool::stopCapture);

    Q_ASSERT(suc);

    // start capture
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
    qInfo() << "capture " << capturedDataCount <<  " spend time : " << timeMs;

    QString msg = QString("End capture, captured %1 frames (%2 dropped)").arg(capturedDataCount).arg(skipDataCount);

    qInfo() << msg;
    emit captureStateChanged(CAPTURE_FINISHED, msg);
}

void CameraCaptureBase::saveFinished()
{
    QMutexLocker locker(&saverMutex);
    capturedDataCount++;

    emit captureNumberUpdated(capturedDataCount, skipDataCount);
}


void CameraCaptureBase::setOutputData(const OutputDataPort& outputDataPort)
{
    QMutexLocker locker(&mutex);
    outputDatas.clear();
    outputDatas.enqueue(outputDataPort);

    cachedDataCount++;
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

    auto& frameData = output.getFrameData();
    if (frameData.frameDataType == TYPE_DEPTH_RGB)
    {
        capturedRgbCount++;
        capturedDepthCount++;
        capturePointCloudCount++;
    }
    else if (frameData.frameDataType == TYPE_RGB)
    {
        capturedRgbCount++;
    }
    else if (frameData.frameDataType == TYPE_DEPTH)
    {
        capturedDepthCount++;
        capturePointCloudCount++;
    }
}