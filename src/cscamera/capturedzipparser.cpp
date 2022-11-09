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
#include "capturedzipparser.h"
#include <imageutil.h>
#include <JlCompress.h>
#include <yaml-cpp/yaml.h>
#include <QDebug>
#include <QMap>
#include <CMath>
#include <QTextStream>
#include <limits.h>

using namespace cs;

static Intrinsics genIntrinsicsFromNode(YAML::Node node)
{
    Intrinsics intrinsics;

    try
    {
        intrinsics.width = node["width"].as<int>();
        intrinsics.height = node["height"].as<int>();

        YAML::Node matrix = node["matrix"];
        YAML::Node data = matrix["data"];

        intrinsics.fx = data[0].as<float>();
        intrinsics.zero01 = data[1].as<float>();
        intrinsics.cx = data[2].as<float>();
        intrinsics.zeor10 = data[3].as<float>();
        intrinsics.fy = data[4].as<float>();
        intrinsics.cy = data[5].as<float>();
        intrinsics.zeor20 = data[6].as<float>();
        intrinsics.zero21 = data[7].as<float>();
        intrinsics.one22 = data[8].as<float>();

    }
    catch (const YAML::Exception& e)
    {
        qWarning() << "gen intrinsics error, " << e.msg.c_str();
    }

    return intrinsics;
}

static Extrinsics genExtrinsicsFromNode(YAML::Node node)
{
    Extrinsics extrinsics;

    try
    {
        YAML::Node rotation = node["rotation"]["data"];
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

QMap<QString, int> dataTypeMap =
{
    { "IR(L)", CAMERA_DATA_L },
    { "IR(R)", CAMERA_DATA_R },
    { "Depth", CAMERA_DATA_DEPTH },
    { "RGB", CAMERA_DATA_RGB },
    { "Point Cloud", CAMERA_DATA_POINT_CLOUD }
};

CapturedZipParser::CapturedZipParser(QString filePath)
    : zipFile(filePath)
{

}

CapturedZipParser::~CapturedZipParser()
{

}

void CapturedZipParser::setZipFile(QString filePath)
{
    zipFile = filePath;
}

bool CapturedZipParser::checkFileValid()
{
    qInfo() << "check zip file is valid or not";

    QuaZip zip(zipFile);
    zip.open(QuaZip::mdUnzip);
    if (!zip.isOpen())
    {
        qWarning() << "Open zip file failed, file:" << zipFile;
        return false;
    }

    bool result = zip.setCurrentFile("CaptureParameters.yaml");
    zip.close();

    return result;
}

QByteArray CapturedZipParser::getFrameData(int frameIndex, int dataType)
{
    QByteArray data;
    do
    {
        bool result = true;

        QuaZip zip(zipFile);
        result = zip.open(QuaZip::mdUnzip);
        if (!result)
        {
            qWarning() << "open zip file failed, file:" << zipFile;
            break;
        }

        QString fileName = getFileName(frameIndex, dataType);
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

    return data;
}

QImage CapturedZipParser::getImageOfFrame(int frameIndex, int dataType)
{
    QByteArray data = getFrameData(frameIndex, dataType);
    if (data.isEmpty())
    {
        qWarning("get frame data from zip file failed");
        return QImage();
    }

    if (isRawFormat())
    {
        QImage image = convertPixels2QImage(data, dataType);
        return image;
    }
    else
    {
        QImage image = convertPng2QImage(data, dataType);
        return image;
    }
}

bool CapturedZipParser::generatePointCloud(int depthIndex, int rgbIndex, bool withTexture, Pointcloud& pc, QImage& tex)
{
    ushort* dataPtr = nullptr;

    QByteArray data = getFrameData(depthIndex, CAMERA_DATA_DEPTH);
    if (data.isNull() || data.isEmpty())
    {
        qWarning() << "depth frame data is empty";
        return false;
    }

    QByteArray pixData = data;
    if (!isRawFormat())
    {
        int width, height, bitDepth;
        if (!ImageUtil::genPixDataFromPngData(data, width, height, bitDepth, pixData))
        {
            qWarning() << "generate pixel data failed.";
            return false;
        }
    }
    dataPtr = (ushort*)pixData.data();

    int width = depthResolution.width();
    int height = depthResolution.height();

    if (withTexture)
    {
        tex = getImageOfFrame(rgbIndex, CAMERA_DATA_RGB);
        pc.generatePoints<ushort>((ushort*)pixData.data(), width, height, depthScale, &depthIntrinsics, &rgbIntrinsics, &extrinsics, true);
    }
    else
    {
        pc.generatePoints<ushort>((ushort*)pixData.data(), width, height, depthScale, &depthIntrinsics, nullptr, nullptr, true);
    }

    return true;
}

QImage CapturedZipParser::convertPng2QImage(QByteArray data, int dataType)
{
    QImage image;
    if (dataType == CAMERA_DATA_DEPTH)
    {
        QByteArray pixData;
        int width, height, bitDepth;
        if (!ImageUtil::genPixDataFromPngData(data, width, height, bitDepth, pixData))
        {
            qWarning() << "generate pixel data failed.";
            return QImage();
        }

        image = convertData2QImage(width, height, pixData);
    }
    else
    {
        image = QImage::fromData(data, "PNG"); // load format is RGB32
        image = image.convertToFormat(QImage::Format_RGB888);
    }

    return image;
}

QImage CapturedZipParser::convertPixels2QImage(QByteArray data, int dataType)
{
    QImage image;

    int width = (dataType == CAMERA_DATA_RGB) ? rgbResolution.width() : depthResolution.width();
    int height = (dataType == CAMERA_DATA_RGB) ? rgbResolution.height() : depthResolution.height();

    if (dataType == CAMERA_DATA_DEPTH)
    {
        image = convertData2QImage(width, height, data);
    }
    else  if (dataType == CAMERA_DATA_RGB)
    {
        image = QImage((const uchar*)data.data(), width, height, QImage::Format_RGB888);
    }
    else
    {
        image = QImage((const uchar*)data.data(), width, height, QImage::Format_Grayscale8);
    }

    if (!image.isNull())
    {
        image = image.copy(image.rect());
    }
    return image;
}

QImage CapturedZipParser::convertData2QImage(int width, int height, QByteArray data)
{
    cs::colorizer color;
    color.setRange(depthMin, depthMax);

    QImage image = QImage(width, height, QImage::Format_RGB888);
    color.process<ushort>((ushort*)data.data(), depthScale, image.bits(), width * height);
    return image;
}

bool CapturedZipParser::parseCaptureInfo()
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
        captureName = node["Name"].as<std::string>().c_str();

        // frame number
        capturedFrameCount = node["Frame Number"].as<int>();

        // data types
        YAML::Node data = node["Data Types"].as<YAML::Node>();
        dataTypes.clear();
        for (int i = 0; i < data.size(); i++)
        {
            std::string s = data[i].as<std::string>();

            QString type = s.c_str();
            Q_ASSERT(dataTypeMap.contains(type));
            dataTypes.push_back(dataTypeMap[type]);
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
        depthMin = node["Depth Min"].as<float>();
        depthMax = node["Depth Max"].as<float>();

    }
    catch (const YAML::Exception& e)
    {
        qWarning() << "Parse yaml file error, " << e.msg.c_str();
        result = false;
    }

    // time stamps
    if (!parseTimeStamps())
    {
        result = false;
    }

    qInfo() << "Parse zip file end";
    return result;
}

bool CapturedZipParser::parseTimeStamps()
{
    bool result = true;

    do
    {
        QuaZip zip(zipFile);
        if (!zip.open(QuaZip::mdUnzip))
        {
            result = false;
            break;
        }

        if (!zip.setCurrentFile("TimeStamps.txt"))
        {
            zip.close();
            result = false;
            break;
        }

        QuaZipFile file(&zip);
        file.open(QIODevice::ReadOnly);
        if (!file.isOpen())
        {
            result = false;
            break;
        }

        QTextStream ss(&file);

        int i = 0;
        bool findHeader = false;
        // depth time stamps
        depthTimeStamps.clear();
        while (!ss.atEnd() && i < capturedFrameCount)
        {
            QString line = ss.readLine();
            if (!findHeader)
            {
                if (line.contains("Depth"))
                {
                    findHeader = true;
                }
                continue;
            }

            // get time stamp
            auto arr = line.split("=");
            if (arr.size() >= 2)
            {
                int timeStamp = arr.last().toInt();
                depthTimeStamps.push_back(timeStamp);
            }

            i++;
        }

        // rgb time stamps
        rgbTimeStamps.clear();
        findHeader = false;
        i = 0;
        while (!ss.atEnd() && i < capturedFrameCount)
        {
            QString line = ss.readLine();
            if (!findHeader)
            {
                if (line.contains("RGB"))
                {
                    findHeader = true;
                }

                continue;
            }

            // get time stamp
            auto arr = line.split("=");
            if (arr.size() >= 2)
            {
                int timeStamp = arr.last().toInt();
                rgbTimeStamps.push_back(timeStamp);
            }

            i++;
        }
         
    } while (false);


    // check the time stamps in zip file 
    if (!checkTimeStampsValid())
    {
        qInfo() << "Time stamps is valid";
    }
    else
    {
        qInfo() << "Time stamps is invalid";
    }

    return result;
}

bool CapturedZipParser::checkTimeStampsValid()
{
    int zeroCount = 0;
    const qreal oneQuarter = 0.25;

    // Check 20 timestamps at most.If one quarter is 0, it is illegal
    const int maxCheckTimeStamp = (depthTimeStamps.size() > 20) ? 20 : depthTimeStamps.size();
    for (int i = 0; i < maxCheckTimeStamp; i++)
    {
        int timeStamp = depthTimeStamps.at(i);
        if (timeStamp == 0)
        {
            zeroCount++;
        }
    }

    //If one quarter is 0, the timestamps are illegal
    isTimeStampsValid = (zeroCount * 1.0 / maxCheckTimeStamp) < oneQuarter;

    return isTimeStampsValid;
}

QVector<int> CapturedZipParser::getDataTypes()
{
    return dataTypes;
}

int CapturedZipParser::getFrameCount()
{
    return capturedFrameCount;
}

bool CapturedZipParser::isRawFormat()
{
    return dataFormat == "raw";
}

QString CapturedZipParser::getCaptureName()
{
    return captureName;
}

bool CapturedZipParser::getIsTimeStampsValid()
{
    return isTimeStampsValid;
}

QString CapturedZipParser::getFileName(int frameIndex, int dataType)
{
    QString fileName;
    QString suffix = isRawFormat() ? ".raw" : ".png";
    switch (dataType)
    {
    case CAMERA_DATA_L:
        fileName = QString("%1-ir-L-%2").arg(captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_R:
        fileName = QString("%1-ir-R-%2").arg(captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_DEPTH:
        fileName = QString("%1-depth-%2").arg(captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_RGB:
        fileName = QString("%1-RGB-%2").arg(captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_POINT_CLOUD:
        fileName = QString("%1-%2.ply").arg(captureName).arg(frameIndex, 4, 10, QChar('0'));
        suffix = "";
        break;
    default:
        break;
    }
    fileName += suffix;

    return fileName;
}

QString CapturedZipParser::getFileName(int dataType, QString name)
{
    QString fileName;
    QString suffix = isRawFormat() ? ".raw" : ".png";
    switch (dataType)
    {
    case CAMERA_DATA_L:
        fileName = QString("%1-ir-L").arg(name);
        break;
    case CAMERA_DATA_R:
        fileName = QString("%1-ir-R").arg(name);
        break;
    case CAMERA_DATA_DEPTH:
        fileName = QString("%1-depth").arg(name);
        break;
    case CAMERA_DATA_RGB:
        fileName = QString("%1-RGB").arg(name);
        break;
    case CAMERA_DATA_POINT_CLOUD:
        fileName = QString("%1.ply").arg(name);
        suffix = "";
        break;
    default:
        break;
    }
    fileName += suffix;

    return fileName;
}

bool CapturedZipParser::saveFrameToLocal(int frameIndex, bool withTexture, QString filePath)
{
    QFileInfo info(filePath);

    QString dir = info.absolutePath();
    QString fileName = info.fileName();

    bool result = true;

    auto newDataTypes = dataTypes;
    if (!newDataTypes.contains(CAMERA_DATA_POINT_CLOUD) && newDataTypes.contains(CAMERA_DATA_DEPTH))
    {
        newDataTypes.push_back(CAMERA_DATA_POINT_CLOUD);
    }

    for (auto type : newDataTypes)
    {
        CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)type;
        QString newfileName = getFileName(dataType, fileName);
        
        QString newFilePath = dir + "/" + newfileName;
        switch (dataType)
        {
        case CAMERA_DATA_L:
        case CAMERA_DATA_R:
        case CAMERA_DATA_DEPTH:
        case CAMERA_DATA_RGB:
            result &= saveImageData(frameIndex, dataType, newFilePath);
            break;
        case CAMERA_DATA_POINT_CLOUD:
            result &= savePointCloud(frameIndex, withTexture, newFilePath);
            break;
        default:
            break;
        }
    }

    return result;
}

bool CapturedZipParser::saveImageData(int frameIndex, int dataType, QString filePath)
{
    QByteArray data = getFrameData(frameIndex, dataType);
    if (data.isEmpty())
    {
        return false;
    }
    if (isRawFormat())
    {
        if (dataType == CAMERA_DATA_DEPTH)
        {
            return ImageUtil::saveGrayScale16ByLibpng(depthResolution.width(), depthResolution.height(), data, filePath);
        }
        else if (dataType == CAMERA_DATA_RGB)
        {
            QImage image = QImage((uchar*)data.data(), rgbResolution.width(), rgbResolution.height(), QImage::Format_RGB888);
            return image.save(filePath, "PNG");
        }
        else 
        {
            QImage image = QImage((uchar*)data.data(), depthResolution.width(), depthResolution.height(), QImage::Format_Grayscale8);
            return image.save(filePath, "PNG");
        }
    }
    else 
    {
        QFile file(filePath);
        file.open(QFile::WriteOnly);
        if (!file.isOpen())
        {
            qWarning() << "Open file failed, file:" << filePath;
            return false;
        }

        file.write(data);
        file.close();
    }
}

bool CapturedZipParser::savePointCloud(int frameIndex, bool withTexture, QString filePath)
{
    Pointcloud pc;
    QImage texImage;
    int rgbFrame = frameIndex;
    
    //If the timestamp is valid, find the RGB frame index through the timestamp
    if (withTexture && getIsTimeStampsValid())
    {
        rgbFrame = getRgbFrameIndexByTimeStamp(frameIndex);
    }

    if (!generatePointCloud(frameIndex, rgbFrame, withTexture, pc, texImage))
    {
        return false;
    }

    if (withTexture && !texImage.isNull())
    {
        pc.exportToFile(filePath.toStdString(), texImage.bits(), texImage.width(), texImage.height());
    }
    else
    {
        pc.exportToFile(filePath.toStdString(), nullptr, 0, 0);
    }

    return true;
}

// Find Rules: Compare RGB and depth timestamp to find the nearest one
int CapturedZipParser::getRgbFrameIndexByTimeStamp(int depthIndex)
{
    const int depthTimeStamp = getTimeStampOfFrame(depthIndex, CAMERA_DATA_DEPTH);
    
    return getNearestRgbFrame(depthIndex, depthTimeStamp);
}

int CapturedZipParser::getNearestRgbFrame(int depthIndex, int timeStamp)
{
    int start = 0;
    int end = rgbTimeStamps.size() - 1;
    int mid;

    if (depthIndex < start || depthIndex > end)
    {
        mid = std::floor((start + end) * 1.0f / 2);   
    }
    else 
    {
        mid = depthIndex;
    }

    // Differential dichotomy search
    do 
    {
        // in left
        if (timeStamp < rgbTimeStamps.at(mid))
        {
            end = mid;
        }
        else 
        {
            start = mid;
        }
        
        mid = std::floor((start + end) * 1.0f / 2);

    } while (end - start > 1);

    return (std::abs(timeStamp - rgbTimeStamps.at(start)) <= std::abs(timeStamp - rgbTimeStamps.at(end))) ? start  : end;
}

int CapturedZipParser::getTimeStampOfFrame(int frameIndex, int dataType)
{
    if (dataType != CAMERA_DATA_DEPTH && dataType != CAMERA_DATA_RGB)
    {
        return 0;
    }

    if (dataType == CAMERA_DATA_DEPTH)
    {
        if (frameIndex < depthTimeStamps.size() && frameIndex >= 0)
        {
            return depthTimeStamps.at(frameIndex);
        }
    }
    else 
    {
        if (frameIndex < rgbTimeStamps.size() && frameIndex >= 0)
        {
            return rgbTimeStamps.at(frameIndex);
        }
    }

    return 0;
}