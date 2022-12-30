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

#include "process/rgbprocessstrategy.h"

#include <QImage>

using namespace cs;

RgbProcessStrategy::RgbProcessStrategy()
    : ProcessStrategy(STRATEGY_RGB)
{

}

void RgbProcessStrategy::doProcess(const FrameData& frameData, OutputDataPort& outputDataPort)
{
    const auto& streamDatas = frameData.data;

    for (const auto& data : streamDatas)
    {
        STREAM_FORMAT format = data.dataInfo.format;
        OutputData2D outputData;

        switch (format)
        {
        case STREAM_FORMAT_RGB8:
            outputData = onProcessRGB8(data);
            break;
        case STREAM_FORMAT_MJPG:
            outputData = onProcessMJPG(data);
            break;
        default:
            break;
        }

        if (outputData.info.cameraDataType != CAMERA_DATA_UNKNOW)
        {
            outputDataPort.addOutputData2D(outputData);
        }
    }
}

OutputData2D RgbProcessStrategy::onProcessRGB8(const StreamData& streamData)
{
    const int width = streamData.dataInfo.width;
    const int height = streamData.dataInfo.height;
    const int dataSize = streamData.data.size();

    QImage image((uchar*)streamData.data.data(), width, height, QImage::Format_RGB888);
    OutputData2D outputData;
    outputData.info.cameraDataType = CAMERA_DATA_RGB;
    outputData.image = image.copy(image.rect());
    emit output2DUpdated(outputData);

    return outputData;
}

OutputData2D RgbProcessStrategy::onProcessMJPG(const StreamData& streamData)
{
    QImage image;
    image.loadFromData(streamData.data, "JPG");
    //image = image.convertToFormat(QImage::Format_RGB888);

    OutputData2D outputData;
    outputData.image = image;
    outputData.info.cameraDataType = CAMERA_DATA_RGB;
    emit output2DUpdated(outputData);

    return outputData;
}