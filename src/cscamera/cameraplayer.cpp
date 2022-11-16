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