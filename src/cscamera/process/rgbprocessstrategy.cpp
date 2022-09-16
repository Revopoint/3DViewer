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

#include "process/rgbprocessstrategy.h"

#include <QImage>

using namespace cs;

void RgbProcessStrategy::doProcess(const FrameData& frameData)
{
    const auto& streamDatas = frameData.data;

    for (const auto& data : streamDatas)
    {
        STREAM_FORMAT format = data.dataInfo.format;
        switch (format)
        {
        case STREAM_FORMAT_RGB8:
            onProcessRGB8(data);
            break;
        case STREAM_FORMAT_MJPG:
            onProcessMJPG(data);
            break;
        default:
            break;
        }
    }
}

void RgbProcessStrategy::onProcessRGB8(const StreamData& streamData)
{
    const int width = streamData.dataInfo.width;
    const int height = streamData.dataInfo.height;
    const int dataSize = streamData.data.size();

    QImage image((uchar*)streamData.data.data(), width, height, QImage::Format_RGB888);
    OutputData2D outputData;
    outputData.info.cameraDataType = CAMERA_DATA_RGB;
    outputData.image = image.copy(image.rect());
    emit output2DUpdated(outputData, streamData);
}

void RgbProcessStrategy::onProcessMJPG(const StreamData& streamData)
{
    QImage image;
    image.loadFromData(streamData.data, "JPG");
    //image = image.convertToFormat(QImage::Format_RGB888);

    OutputData2D outputData;
    outputData.image = image;
    outputData.info.cameraDataType = CAMERA_DATA_RGB;
    emit output2DUpdated(outputData, streamData);
}