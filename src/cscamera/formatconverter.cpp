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

#include "formatconverter.h"
#include "capturedzipparser.h"

#include <QDir>
#include <QDebug>

using namespace cs;

FormatConverter::FormatConverter()
{
    moveToThread(this);
    start();
}

FormatConverter::~FormatConverter()
{
    if (capturedZipParser)
    {
        delete capturedZipParser;
    }
}

void FormatConverter::setSourceFile(QString sourceFile)
{
    this->sourceFile = sourceFile;
}

void FormatConverter::setOutputDirectory(QString output)
{
    outputDirectory = output;
}

void FormatConverter::setWithTexture(bool withTexture)
{
    this->withTexture = withTexture;
}

void FormatConverter::onConvert()
{
    if (getIsConverting())
    {
        qWarning() << "Converting, please wait";
        emit convertStateChanged(CONVERT_FAILED, 0, tr("Converting, please wait"));
        return;
    }

    setIsConverting(true);
    do 
    {
        if (!capturedZipParser)
        {
            capturedZipParser = new CapturedZipParser();
        }

        capturedZipParser->setZipFile(sourceFile);

        int couvertCount = 0;
        emit convertStateChanged(CONVERTING, couvertCount, tr("Converting..."));

        if (!capturedZipParser->checkFileValid())
        {
            qWarning() << "Illegal zip file, or the zip file does not contain CaptureParameters.yaml";
            emit convertStateChanged(CONVERT_FAILED, couvertCount, tr("Invalid zip file, not find CaptureParameters.yaml"));
            break;
        }

        // parse the zip file
        if (!capturedZipParser->parseCaptureInfo())
        {
            qWarning() << "Parse zip file failed";
            emit convertStateChanged(CONVERT_FAILED, couvertCount, tr("Parse zip file failed"));
            break;
        }

        QVector<int> dataTypes = capturedZipParser->getDataTypes();
        if (!dataTypes.contains(CAMERA_DATA_DEPTH))
        {
            qWarning() << "convert failed, no depth data";
            emit convertStateChanged(CONVERT_FAILED, couvertCount, tr("No depth data"));
            break;
        }

        QDir outputDir(outputDirectory);
        if (!outputDir.exists())
        {
            if (!outputDir.mkpath(outputDirectory))
            {
                qWarning() << "make path failed, dir:" << outputDirectory;
                emit convertStateChanged(CONVERT_FAILED, couvertCount, tr("Failed to create folder"));
                break;
            }
        }

        const int totalCount = capturedZipParser->getFrameCount();
        QString fileName = capturedZipParser->getCaptureName();

        int successCount = 0;
        for (couvertCount = 0; couvertCount < totalCount; couvertCount++)
        {
            if (getInterruptConvert())
            {
                break;
            }

            Pointcloud pc;
            QImage texImage;
            int rgbIndex = couvertCount;

            // If the timestamp is valid, find the RGB frame index through the timestamp
            if (withTexture && capturedZipParser->getIsTimeStampsValid())
            {
                rgbIndex = capturedZipParser->getRgbFrameIndexByTimeStamp(couvertCount);
            }

            if (!capturedZipParser->generatePointCloud(couvertCount, rgbIndex, withTexture, pc, texImage))
            {
                qWarning() << "Failed to generate point cloud";
                int progress = couvertCount * 1.0 / totalCount * 100;
                emit convertStateChanged(CONVERT_ERROR, progress, tr("Failed to generate point cloud"));
                continue;
            }

            QString savePath = QString("%1/%2-%3.ply").arg(outputDirectory).arg(fileName).arg(couvertCount, 4, 10, QChar('0'));
            if (withTexture && !texImage.isNull())
            {
                pc.exportToFile(savePath.toStdString(), texImage.bits(), texImage.width(), texImage.height());
            }
            else
            {
                pc.exportToFile(savePath.toStdString(), nullptr, 0, 0);
            }

            successCount++;

            int progress = successCount * 1.0 / totalCount * 100;
            emit convertStateChanged(CONVERTING, progress, tr("Converting..."));
        }

        int progress = successCount * 1.0 / totalCount * 100;
        auto state = (successCount == totalCount) ? CONVERT_SUCCESS : CONVERT_FAILED;
        emit convertStateChanged(state, progress, QString(tr("Conversion completed, %1 successful, %2 failed").arg(successCount).arg(totalCount - successCount)));

    } while (false);

    setInterruptConvert(false);
    setIsConverting(false);
}

bool FormatConverter::getIsConverting()
{
    return isConverting;
}

void FormatConverter::setIsConverting(bool value)
{
    isConverting = value;
}

void FormatConverter::setInterruptConvert(bool value)
{
    interruptConvert = value;
}

bool FormatConverter::getInterruptConvert()
{
    return interruptConvert;
}

