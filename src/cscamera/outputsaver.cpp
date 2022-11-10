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

#include "outputsaver.h"
#include <QDebug>
#include <QFile>

#include <imageutil.h>
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
    if (!captureConfig.captureDataTypes.contains(CAMERA_DATA_POINT_CLOUD))
    {
        return;
    }

    if (!outputDataPort.hasData(CAMERA_DATA_POINT_CLOUD) && !outputDataPort.hasData(CAMERA_DATA_DEPTH))
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

    if (outputDataPort.hasData(CAMERA_DATA_POINT_CLOUD))
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
    QString savePath = getSavePath(CAMERA_DATA_POINT_CLOUD);
    QByteArray pathData = savePath.toLocal8Bit();
    std::string realPath = pathData.data();

    if (texImage.isNull())
    {
        pointCloud.exportToFile(realPath, nullptr, 0, 0);
    }
    else 
    {
        pointCloud.exportToFile(realPath, texImage.bits(), texImage.width(), texImage.height());
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
    case CAMERA_DATA_POINT_CLOUD:
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

void ImageOutputSaver::saveGrayScale16(StreamData& streamData, QString path)
{
    ImageUtil::saveGrayScale16ByLibpng(streamData.dataInfo.width, streamData.dataInfo.height, streamData.data, path);
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
            QString savePath = getSavePath(dataType);

            const int width = streamData.dataInfo.width;
            const int height = streamData.dataInfo.height;

            const int offset2 = pair.second * width * height + offset;

            QByteArray data = streamData.data.right(streamData.data.size() - offset2);
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
