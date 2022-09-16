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

#include "dataexporter.h"

#include <QDebug>
#include <QFile>

#define DEPTH_BIN_SUFFIX "_depth.bin"
#define RGB_BIN_SUFFIX "_rgb.bin"
#define L_BIN_SUFFIX "_depthL.bin"
#define R_BIN_SUFFIX "_depthR.bin"

DataExporter::DataExporter()
{
    moveToThread(&thread);
    thread.start();
}

DataExporter::~DataExporter()
{
    thread.quit();
    thread.wait();
}

void DataExporter::onExportStreamData(StreamData streamData, QImage image, QString filePath)
{
    if (streamData.data.size() <= 0)
    {
        qInfo() << "export failed, data is empty";
        emit exportFinished(false);
        return;
    }

    auto format = streamData.dataInfo.format;
    bool result = false;

    switch (format)
    {
    case STREAM_FORMAT_MJPG:
    case STREAM_FORMAT_RGB8:
        result = exportRgbData(streamData, image, filePath);
        break;
    case STREAM_FORMAT_Z16:
    case STREAM_FORMAT_Z16Y8Y8:
        result = exportDepthData(streamData, image, filePath);
        break;
    case STREAM_FORMAT_PAIR:
        result = exportPairData(streamData, filePath);
        break;
    default:
        break;
    }

    emit exportFinished(result);
}

bool DataExporter::exportRgbData(const StreamData& streamData, const QImage& image, const QString& filePath)
{
    const int width = streamData.dataInfo.width;
    const int height = streamData.dataInfo.height;

    QString fileName = QString("%1%2").arg(filePath).arg(RGB_BIN_SUFFIX);
    //save bin data
    const char* data = (const char*)streamData.data.data();
    const int dataLength = streamData.data.size();
    
    bool result = true;
    result = exportBinData(data, dataLength, fileName);

    //save image
    if (!image.isNull() && image.width() > 0) 
    {
        QString imgFileName = QString("%1%2").arg(filePath).arg(".png");
        result &= exportImage(image, imgFileName);
    }

    return true;
}

bool DataExporter::exportDepthData(const StreamData& streamData, const QImage& image, const QString& filePath)
{
    const int width = streamData.dataInfo.width;
    const int height = streamData.dataInfo.height;
    auto format = streamData.dataInfo.format;

    const int dataLength = streamData.data.size();
    const char* data = (const char*)streamData.data.data();

    const int depthLen = width * height * sizeof(ushort);
    QString fileName = QString("%1%2").arg(filePath).arg(DEPTH_BIN_SUFFIX);

    bool result = true;
    result = exportBinData(data, depthLen, fileName);
    
    int offset = depthLen;
    if (format == STREAM_FORMAT_Z16Y8Y8 && dataLength > width * height * sizeof(ushort))
    {
        const int irLen = width * height;
        fileName = QString("%1%2").arg(filePath).arg(L_BIN_SUFFIX);
        result &= exportBinData(data + offset, irLen, fileName);
        offset += depthLen;

        fileName = QString("%1%2").arg(filePath).arg(R_BIN_SUFFIX);
        result &= exportBinData(data + offset, irLen, fileName);
    }

    //save image
    if (!image.isNull() && image.width() > 0) 
    {
        QString imgFileName = QString("%1%2").arg(filePath).arg(".png");
        result &= exportImage(image, imgFileName);
    }

    return result;
}

bool DataExporter::exportPairData(const StreamData& streamData, const QString& filePath)
{
    const int& width = streamData.dataInfo.width;
    const int& height = streamData.dataInfo.height;

    const int irLen = width * height;
    const char* data = (const char*)streamData.data.data();

    bool result = true;

    QString fileName = QString("%1%2").arg(filePath).arg(L_BIN_SUFFIX);
    result &= exportBinData(data, irLen, fileName);

    fileName = QString("%1%2").arg(filePath).arg(R_BIN_SUFFIX);
    result &= exportBinData(data + irLen, irLen, fileName);

    return result;
}

void DataExporter::onExportPointCloud(cs::Pointcloud pointCloud, QImage image, QString filePath)
{
    if (filePath.endsWith(".ply"))
    {
        qInfo() << "export file : " << filePath;
        pointCloud.exportToFile(filePath.toStdString(), (uchar*)image.bits(), image.width(), image.height());
        emit exportFinished(true);
    }
    else
    {
        qWarning() << "the export file name is invalid, file : " << filePath;
        emit exportFinished(false);
    }
}

bool DataExporter::exportBinData(const char* data, int dataLength, QString fileName)
{
    QFile file(fileName);
    bool result = file.open(QFile::WriteOnly);
    if (!result)
    {
        qWarning() << "export bin failed, open file failed, file : " << fileName;
        return false;
    }

    file.write(data, dataLength);
    file.close();

    return true;
}

bool DataExporter::exportImage(const QImage& image, QString fileName)
{
    bool result = image.save(fileName, "PNG");
    if (!result)
    {
        qWarning() << "export image failed, file : " << fileName;
    }

    return result;
}