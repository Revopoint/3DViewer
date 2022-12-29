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
            QByteArray pathData = savePath.toLocal8Bit();
            std::string savePathNew = pathData.data();

            if (withTexture && !texImage.isNull())
            {
                pc.exportToFile(savePathNew, texImage.bits(), texImage.width(), texImage.height());
            }
            else
            {
                pc.exportToFile(savePathNew, nullptr, 0, 0);
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

