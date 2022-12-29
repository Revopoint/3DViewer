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

#include "cameraplayer.h"
#include <QDebug>
#include <QImage>
#include <QReadLocker>
#include <QWriteLocker>
#include "capturedzipparser.h"

using namespace cs;

CameraPlayer::CameraPlayer(QObject* parent)
    : QThread(parent)
    , zipParser(new CapturedZipParser())
{
    moveToThread(this);
    start();
}

CameraPlayer::~CameraPlayer()
{
    delete zipParser;
}

void CameraPlayer::onLoadFile(QString file)
{
    emit playerStateChanged(PLAYER_LOADING, tr("Loading file..."));
    zipParser->setZipFile(file);

    if (!zipParser->checkFileValid())
    {
        qWarning() << "Illegal zip file, or the zip file does not contain CaptureParameters.yaml";
        // notify to ui
        emit playerStateChanged(PLAYER_ERROR, tr("Invalid zip file, or the zip file does not contain CaptureParameters.yaml"));
        return;
    }

    // parse the zip file
    if (!zipParser->parseCaptureInfo())
    {
        emit playerStateChanged(PLAYER_ERROR, tr("Parse zip file failed"));
        return;
    }

    emit playerStateChanged(PLAYER_READY, tr("Ready to play"));
}

void CameraPlayer::onPalyFrameUpdated(int curFrame, bool force)
{
    if (curFrame < 1)
    {
        return;
    }

    if (currentFrame != curFrame || force)
    {
        currentFrame = curFrame;
        updateCurrentFrame();
    }
}

void CameraPlayer::updateCurrentFrame()
{
    QReadLocker locker(&lock);
   
    for (auto type : currentDataTypes)
    {
        switch (type)
        {
        case CAMERA_DATA_L:
        case CAMERA_DATA_R:
        case CAMERA_DATA_DEPTH:
        case CAMERA_DATA_RGB:
            updateCurrentImage(type);
            break;
        case CAMERA_DATA_POINT_CLOUD:
            updateCurrentPointCloud();
            break;
        default:
            break;
        }
    }
}

QVector<int> CameraPlayer::getDataTypes()
{
    QVector<int> dataTypes = zipParser->getDataTypes();
    if (dataTypes.contains(CAMERA_DATA_DEPTH) && !dataTypes.contains(CAMERA_DATA_POINT_CLOUD))
    {
        dataTypes.push_back(CAMERA_DATA_POINT_CLOUD);
    }

    return dataTypes;
}

int CameraPlayer::getFrameNumber()
{
    return zipParser->getFrameCount();
}

void CameraPlayer::currentDataTypesUpdated(QVector<int> dataTypes)
{
    QWriteLocker locker(&lock);
    currentDataTypes = dataTypes;
}

void CameraPlayer::updateCurrentImage(int type)
{
    // If the timestamp is valid, find the RGB frame index through the timestamp
    int frameIndex = currentFrame - 1;
    if (type == CAMERA_DATA_RGB && zipParser->getIsTimeStampsValid())
    {
        int rgbIndex = zipParser->getRgbFrameIndexByTimeStamp(frameIndex);
        qInfo() << "update rgb frame, depth index : " << frameIndex << ", rgb index:" << rgbIndex;

        frameIndex = rgbIndex;
    }

    QImage image = zipParser->getImageOfFrame(frameIndex, type);
    if (image.isNull())
    {
        qWarning() << "Failed to generate image";
        emit playerStateChanged(PLAYER_ERROR, tr("Failed to generate image"));
        return;
    }

    OutputData2D outputData;
    outputData.image = image;
    outputData.info.cameraDataType = type;

    emit output2DUpdated(outputData);
}

void CameraPlayer::updateCurrentPointCloud()
{
    auto dataTypes = zipParser->getDataTypes();
    Pointcloud pc;
    QImage texImage;

    if (dataTypes.contains(CAMERA_DATA_POINT_CLOUD))
    {
        // generate Pointcloud from ply file
        if (!zipParser->getPointCloud(currentFrame - 1, pc, texImage))
        {
            emit playerStateChanged(PLAYER_ERROR, tr("Failed to generate point cloud"));
            return;
        }

        if (texImage.isNull())
        {
            // generate Pointcloud from depth data 
            int rgbFrame = currentFrame - 1;
            // If the timestamp is valid, find the RGB frame index through the timestamp
            if (show3dTexture && zipParser->getIsTimeStampsValid())
            {
                rgbFrame = zipParser->getRgbFrameIndexByTimeStamp(currentFrame - 1);
            }

            texImage = zipParser->getImageOfFrame(rgbFrame, CAMERA_DATA_RGB);
        }
    }
    else 
    {
        // generate Pointcloud from depth data 
        int rgbFrame = currentFrame - 1;
        // If the timestamp is valid, find the RGB frame index through the timestamp
        if (show3dTexture && zipParser->getIsTimeStampsValid())
        {
            rgbFrame = zipParser->getRgbFrameIndexByTimeStamp(currentFrame - 1);
        }

        if (!zipParser->generatePointCloud(currentFrame - 1, rgbFrame, show3dTexture, pc, texImage))
        {
            qWarning() << "Failed to generate point cloud";
            emit playerStateChanged(PLAYER_ERROR, tr("Failed to generate point cloud"));
            return;
        }
    }

    emit output3DUpdated(pc, texImage);
}

void CameraPlayer::onShow3DTextureChanged(bool show)
{
    show3dTexture = show;
}

void CameraPlayer::onSaveCurrentFrame(QString filePath)
{
    emit playerStateChanged(PLAYER_SAVING, tr("Saving current frame"));

    if (zipParser->saveFrameToLocal(currentFrame - 1, show3dTexture, filePath))
    {
        emit playerStateChanged(PLAYER_SAVE_SUCCESS, tr("Save current frame successfuly"));
    }
    else 
    {
        emit playerStateChanged(PLAYER_SAVE_FAILED, tr("Save current frame failed"));
    }
}

bool CameraPlayer::enablePointCloudTexture()
{
    return zipParser->enablePointCloudTexture();
}