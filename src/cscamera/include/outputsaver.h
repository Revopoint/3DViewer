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

#ifndef _CS_OUTPUT_SAVER_H
#define _CS_OUTPUT_SAVER_H

#include <QRunnable>

#include "cstypes.h"
#include "process/outputdataport.h"

namespace cs {

class CameraCaptureBase;
class OutputSaver : public QRunnable
{
public:
    OutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output);
    virtual ~OutputSaver();
    void run() override;

    void updateCaptureConfig(const CameraCaptureConfig& config);
    void updateSaveIndex();
protected:
    void setSaveIndex(int rgbFrameIndex, int depthFrameIndex, int pointCloudIndex);
    void savePointCloud();

    void saveOutput2D();
    void saveOutput2D(StreamData& streamData);

    virtual void saveOutputRGB(StreamData& streamData) {}
    virtual void saveOutputDepth(StreamData& streamData) {}
    virtual void saveOutputIr(StreamData& streamData) {}

    void savePointCloud(cs::Pointcloud& pointCloud, QImage& texImage);

    QString getSavePath(CS_CAMERA_DATA_TYPE dataType);

protected:
    CameraCaptureBase* m_cameraCapture = nullptr;
    CameraCaptureConfig m_captureConfig;
    OutputDataPort m_outputDataPort;
    QString m_suffix2D;

    int m_rgbFrameIndex = -1;
    int m_depthFrameIndex = -1;
    int m_pointCloudIndex = -1;
};

class ImageOutputSaver : public OutputSaver
{
public:
    ImageOutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output);
    void saveOutputRGB(StreamData& streamData) override;
    void saveOutputDepth(StreamData& streamData) override;
    void saveOutputIr(StreamData& streamData) override;
private:
    void saveGrayScale16(StreamData& streamData, QString path);
};

class RawOutputSaver : public OutputSaver
{
public:
    RawOutputSaver(CameraCaptureBase* cameraCapture, const CameraCaptureConfig& config, const OutputDataPort& output);
    void saveOutputRGB(StreamData& streamData) override;
    void saveOutputDepth(StreamData& streamData) override;
    void saveOutputIr(StreamData& streamData) override;
private:
    void saveDataToFile(QString filePath, QByteArray data);
};

}

#endif // _CS_OUTPUT_SAVER_H