#include "cameracapturetool.h"
#include <QMutexLocker>
#include <QDebug>

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
}

CameraCaptureBase::CAPTURE_TYPE CameraCaptureBase::getCaptureType() const
{
    return captureType;
}

void CameraCaptureBase::run()
{
    qInfo() << "start capturing";

    emit captureStateChanged(CAPTURING, tr("Start capturing"));

    while (!isInterruptionRequested() && (skipDataCount + capturedDataCount) < captureConfig.captureNumber)
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

            // save the data 
            saveOutputData(outputData);
            capturedDataCount++;

            emit captureNumberUpdated(capturedDataCount, skipDataCount);
        }
    }

    captureFinished = true;   
    QString msg = QString("End capture, captured %1 frames (%2 dropped)").arg(capturedDataCount).arg(skipDataCount);

    qInfo() << msg;
    emit captureStateChanged(CAPTURE_FINISHED, msg);
}

void CameraCaptureBase::setOutputData(const OutputDataPort& outputDataPort)
{
    QMutexLocker locker(&mutex);
    outputDatas.clear();
    outputDatas.enqueue(outputDataPort);

    cachedDataCount++;
}

void CameraCaptureBase::saveOutputData(OutputDataPort& output)
{
    if (output.isEmpty())
    {
        emit captureStateChanged(CAPTURE_ERROR, tr("save frame data is empty"));
        return;
    }

    //qInfo() << "save frame data to disk";
  
    auto captureTypes = captureConfig.captureDataTypes;

    // save point cloud
    if (captureTypes.contains(CAMERA_DTA_POINT_CLOUD))
    {
        if (output.hasData(CAMERA_DTA_POINT_CLOUD))
        {
            savePointCloud(output.getPointCloud());
        }
        else
        {
            //TODO: save from frameData
        }
    }

    // save images
    saveOutput2D(output);
}

void CameraCaptureBase::saveOutput2D(OutputDataPort& output)
{
    auto& frameData = output.getFrameData();
    
    for (auto& streamData : frameData.data)
    {
        saveOutput2D(streamData, output);
    }
}

void CameraCaptureBase::saveOutput2D(StreamData& streamData, OutputDataPort& output)
{
    auto captureTypes = captureConfig.captureDataTypes;
    if (captureTypes.contains(CAMERA_DATA_RGB))
    {
        saveOutputRGB(streamData, output);
    }

    if (captureTypes.contains(CAMERA_DATA_DEPTH))
    {
        saveOutputDepth(streamData, output);
    }

    if (captureTypes.contains(CAMERA_DATA_L) || captureTypes.contains(CAMERA_DATA_R))
    {
        saveOutputIr(streamData, output);
    }
}

void CameraCaptureBase::saveOutputRGB(StreamData& streamData, OutputDataPort& output)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_RGB8:
    case STREAM_FORMAT_MJPG: 
        {
            CS_CAMERA_DATA_TYPE dataType = CAMERA_DATA_RGB;
            QString savePath = QString("%1/%2%3.png").arg(captureConfig.saveDir).arg(captureConfig.saveName).arg(getSaveFileSuffix(dataType));
            QImage image;
            if (output.hasData(dataType))
            {
                image = output.getOutputData2D(dataType).image;
            }
            else
            {
                // save from streamData
                if (streamData.dataInfo.format == STREAM_FORMAT_RGB8)
                {
                    image = QImage((uchar*)streamData.data.data(), streamData.dataInfo.width, streamData.dataInfo.height, QImage::Format_RGB888);
                }
                else 
                {
                    image.loadFromData(streamData.data, "JPG");
                }
            }
            if (!image.save(savePath, "PNG"))
            {
                qWarning() << "save image failed:" << savePath;
            }
            break;
        }
    default:
        break;
    }
}

void CameraCaptureBase::saveOutputDepth(StreamData& streamData, OutputDataPort& output)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_Z16:
    case STREAM_FORMAT_Z16Y8Y8:
    {
        CS_CAMERA_DATA_TYPE dataType = CAMERA_DATA_DEPTH;
        QString savePath = QString("%1/%2%3.png").arg(captureConfig.saveDir).arg(captureConfig.saveName).arg(getSaveFileSuffix(dataType));

        QImage image((uchar*)streamData.data.data(), streamData.dataInfo.width, streamData.dataInfo.height, QImage::Format_RGB16);
        if (!image.save(savePath, "PNG"))
        {
            qWarning() << "save image failed:" << savePath;
        }
        break;
    }
    default:
        break;
    }
}

void CameraCaptureBase::saveOutputIr(StreamData& streamData, OutputDataPort& output)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_Z16Y8Y8:
    case STREAM_FORMAT_PAIR: 
    {
        QVector<QPair<CS_CAMERA_DATA_TYPE, int>> saveInfos =
        {
            { CAMERA_DATA_L, 0},
            { CAMERA_DATA_R, 1}
        };

        const int offset = (streamData.dataInfo.format == STREAM_FORMAT_Z16Y8Y8) ? (streamData.data.size() / 2) : 0;

        for (auto pair : saveInfos)
        {
            CS_CAMERA_DATA_TYPE dataType = pair.first;
            QString savePath = QString("%1/%2%3.png").arg(captureConfig.saveDir).arg(captureConfig.saveName).arg(getSaveFileSuffix(dataType));

            const int width = streamData.dataInfo.width;
            const int height = streamData.dataInfo.height;

            const int offset2 = pair.second * width * height + offset;

            QImage image;
            if (output.hasData(dataType))
            {
                image = output.getOutputData2D(dataType).image;
            }
            else
            {
                image = QImage((uchar*)streamData.data.data() + offset2, width, height, QImage::Format_Grayscale8);
            }

            if (!image.save(savePath, "PNG"))
            {
                qWarning() << "save image failed:" << savePath;
            }
        }
        break;
    }
    default:
        break;
    }
}

void CameraCaptureBase::savePointCloud(cs::Pointcloud& pointCloud)
{
    QString savePath = QString("%1/%2%3.ply").arg(captureConfig.saveDir).arg(captureConfig.saveName).arg(getSaveFileSuffix(CAMERA_DTA_POINT_CLOUD));
    pointCloud.exportToFile(savePath.toStdString(), nullptr, 0, 0);
}

QString CameraCaptureBase::getSaveFileSuffix(CS_CAMERA_DATA_TYPE dataType)
{
    QString suffix = "";
    switch (dataType)
    {
    case CAMERA_DATA_L:
        suffix = "-ir-L";
        break;
    case CAMERA_DATA_R:
        suffix = "-ir-R";
        break;
    case CAMERA_DATA_DEPTH:
        suffix = "-depth";
        break;
    case CAMERA_DATA_RGB:
        suffix = "-RGB";
        break;
    case CAMERA_DTA_POINT_CLOUD:
        suffix = "";
        break;
    default:
        break;
    }

    suffix = QString("%1").arg(suffix);

    return suffix;
}

CameraCaptureSingle::CameraCaptureSingle(const CameraCaptureConfig& config)
    : CameraCaptureBase(config, CAPTURE_TYPE_SINGLE)
{

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

QString CameraCaptureMutiple::getSaveFileSuffix(CS_CAMERA_DATA_TYPE dataType)
{
    QString suffix = "";
    int count = 0;

    switch (dataType)
    {
    case CAMERA_DATA_L:
        suffix = "-ir-L";
        count = capturedDepthCount;
        break;
    case CAMERA_DATA_R:
        suffix = "-ir-R";
        count = capturedDepthCount;
        break;
    case CAMERA_DATA_DEPTH:
        suffix = "-depth";
        count = capturedDepthCount;
        break;
    case CAMERA_DATA_RGB:
        suffix = "-RGB";
        count = capturedRgbCount;
        break;
    case CAMERA_DTA_POINT_CLOUD:
        suffix = "-";
        count = capturePointCloudCount;
        break;
    default:
        break;
    }

    suffix = QString("%1-%2").arg(suffix).arg(count, 4, 10, QLatin1Char('0'));

    return suffix;
}

void CameraCaptureMutiple::savePointCloud(cs::Pointcloud& pointCloud)
{
    CameraCaptureBase::savePointCloud(pointCloud);
    capturePointCloudCount++;
}

void CameraCaptureMutiple::saveOutput2D(OutputDataPort& output)
{
    CameraCaptureBase::saveOutput2D(output);

    switch (output.getFrameData().frameDataType)
    {
    case TYPE_DEPTH:
        capturedDepthCount++;
        break;
    case TYPE_RGB:
        capturedRgbCount++;
        break;
    case TYPE_DEPTH_RGB:
        capturedDepthCount++;
        capturedRgbCount++;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}
