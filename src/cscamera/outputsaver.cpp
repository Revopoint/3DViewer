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

#include "outputsaver.h"
#include <QDebug>
#include <QFile>

#include <imageutil.h>
#include "cameracapturetool.h"

using namespace cs;
OutputSaver::OutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output)
    : m_cameraCapture(cameraCapture)
    , m_captureConfig(config)
    , m_outputDataPort(output)
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
    // save 2D datas
    saveOutput2D();

    // save point cloud
    savePointCloud();

    m_cameraCapture->saveFinished(this);
}

void OutputSaver::updateCaptureConfig(const CameraCaptureConfig& config)
{
    m_captureConfig = config;
}

void OutputSaver::updateSaveIndex()
{
    int rgbFrameIndex = -1, depthFrameIndex = -1, pointCloudIndex = -1;
    m_cameraCapture->getCaptureIndex(m_outputDataPort, rgbFrameIndex, depthFrameIndex, pointCloudIndex);
    setSaveIndex(rgbFrameIndex, depthFrameIndex, pointCloudIndex);
}

void OutputSaver::setSaveIndex(int rgbFrameIndex, int depthFrameIndex, int pointCloudIndex)
{
    this->m_rgbFrameIndex = rgbFrameIndex;
    this->m_depthFrameIndex = depthFrameIndex;
    this->m_pointCloudIndex = pointCloudIndex;
}

void OutputSaver::savePointCloud()
{
    if (!m_captureConfig.captureDataTypes.contains(CAMERA_DATA_POINT_CLOUD))
    {
        return;
    }

    if (!m_outputDataPort.hasData(CAMERA_DATA_POINT_CLOUD) && !m_outputDataPort.hasData(CAMERA_DATA_DEPTH))
    {
        return;
    }

    QImage texImage;
    auto frameData = m_outputDataPort.getFrameData();

    // generate texture image from frame data
    if (m_captureConfig.savePointCloudWithTexture)
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

    if (m_outputDataPort.hasData(CAMERA_DATA_POINT_CLOUD))
    {
        cs::Pointcloud pointCloud = m_outputDataPort.getPointCloud();
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
    FrameData frameData = m_outputDataPort.getFrameData();
    for (auto& streamData : frameData.data)
    {
        saveOutput2D(streamData);
    }
}

void OutputSaver::saveOutput2D(StreamData& streamData)
{
    auto captureTypes = m_captureConfig.captureDataTypes;
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
    QString fileName = m_captureConfig.saveName;
    QString savePath;

    switch (dataType)
    {
    case CAMERA_DATA_L:
        fileName = (m_depthFrameIndex < 0) ? QString("%1-ir-L").arg(fileName) : QString("%1-ir-L-%2").arg(fileName).arg(m_depthFrameIndex, 4, 10, QChar('0'));
        fileName += m_suffix2D;
        break;
    case CAMERA_DATA_R:
        fileName = (m_depthFrameIndex < 0) ? QString("%1-ir-R").arg(fileName) : QString("%1-ir-R-%2").arg(fileName).arg(m_depthFrameIndex, 4, 10, QChar('0'));
        fileName += m_suffix2D;
        break;
    case CAMERA_DATA_DEPTH:
        fileName = (m_depthFrameIndex < 0) ? QString("%1-depth").arg(fileName) : QString("%1-depth-%2").arg(fileName).arg(m_depthFrameIndex, 4, 10, QChar('0'));
        fileName += m_suffix2D;
        break;
    case CAMERA_DATA_RGB:
        fileName = (m_rgbFrameIndex < 0) ? QString("%1-RGB").arg(fileName) : QString("%1-RGB-%2").arg(fileName).arg(m_rgbFrameIndex, 4, 10, QChar('0'));
        fileName += m_suffix2D;
        break;
    case CAMERA_DATA_POINT_CLOUD:
        fileName = (m_pointCloudIndex < 0) ? QString("%1.ply").arg(fileName) : QString("%1-%2.ply").arg(fileName).arg(m_pointCloudIndex, 4, 10, QChar('0'));
        break;
    default:
        break;
    }

    savePath = QString("%1/%2").arg(m_captureConfig.saveDir).arg(fileName);

    return savePath;
}

ImageOutputSaver::ImageOutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output)
    : OutputSaver(cameraCapture, config, output)
{
    m_suffix2D = ".png";
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
        if (m_outputDataPort.hasData(dataType))
        {
            image = m_outputDataPort.getOutputData2D(dataType).image;
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
            if (m_outputDataPort.hasData(dataType))
            {
                image = m_outputDataPort.getOutputData2D(dataType).image;
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
    m_suffix2D = ".raw";
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
        QByteArray saveData;
        QString savePath = getSavePath(CAMERA_DATA_DEPTH);
        if (streamData.dataInfo.format == STREAM_FORMAT_Z16)
        {
            saveData = streamData.data;
        }
        else 
        {
            saveData = streamData.data.left(streamData.data.size() / 2);
        }

        saveDataToFile(savePath, saveData);
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

            QByteArray data = streamData.data.mid(offset2, width * height);
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
