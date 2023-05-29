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

#include "capturedzipparser.h"
#include <imageutil.h>
#include <JlCompress.h>
#include <yaml-cpp/yaml.h>
#include <QDebug>
#include <QMap>
#include <math.h>
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
    : m_zipFile(filePath)
{

}

CapturedZipParser::~CapturedZipParser()
{

}

void CapturedZipParser::setZipFile(QString filePath)
{
    m_zipFile = filePath;
}

bool CapturedZipParser::checkFileValid()
{
    qInfo() << "check zip file is valid or not";

    QuaZip zip(m_zipFile);
    zip.open(QuaZip::mdUnzip);
    if (!zip.isOpen())
    {
        qWarning() << "Open zip file failed, file:" << m_zipFile;
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

        QuaZip zip(m_zipFile);
        result = zip.open(QuaZip::mdUnzip);
        if (!result)
        {
            qWarning() << "open zip file failed, file:" << m_zipFile;
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

// generate PoinCloud from .ply file
bool CapturedZipParser::getPointCloud(int frameIndex, Pointcloud& pc, QImage& texImage)
{
    bool result = true;
    do 
    {
        QuaZip zip(m_zipFile);
        result = zip.open(QuaZip::mdUnzip);
        if (!result)
        {
            qWarning() << "open zip file failed, file:" << m_zipFile;
            break;
        }

        QString fileName = getFileName(frameIndex, CAMERA_DATA_POINT_CLOUD);
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

        QTextStream ts(&file);
        int vertexCount = 0;
        bool hasTexture = false;
        bool hasNormal = false;
        // read header
        while (!ts.atEnd())
        {
            QString line = ts.readLine();
            if (line.startsWith("element vertex"))
            {
                auto arr = line.split(" ");
                if (arr.size() >= 3)
                {
                    vertexCount = arr[2].toInt();
                }
                continue;
            }

            // read header end
            if (line.startsWith("end_header"))
            {
                break;
            }

            // check has texture info or not
            if (line.contains("red"))
            {
                hasTexture = true;
            }

            if (line.contains("nx"))
            {
                hasNormal = true;
            }
        }

        std::vector<float3>& points = pc.getVertices();
        std::vector<float2>& textures = pc.getTexcoords();
        std::vector<float3>& normals = pc.getNormals();

        float3 p(0, 0, 0);
        float3 n(0, 0, 0);
        int3 rgb(0, 0, 0);
        int texWidth = 0, texHeight = 0;

        QByteArray textureData;
        if (hasTexture)
        {
            texWidth = m_depthResolution.width();
            texHeight = vertexCount / texWidth + ((vertexCount % texWidth) == 0 ? 0 : 1);
            textureData.resize(texWidth * texHeight * 3); // RGB888
        }

        points.clear();
        textures.clear();
        normals.clear();

        int vIndex = 0;
        uchar* texPtr = (uchar*)textureData.data();

        int w = 0, h = 0;
        // read point data
        while (!ts.atEnd() && vIndex < vertexCount)
        {
            ts >> p.x >> p.y >> p.z;
            points.push_back(p);

            if (hasNormal)
            {
                ts >> n.x >> n.y >> n.z;
                normals.push_back(n);
            }

            if (hasTexture)
            {
                ts >> rgb.x >> rgb.y >> rgb.z;
                float2 texPos(0, 0);

                texPtr[3 * vIndex] = rgb.x;
                texPtr[3 * vIndex + 1] = rgb.y;
                texPtr[3 * vIndex + 2] = rgb.z;

                texPos.u = w * 1.0f / texWidth;
                texPos.v = h * 1.0f / texHeight;
                textures.push_back(texPos);

                w++;
                if (w == texWidth)
                {
                    h++;
                    w = 0;
                }
            }

            vIndex++;
        }

        if (hasTexture)
        {
            QImage image = QImage(texPtr, texWidth, texHeight, QImage::Format_RGB888);
            texImage = image.copy(image.rect());
        }

        zip.close();
        file.close();
    } while (false);


    return result;
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

    int width = m_depthResolution.width();
    int height = m_depthResolution.height();

    if (withTexture)
    {
        tex = getImageOfFrame(rgbIndex, CAMERA_DATA_RGB);
        pc.generatePoints<ushort>((ushort*)pixData.data(), width, height, m_depthScale, &m_depthIntrinsics, &m_rgbIntrinsics, &m_extrinsics, true);
    }
    else
    {
        pc.generatePoints<ushort>((ushort*)pixData.data(), width, height, m_depthScale, &m_depthIntrinsics, nullptr, nullptr, true);
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

    int width = (dataType == CAMERA_DATA_RGB) ? m_rgbResolution.width() : m_depthResolution.width();
    int height = (dataType == CAMERA_DATA_RGB) ? m_rgbResolution.height() : m_depthResolution.height();

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
    color.setRange(m_depthMin, m_depthMax);

    QImage image = QImage(width, height, QImage::Format_RGB888);
    color.process<ushort>((ushort*)data.data(), m_depthScale, image.bits(), width * height);
    return image;
}

bool CapturedZipParser::parseCaptureInfo()
{
    qInfo() << "Parse zip file";

    bool result = true;

    QuaZip zip(m_zipFile);
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
        m_captureName = node["Name"].as<std::string>().c_str();

        // frame number
        m_capturedFrameCount = node["Frame Number"].as<int>();

        // data types
        YAML::Node data = node["Data Types"].as<YAML::Node>();
        m_dataTypes.clear();
        for (int i = 0; i < data.size(); i++)
        {
            std::string s = data[i].as<std::string>();

            QString type = s.c_str();
            Q_ASSERT(dataTypeMap.contains(type));
            m_dataTypes.push_back(dataTypeMap[type]);
        }

        // data format
        m_dataFormat = node["Save Format"].as<std::string>().c_str();

        // with texture or not
        YAML::Node nodeTmp = node["With Texture"];
        if (nodeTmp.IsDefined())
        {
            m_enableTexture = nodeTmp.as<bool>();
        }
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

            m_depthResolution.setWidth(width);
            m_depthResolution.setHeight(height);
        }

        // RGB resolution
        nodeTmp = node["RGB resolution"];
        if (nodeTmp.IsDefined())
        {
            int width = nodeTmp["width"].as<int>();
            int height = nodeTmp["height"].as<int>();;

            m_rgbResolution.setWidth(width);
            m_rgbResolution.setHeight(height);
        }

        // depth intrinsics
        nodeTmp = node["Depth intrinsics"];
        if (nodeTmp.IsDefined())
        {
            m_depthIntrinsics = genIntrinsicsFromNode(nodeTmp);
        }

        // rgb intrinsics
        nodeTmp = node["RGB intrinsics"];
        if (nodeTmp.IsDefined())
        {
            m_rgbIntrinsics = genIntrinsicsFromNode(nodeTmp);
        }

        // extrinsics
        nodeTmp = node["Extrinsics"];
        if (nodeTmp.IsDefined())
        {
            m_extrinsics = genExtrinsicsFromNode(nodeTmp);
        }

        // depth scale
        m_depthScale = node["Depth Scale"].as<float>();
        m_depthMin = node["Depth Min"].as<float>();
        m_depthMax = node["Depth Max"].as<float>();

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
        QuaZip zip(m_zipFile);
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
        m_depthTimeStamps.clear();
        while (!ss.atEnd() && i < m_capturedFrameCount)
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

            if (line.isEmpty())
            {
                break;
            }

            // get time stamp
            auto arr = line.split("=");
            if (arr.size() >= 2)
            {
                int timeStamp = arr.last().toInt();
                m_depthTimeStamps.push_back(timeStamp);
            }

            i++;
        }

        // rgb time stamps
        m_rgbTimeStamps.clear();
        findHeader = false;
        i = 0;
        while (!ss.atEnd() && i < m_capturedFrameCount)
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
                m_rgbTimeStamps.push_back(timeStamp);
            }

            i++;
        }
         
    } while (false);


    // check the time stamps in zip file 
    if (checkTimeStampsValid())
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
    const int maxCheckTimeStamp = (m_depthTimeStamps.size() > 20) ? 20 : m_depthTimeStamps.size();
    for (int i = 0; i < maxCheckTimeStamp; i++)
    {
        int timeStamp = m_depthTimeStamps.at(i);
        if (timeStamp == 0)
        {
            zeroCount++;
        }
    }

    //If one quarter is 0, the timestamps are illegal
    m_isTimeStampsValid = (zeroCount * 1.0 / maxCheckTimeStamp) < oneQuarter;

    return m_isTimeStampsValid;
}

QVector<int> CapturedZipParser::getDataTypes()
{
    return m_dataTypes;
}

int CapturedZipParser::getFrameCount()
{
    return m_capturedFrameCount;
}

bool CapturedZipParser::isRawFormat()
{
    return m_dataFormat == "raw";
}

QString CapturedZipParser::getCaptureName()
{
    return m_captureName;
}

bool CapturedZipParser::getIsTimeStampsValid()
{
    return m_isTimeStampsValid;
}

QString CapturedZipParser::getFileName(int frameIndex, int dataType)
{
    QString fileName;
    QString suffix = isRawFormat() ? ".raw" : ".png";
    switch (dataType)
    {
    case CAMERA_DATA_L:
        fileName = QString("%1-ir-L-%2").arg(m_captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_R:
        fileName = QString("%1-ir-R-%2").arg(m_captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_DEPTH:
        fileName = QString("%1-depth-%2").arg(m_captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_RGB:
        fileName = QString("%1-RGB-%2").arg(m_captureName).arg(frameIndex, 4, 10, QChar('0'));
        break;
    case CAMERA_DATA_POINT_CLOUD:
        fileName = QString("%1-%2.ply").arg(m_captureName).arg(frameIndex, 4, 10, QChar('0'));
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

    auto newDataTypes = m_dataTypes;
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
            return ImageUtil::saveGrayScale16ByLibpng(m_depthResolution.width(), m_depthResolution.height(), data, filePath);
        }
        else if (dataType == CAMERA_DATA_RGB)
        {
            QImage image = QImage((uchar*)data.data(), m_rgbResolution.width(), m_rgbResolution.height(), QImage::Format_RGB888);
            return image.save(filePath, "PNG");
        }
        else 
        {
            QImage image = QImage((uchar*)data.data(), m_depthResolution.width(), m_depthResolution.height(), QImage::Format_Grayscale8);
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

    QByteArray pathData = filePath.toLocal8Bit();
    std::string realPath = pathData.data();

    if (withTexture && !texImage.isNull())
    {
        pc.exportToFile(realPath, texImage.bits(), texImage.width(), texImage.height());
    }
    else
    {
        pc.exportToFile(realPath, nullptr, 0, 0);
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
    if (m_rgbTimeStamps.size() <= 0)
    {
        return depthIndex;
    }

    int start = 0;
    int end = m_rgbTimeStamps.size() - 1;
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
        if (timeStamp < m_rgbTimeStamps.at(mid))
        {
            end = mid;
        }
        else 
        {
            start = mid;
        }
        
        mid = std::floor((start + end) * 1.0f / 2);

    } while (end - start > 1);

    return (std::abs(timeStamp - m_rgbTimeStamps.at(start)) <= std::abs(timeStamp - m_rgbTimeStamps.at(end))) ? start  : end;
}

int CapturedZipParser::getTimeStampOfFrame(int frameIndex, int dataType)
{
    if (dataType != CAMERA_DATA_DEPTH && dataType != CAMERA_DATA_RGB)
    {
        return 0;
    }

    if (dataType == CAMERA_DATA_DEPTH)
    {
        if (frameIndex < m_depthTimeStamps.size() && frameIndex >= 0)
        {
            return m_depthTimeStamps.at(frameIndex);
        }
    }
    else 
    {
        if (frameIndex < m_rgbTimeStamps.size() && frameIndex >= 0)
        {
            return m_rgbTimeStamps.at(frameIndex);
        }
    }

    return 0;
}

bool CapturedZipParser::enablePointCloudTexture()
{
    return m_enableTexture || (m_dataTypes.contains(CAMERA_DATA_RGB) && m_dataTypes.contains(CAMERA_DATA_DEPTH));
}
