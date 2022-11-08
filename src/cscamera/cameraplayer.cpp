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
#include <JlCompress.h>
#include <yaml-cpp/yaml.h>
#include <QDebug>
#include <QImage>
#include <imageutil.h>

using namespace cs;

CameraPlayer::CameraPlayer(QObject* parent)
    : QThread(parent)
{
    moveToThread(this);
    start();
}

CameraPlayer::~CameraPlayer()
{

}

void CameraPlayer::onLoadFile(QString file)
{
    emit playerStateChanged(PLAYER_LOADING, tr("Loading file..."));

    if (!checkFileValid(file))
    {
        qWarning() << "Illegal zip file, or the zip file does not contain CaptureParameters.yaml";
        // notify to ui
        emit playerStateChanged(PLAYER_ERROR, tr("Invalid zip file, not find CaptureParameters.yaml"));
        return;
    }

    zipFile = file;
    // parse the zip file
    if (!parseZipFile())
    {
        emit playerStateChanged(PLAYER_ERROR, tr("Parse zip file failed"));
        return;
    }

    emit playerStateChanged(PLAYER_READY, tr("Ready to play"));
}

void CameraPlayer::onPalyFrameUpdated(int curFrame, bool force)
{
    if (currentFrame != curFrame || force)
    {
        currentFrame = curFrame;
        updateCurrentFrame();
    }
}

void CameraPlayer::updateCurrentFrame()
{
    for (auto type : currentDataTypes)
    {
        CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)type;
        switch (dataType)
        {
        case CAMERA_DATA_L:
        case CAMERA_DATA_R:
        case CAMERA_DATA_RGB:
        case CAMERA_DATA_DEPTH:
            updateCurrentImage(dataType);
            break;
        case CAMERA_DTA_POINT_CLOUD:
            updateCurrentPointCloud();
            break;
        }
    }
}

QVector<CS_CAMERA_DATA_TYPE> CameraPlayer::getDataTypes()
{
    QVector<CS_CAMERA_DATA_TYPE> types;
    
    for (auto type : dataTypes)
    {
        if (type == "IR(L)")
        {
            if (!types.contains(CAMERA_DATA_L))
            {
                types.push_back(CAMERA_DATA_L);
            }
            continue;
        }
        if (type == "IR(R)")
        {
            if (!types.contains(CAMERA_DATA_R))
            {
                types.push_back(CAMERA_DATA_R);
            }
            continue;
        }
        if (type == "Depth")
        {
            if (!types.contains(CAMERA_DATA_DEPTH))
            {
                types.push_back(CAMERA_DATA_DEPTH);
            }

            if (!types.contains(CAMERA_DTA_POINT_CLOUD))
            {
                types.push_back(CAMERA_DTA_POINT_CLOUD);
            }
            continue;
        }
        if (type == "RGB")
        {
            if (!types.contains(CAMERA_DATA_RGB))
            {
                types.push_back(CAMERA_DATA_RGB);
            }
            continue;
        }
        if (type == "Point Cloud")
        {
            if (!types.contains(CAMERA_DTA_POINT_CLOUD))
            {
                types.push_back(CAMERA_DTA_POINT_CLOUD);
            }
            continue;
        }
    }
    return types;
}

int CameraPlayer::getFrameNumber()
{
    return frameNumber;
}

void CameraPlayer::currentDataTypesUpdated(QVector<int> dataTypes)
{
    currentDataTypes = dataTypes;
}

bool CameraPlayer::checkFileValid(QString file)
{
    qInfo() << "check zip file is valid or not";

    QuaZip zip(file);
    zip.open(QuaZip::mdUnzip);
    if (!zip.isOpen())
    {
        qWarning() << "Open zip file failed, file:" << file;
        return false;
    }

    bool result = zip.setCurrentFile("CaptureParameters.yaml");
    zip.close();

    return result;
}

Intrinsics genIntrinsicsFromNode(YAML::Node node)
{
    Intrinsics intrinsics;

    try
    {
        intrinsics.width = node["width"].as<int>();
        intrinsics.height = node["height"].as<int>();

        YAML::Node matrix = node["matrix"];
        YAML::Node data = matrix["data"];

        intrinsics.fx      = data[0].as<float>();
        intrinsics.zero01  = data[1].as<float>();
        intrinsics.cx      = data[2].as<float>();
        intrinsics.zeor10  = data[3].as<float>();
        intrinsics.fy      = data[4].as<float>();
        intrinsics.cy      = data[5].as<float>();
        intrinsics.zeor20  = data[6].as<float>();
        intrinsics.zero21  = data[7].as<float>();
        intrinsics.one22   = data[8].as<float>();
        
    }
    catch (const YAML::Exception& e)
    {
        qWarning() << "gen intrinsics error, " << e.msg.c_str();
    }

    return intrinsics;
}

Extrinsics genExtrinsicsFromNode(YAML::Node node)
{
    Extrinsics extrinsics;

    try
    {
        YAML::Node rotation    = node["rotation"]["data"];
        YAML::Node translation = node["translation"]["data"];

        extrinsics.rotation[0] = rotation[0].as<float>();
        extrinsics.rotation[1] = rotation[1].as<float>();
        extrinsics.rotation[2] = rotation[2].as<float>();
        extrinsics.rotation[3] = rotation[3].as<float>();
        extrinsics.rotation[4] = rotation[4].as<float>();
        extrinsics.rotation[5] = rotation[5].as<float>();
        extrinsics.rotation[6] = rotation[6].as<float>();
        extrinsics.rotation[7] = rotation[7].as<float>();
        extrinsics.rotation[8] = rotation[8].as<float>();

        extrinsics.translation[0] = translation[0].as<float>();
        extrinsics.translation[1] = translation[1].as<float>();
        extrinsics.translation[2] = translation[2].as<float>();
    }
    catch (const YAML::Exception& e)
    {
        qWarning() << "gen extrinsics error, " << e.msg.c_str();
    }

    return extrinsics;
}

bool CameraPlayer::parseZipFile()
{
    qInfo() << "Parse zip file";

    bool result = true;

    QuaZip zip(zipFile);
    zip.open(QuaZip::mdUnzip);
    zip.setCurrentFile("CaptureParameters.yaml");

    QuaZipFile file(&zip);
    file.open(QIODevice::ReadOnly);

    YAML::Node node = YAML::Load((const char*)file.readAll().data());
    file.close();
    zip.close();

    try
    {
        // name
        playName = node["Name"].as<std::string>().c_str();

        // frame number
       frameNumber = node["Frame Number"].as<int>();

        // data types
        YAML::Node data = node["Data Types"].as<YAML::Node>();
        dataTypes.clear();
        for (int i = 0; i < data.size(); i++)
        {
            std::string s = data[i].as<std::string>();
            dataTypes.push_back(s.c_str());
        }

        // data format
        dataFormat = node["Save Format"].as<std::string>().c_str();
    }
    catch (const YAML::Exception& e)
    {
        qWarning() << "Parse yaml file error, " << e.msg.c_str() << " " << e.what();
        result = false;
    }
   
    // camera parameters
    try
    {
        // depth resolution
        YAML::Node nodeTmp = node["Depth resolution"];
        if (nodeTmp.IsDefined())
        {
            int width = nodeTmp["width"].as<int>();
            int height = nodeTmp["height"].as<int>();;

            depthResolution.setWidth(width);
            depthResolution.setHeight(height);
        }

        // RGB resolution
        nodeTmp = node["RGB resolution"];
        if (nodeTmp.IsDefined())
        {
            int width = nodeTmp["width"].as<int>();
            int height = nodeTmp["height"].as<int>();;

            rgbResolution.setWidth(width);
            rgbResolution.setHeight(height);
        }

        // depth intrinsics
        nodeTmp = node["Depth intrinsics"];
        if (nodeTmp.IsDefined())
        {
            depthIntrinsics = genIntrinsicsFromNode(nodeTmp);
        }

        // rgb intrinsics
        nodeTmp = node["RGB intrinsics"];
        if (nodeTmp.IsDefined())
        {
            rgbIntrinsics = genIntrinsicsFromNode(nodeTmp);
        }

        // extrinsics
        nodeTmp = node["Extrinsics"];
        if (nodeTmp.IsDefined())
        {
            extrinsics = genExtrinsicsFromNode(nodeTmp);
        }

        // depth scale
        depthScale = node["Depth Scale"].as<float>();
    }
    catch (const YAML::Exception& e)
    {
        qWarning() << "Parse yaml file error, " << e.msg.c_str();
        result = false;
    }

    qInfo() << "Parse zip file end";
    return result;
}

bool CameraPlayer::readDataFromZip(QString fileName, QByteArray& data)
{
    bool result = true;
    do 
    {
        QuaZip zip(zipFile);
        result = zip.open(QuaZip::mdUnzip);
        if (!result)
        {
            qWarning() << "open zip file failed, file:" << zipFile;
            break;
        }

        result = zip.setCurrentFile(fileName);
        if (!result)
        {
            qWarning() << "set current file failed, file name:" << fileName;
            break;
        }

        QuaZipFile file(&zip);
        result = file.open(QIODevice::ReadOnly);
        if (!result)
        {
            qWarning() << "open file failed, file name:" << fileName;
            break;
        }
        data = file.readAll();

    } while (false);

    return result;
}

void CameraPlayer::updateCurrentImage(int type)
{
    if (dataFormat == "raw")
    {
        updateCurrentFromRaw(type);
    }
    else 
    {
        updateCurrentFromPng(type);
    }
}

void CameraPlayer::updateCurrentPointCloud()
{
    if (dataFormat == "raw")
    {
        genPointCloudFromDepthData();
    }
    else 
    {
        genPointCloudFromDepthImage();
    }
}

QImage CameraPlayer::genDepthImage(int width, int height, QByteArray data)
{
    cs::colorizer color;
    color.setRange(50, 2000);

    QImage image = QImage(width, height, QImage::Format_RGB888);
    color.process<ushort>((ushort*)data.data(), depthScale, image.bits(), width * height);

    return image;
}

void CameraPlayer::updateCurrentFromPng(int type)
{
    QImage image = genImageFromPngData(type);

    OutputData2D outputData;
    outputData.image = image;
    outputData.info.cameraDataType = type;

    emit output2DUpdated(outputData);
}

bool CameraPlayer::genPointCloudFromDepthImage()
{
    QString fileName = getCurrentFileName(CAMERA_DATA_DEPTH) + ".png";

    QByteArray pngData;
    // read data from zip file
    if (!readDataFromZip(fileName, pngData))
    {
        emit playerStateChanged(PLAYER_ERROR, tr("Failed to read data"));
        return false;
    }

    QByteArray pixData;
    int width, height, bitDepth;
    if (!ImageUtil::genPixDataFromPngData(pngData, width, height, bitDepth, pixData))
    {
        qWarning() << "generate pixel data failed.";
        return false;
    }

    Pointcloud pc;
    
    QImage texImage;
    if (show3dTexture)
    {
        texImage = genImageFromPngData(CAMERA_DATA_RGB);
        pc.generatePoints<ushort>((ushort*)pixData.data(), width, height, depthScale, &depthIntrinsics, &rgbIntrinsics, &extrinsics, true);
    }
    else 
    {
        pc.generatePoints<ushort>((ushort*)pixData.data(), width, height, depthScale, &depthIntrinsics, nullptr, nullptr, true);
    }

    emit output3DUpdated(pc, texImage);

    return true;
}

bool CameraPlayer::genPointCloudFromDepthData()
{
    QString fileName = getCurrentFileName(CAMERA_DATA_DEPTH) + ".raw";

    QByteArray data;
    // read data from zip file
    if (!readDataFromZip(fileName, data))
    {
        emit playerStateChanged(PLAYER_ERROR, tr("Failed to read data"));
        return false;
    }

    int width = depthResolution.width();
    int height = depthResolution.height();

    Pointcloud pc;
    QImage texImage;
    if (show3dTexture)
    {
        texImage =  genImageFromPixelData(CAMERA_DATA_RGB);
        pc.generatePoints<ushort>((ushort*)data.data(), width, height, depthScale, &depthIntrinsics, &rgbIntrinsics, &extrinsics, true);
    }
    else
    {
        pc.generatePoints<ushort>((ushort*)data.data(), width, height, depthScale, &depthIntrinsics, nullptr, nullptr, true);
    }

    emit output3DUpdated(pc, texImage);

    return true;
}

void CameraPlayer::updateCurrentFromRaw(int type)
{
    QImage image = genImageFromPixelData(type);

    OutputData2D outputData;
    outputData.image = image;
    outputData.info.cameraDataType = type;

    emit output2DUpdated(outputData);
}

QImage CameraPlayer::genImageFromPngData(int type)
{
    QString fileName = getCurrentFileName((CS_CAMERA_DATA_TYPE)type) + ".png";
    QByteArray data;

    // read data from zip file
    if (!readDataFromZip(fileName, data))
    {
        emit playerStateChanged(PLAYER_ERROR, tr("Failed to read data"));
        return QImage();
    }

    QImage image;
    if (type == CAMERA_DATA_DEPTH)
    {
        QByteArray pixData;
        int width, height, bitDepth;
        if (!ImageUtil::genPixDataFromPngData(data, width, height, bitDepth, pixData))
        {
            qWarning() << "generate pixel data failed.";
            return QImage();
        }
        image = genDepthImage(width, height, pixData);
    }
    else
    {
        image = QImage::fromData(data, "PNG");
    }

    return image;
}

QImage CameraPlayer::genImageFromPixelData(int type)
{
    QString fileName = getCurrentFileName((CS_CAMERA_DATA_TYPE)type) + ".raw";
    QByteArray data;

    // read data from zip file
    if (!readDataFromZip(fileName, data))
    {
        emit playerStateChanged(PLAYER_ERROR, tr("Failed to read data"));
        return QImage();
    }

    QImage image;
    if (type == CAMERA_DATA_DEPTH)
    {
        int width = depthResolution.width();
        int height = depthResolution.height();

        image = genDepthImage(width, height, data);
    }
    else  if (type == CAMERA_DATA_RGB)
    {
        int width = rgbResolution.width();
        int height = rgbResolution.height();
        image = QImage((const uchar*)data.data(), width, height, QImage::Format_RGB888);
    }
    else
    {
        int width = depthResolution.width();
        int height = depthResolution.height();
        image = QImage((const uchar*)data.data(), width, height, QImage::Format_Grayscale8);
    }

    image = image.copy(image.rect());

    return image;
}

QString CameraPlayer::getCurrentFileName(CS_CAMERA_DATA_TYPE dataType)
{
    QString fileName;

    switch (dataType)
    {
    case CAMERA_DATA_L:
        fileName = QString("%1-ir-L-%2").arg(playName).arg(currentFrame - 1, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_R:
        fileName = QString("%1-ir-R-%2").arg(playName).arg(currentFrame - 1, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_DEPTH:
        fileName = QString("%1-depth-%2").arg(playName).arg(currentFrame - 1, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_RGB:
        fileName = QString("%1-RGB-%2").arg(playName).arg(currentFrame - 1, 4, 10, QChar('0'));
        break;
    case CAMERA_DTA_POINT_CLOUD:
        fileName = QString("%1-%2.ply").arg(playName).arg(currentFrame - 1, 4, 10, QChar('0'));
        break;
    default:
        break;
    }

    return fileName;
}

void CameraPlayer::onShow3DTextureChanged(bool show)
{
    show3dTexture = show;
}