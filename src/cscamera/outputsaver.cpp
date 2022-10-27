#include "outputsaver.h"
#include <QDebug>
#include <QFile>

#include <png.h>
#include "cameracapturetool.h"

using namespace cs;
OutputSaver::OutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output)
    : cameraCapture(cameraCapture)
    , captureConfig(config)
    , outputDataPort(output)
{
    //qInfo() << "OutputSaver";
    setAutoDelete(true);
}

OutputSaver::~OutputSaver()
{
    //qInfo() << "~OutputSaver";
}

void OutputSaver::run()
{
    // save point cloud
    savePointCloud();

    // save 2D datas
    saveOutput2D();

    cameraCapture->saveFinished();
}

void OutputSaver::setSaveIndex(int rgbFrameIndex, int depthFrameIndex, int pointCloudIndex)
{
    this->rgbFrameIndex = rgbFrameIndex;
    this->depthFrameIndex = depthFrameIndex;
    this->pointCloudIndex = pointCloudIndex;
}

void OutputSaver::savePointCloud()
{
    if (!captureConfig.captureDataTypes.contains(CAMERA_DTA_POINT_CLOUD))
    {
        return;
    }

    QImage texImage;
    auto frameData = outputDataPort.getFrameData();

    // generate texture image from frame data
    if (captureConfig.savePointCloudWithTexture)
    {
        for (auto& streamData : frameData.data)
        {
            STREAM_FORMAT format = streamData.dataInfo.format;

            if (format == STREAM_FORMAT_RGB8)
            {
                QImage image = QImage((uchar*)streamData.data.data(), streamData.dataInfo.width, streamData.dataInfo.height, QImage::Format_RGB888);
                texImage = image.copy(image.rect());
            }
            else if (format == STREAM_FORMAT_MJPG)
            {
                QImage image;
                image.loadFromData(streamData.data, "JPG");
                texImage = image;
            }
        }
    }

    bool saveTexture = texImage.isNull();

    if (outputDataPort.hasData(CAMERA_DTA_POINT_CLOUD))
    {
        cs::Pointcloud pointCloud = outputDataPort.getPointCloud();
        savePointCloud(pointCloud, texImage);
    }
    else
    {
        cs::Pointcloud pc;
        for (auto& streamData : frameData.data)
        {
            switch (streamData.dataInfo.format)
            {
            case STREAM_FORMAT_Z16:
            case STREAM_FORMAT_Z16Y8Y8: 
            {
                int width = streamData.dataInfo.width;
                int height = streamData.dataInfo.height;
                float depthScale = frameData.depthScale;

                Intrinsics depthIntrinsics = frameData.depthIntrinsics;

                if (saveTexture)
                {
                    Intrinsics rgbIntrinsics = frameData.rgbIntrinsics;
                    Extrinsics extrinsics = frameData.extrinsics;

                    pc.generatePoints<ushort>((ushort*)streamData.data.data(), width, height, depthScale, &depthIntrinsics, &rgbIntrinsics, &extrinsics, true);
                }
                else 
                {
                    pc.generatePoints<ushort>((ushort*)streamData.data.data(), width, height, depthScale, &depthIntrinsics, nullptr, nullptr, true);
                }
                break;
            }
            default:
                break;
            }
        }

        savePointCloud(pc, texImage);
    }
}

void OutputSaver::saveOutput2D()
{
    FrameData frameData = outputDataPort.getFrameData();
    for (auto& streamData : frameData.data)
    {
        saveOutput2D(streamData);
    }
}

void OutputSaver::saveOutput2D(StreamData& streamData)
{
    auto captureTypes = captureConfig.captureDataTypes;
    if (captureTypes.contains(CAMERA_DATA_RGB))
    {
        saveOutputRGB(streamData);
    }

    if (captureTypes.contains(CAMERA_DATA_DEPTH))
    {
        saveOutputDepth(streamData);
    }

    if (captureTypes.contains(CAMERA_DATA_L) || captureTypes.contains(CAMERA_DATA_R))
    {
        saveOutputIr(streamData);
    }
}

void OutputSaver::savePointCloud(cs::Pointcloud& pointCloud, QImage& texImage)
{
    QString savePath = getSavePath(CAMERA_DTA_POINT_CLOUD);
    if (texImage.isNull())
    {
        pointCloud.exportToFile(savePath.toStdString(), nullptr, 0, 0);
    }
    else 
    {
        pointCloud.exportToFile(savePath.toStdString(), texImage.bits(), texImage.width(), texImage.height());
    }
}

QString OutputSaver::getSavePath(CS_CAMERA_DATA_TYPE dataType)
{
    QString fileName = captureConfig.saveName;
    QString savePath;

    switch (dataType)
    {
    case CAMERA_DATA_L:
        fileName = (depthFrameIndex < 0) ? QString("%1-ir-L").arg(fileName) : QString("%1-ir-L-%2").arg(fileName).arg(depthFrameIndex, 4, 10, QChar('0'));
        fileName += suffix2D;
        break;
    case CAMERA_DATA_R:
        fileName = (depthFrameIndex < 0) ? QString("%1-ir-R").arg(fileName) : QString("%1-ir-R-%2").arg(fileName).arg(depthFrameIndex, 4, 10, QChar('0'));
        fileName += suffix2D;
        break;
    case CAMERA_DATA_DEPTH:
        fileName = (depthFrameIndex < 0) ? QString("%1-depth").arg(fileName) : QString("%1-depth-%2").arg(fileName).arg(depthFrameIndex, 4, 10, QChar('0'));
        fileName += suffix2D;
        break;
    case CAMERA_DATA_RGB:
        fileName = (rgbFrameIndex < 0) ? QString("%1-RGB").arg(fileName) : QString("%1-RGB-%2").arg(fileName).arg(rgbFrameIndex, 4, 10, QChar('0'));
        fileName += suffix2D;
        break;
    case CAMERA_DTA_POINT_CLOUD:
        fileName = (pointCloudIndex < 0) ? QString("%1.ply").arg(fileName) : QString("%1-%2.ply").arg(fileName).arg(pointCloudIndex, 4, 10, QChar('0'));
        break;
    default:
        break;
    }

    savePath = QString("%1/%2").arg(captureConfig.saveDir).arg(fileName);

    return savePath;
}

ImageOutputSaver::ImageOutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output)
    : OutputSaver(cameraCapture, config, output)
{
    suffix2D = ".png";
}

void ImageOutputSaver::saveOutputRGB(StreamData& streamData)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_RGB8:
    case STREAM_FORMAT_MJPG:
    {
        CS_CAMERA_DATA_TYPE dataType = CAMERA_DATA_RGB;
        QString savePath = getSavePath(dataType);

        QImage image;
        if (outputDataPort.hasData(dataType))
        {
            image = outputDataPort.getOutputData2D(dataType).image;
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

void ImageOutputSaver::saveOutputDepth(StreamData& streamData)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_Z16:
    case STREAM_FORMAT_Z16Y8Y8:
    {
        CS_CAMERA_DATA_TYPE dataType = CAMERA_DATA_DEPTH;

        QString savePath = getSavePath(dataType);
        saveGrayScale16(streamData, savePath);
        break;
    }
    default:
        break;
    }
}

FILE* openFile(QString& path)
{
    FILE* fp = nullptr;
    
    QByteArray data = path.toLocal8Bit();
    fp = fopen(data.data(), "wb");

    return fp;
}

void ImageOutputSaver::saveGrayScale16(StreamData& streamData, QString path)
{
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    FILE* fp = nullptr;

    int bitDepth = 16;
    int width = streamData.dataInfo.width;
    int height = streamData.dataInfo.height;

    uchar* pngData = (uchar*)streamData.data.data();
    do 
    {
        fp = openFile(path);
        if (fp == nullptr) {
            qWarning() << "Open file failed, file : " << path;
            break;
        }

        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png_ptr == nullptr) {
            qWarning() << ("Could not allocate write struct");
            break;
        }

        // Initialize info structure
        info_ptr = png_create_info_struct(png_ptr);
        if (info_ptr == nullptr) {
            qWarning() << "Could not allocate info struct";
            break;
        }

        if (setjmp(png_jmpbuf(png_ptr))) {
            qWarning() << "DisplayTool::writeImgToFile, Error during png creation";
            break;
        }

        png_init_io(png_ptr, fp);

        png_set_IHDR(png_ptr, info_ptr, width, height, bitDepth, PNG_FORMAT_GRAY, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);

        int rowSize = width * (bitDepth / 8);

        for (int i = 0; i < height; i++) {
            int offset = i * rowSize;
            png_write_row(png_ptr, pngData + offset);
        }

        png_write_end(png_ptr, nullptr);
    
    } while (false);


    if (fp != nullptr)
        fclose(fp);

    if (png_ptr != nullptr || info_ptr != nullptr)
        png_destroy_write_struct(&png_ptr, &info_ptr);
}

void ImageOutputSaver::saveOutputIr(StreamData& streamData)
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
            QString savePath = getSavePath(dataType);

            const int width = streamData.dataInfo.width;
            const int height = streamData.dataInfo.height;

            const int offset2 = pair.second * width * height + offset;

            QImage image;
            if (outputDataPort.hasData(dataType))
            {
                image = outputDataPort.getOutputData2D(dataType).image;
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

RawOutputSaver::RawOutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output)
    : OutputSaver(cameraCapture, config, output)
{
    suffix2D = ".raw";
}

void RawOutputSaver::saveOutputRGB(StreamData& streamData)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_RGB8:
    case STREAM_FORMAT_MJPG:
    {
        QString savePath = getSavePath(CAMERA_DATA_RGB);
        saveDataToFile(savePath, streamData.data);
        break;
    }
    default:
        break;
    }
}

void RawOutputSaver::saveOutputDepth(StreamData& streamData)
{
    switch (streamData.dataInfo.format)
    {
    case STREAM_FORMAT_Z16:
    case STREAM_FORMAT_Z16Y8Y8:
    {
        QString savePath = getSavePath(CAMERA_DATA_DEPTH);
        saveDataToFile(savePath, streamData.data);
        break;  
    }
    default:
        break;
    }
}

void RawOutputSaver::saveOutputIr(StreamData& streamData)
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
            QString savePath = getSavePath(CAMERA_DATA_DEPTH);

            const int width = streamData.dataInfo.width;
            const int height = streamData.dataInfo.height;

            const int offset2 = pair.second * width * height + offset;

            QByteArray data = streamData.data.right(streamData.data.size() - offset);  
            saveDataToFile(savePath, data);
        }
        break;
    }
    default:
        break;
    }
}

void RawOutputSaver::saveDataToFile(QString filePath, QByteArray data)
{
    QFile file(filePath);
    file.open(QFile::WriteOnly);
    if (!file.isOpen())
    {
        qWarning() << "open file failed, file:" << filePath;
        return;
    }

    file.write(data);
    file.flush();
    file.close();
}
