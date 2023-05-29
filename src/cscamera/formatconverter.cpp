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

    connect(this, &FormatConverter::loadFileSignal, this, &FormatConverter::onLoadFile, Qt::QueuedConnection);
}

FormatConverter::~FormatConverter()
{
    if (m_capturedZipParser)
    {
        delete m_capturedZipParser;
    }
}

void FormatConverter::setSourceFile(QString sourceFile)
{
    this->m_sourceFile = sourceFile;
}

void FormatConverter::setOutputDirectory(QString output)
{
    m_outputDirectory = output;
}

void FormatConverter::setWithTexture(bool withTexture)
{
    this->m_withTexture = withTexture;
}

void FormatConverter::onLoadFile()
{
    do 
    {
        m_isFileValid = true;
        if (!m_capturedZipParser)
        {
            m_capturedZipParser = new CapturedZipParser();
        }

        m_capturedZipParser->setZipFile(m_sourceFile);

        emit convertStateChanged(CONVERT_LOADING, 0, tr("Loading file..."));

        if (!m_capturedZipParser->checkFileValid())
        {
            qWarning() << "Illegal zip file, or the zip file does not contain CaptureParameters.yaml";
            emit convertStateChanged(CONVERT_LOADING_FAILED, 0, tr("Invalid zip file, not find CaptureParameters.yaml"));
            m_isFileValid = false;
            break;
        }

        // parse the zip file
        if (!m_capturedZipParser->parseCaptureInfo())
        {
            qWarning() << "Parse zip file failed";
            emit convertStateChanged(CONVERT_LOADING_FAILED, 0, tr("Parse zip file failed"));
            m_isFileValid = false;
            break;
        }

        QVector<int> dataTypes = m_capturedZipParser->getDataTypes();
        if (!dataTypes.contains(CAMERA_DATA_DEPTH))
        {
            qWarning() << "convert failed, no depth data";
            emit convertStateChanged(CONVERT_LOADING_FAILED, 0, tr("No depth data"));
            m_isFileValid = false;
            break;
        }

        // has RGB data or not
        m_hasRGBData = dataTypes.contains(CAMERA_DATA_RGB);
        emit convertStateChanged(CONVERT_READDY, 0, "");

    } while (false);
}

void FormatConverter::onConvert()
{
    if (getIsConverting())
    {
        qWarning() << "Converting, please wait";
        emit convertStateChanged(CONVERT_FAILED, 0, tr("Converting, please wait"));
        return;
    }

    if (!m_isFileValid)
    {
        emit convertStateChanged(CONVERT_FAILED, 0, tr("Parse zip file failed"));
        return;
    }

    setIsConverting(true);
    emit convertStateChanged(CONVERTING, 0, tr("Converting..."));

    do 
    {
        QDir outputDir(m_outputDirectory);
        if (!outputDir.exists())
        {
            if (!outputDir.mkpath(m_outputDirectory))
            {
                qWarning() << "make path failed, dir:" << m_outputDirectory;
                emit convertStateChanged(CONVERT_FAILED, 0, tr("Failed to create folder"));
                m_isFileValid = false;
                break;
            }
        }

        int couvertCount = 0;
        const int totalCount = m_capturedZipParser->getFrameCount();
        QString fileName = m_capturedZipParser->getCaptureName();

        int successCount = 0;
        bool convertWithTexture = m_withTexture && m_hasRGBData;

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
            if (convertWithTexture  && m_capturedZipParser->getIsTimeStampsValid())
            {
                rgbIndex = m_capturedZipParser->getRgbFrameIndexByTimeStamp(couvertCount);
            }

            if (!m_capturedZipParser->generatePointCloud(couvertCount, rgbIndex, convertWithTexture, pc, texImage))
            {
                qWarning() << "Failed to generate point cloud";
                int progress = couvertCount * 1.0 / totalCount * 100;
                emit convertStateChanged(CONVERT_ERROR, progress, tr("Failed to generate point cloud"));
                continue;
            }

            QString savePath = QString("%1/%2-%3.ply").arg(m_outputDirectory).arg(fileName).arg(couvertCount, 4, 10, QChar('0'));
            QByteArray pathData = savePath.toLocal8Bit();
            std::string savePathNew = pathData.data();

            if (convertWithTexture && !texImage.isNull())
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
    return m_isConverting;
}

void FormatConverter::setIsConverting(bool value)
{
    m_isConverting = value;
}

void FormatConverter::setInterruptConvert(bool value)
{
    m_interruptConvert = value;
}

bool FormatConverter::getInterruptConvert()
{
    return m_interruptConvert;
}

bool FormatConverter::getHasRGBData() const
{
    return m_hasRGBData;
}

void FormatConverter::setHasRGBData(bool value)
{
    m_hasRGBData = value;
}

