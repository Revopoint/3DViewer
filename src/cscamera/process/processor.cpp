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

#include "process/processor.h"
#include "process/depthprocessstrategy.h"
#include "process/rgbprocessstrategy.h"
#include "process/pointcloudprocessstrategy.h"

#include <QDebug>
using namespace cs;

Processor::Processor()
    : calcPointCloud(false)
{
    initialize();
}

Processor::~Processor()
{
    for (auto strategy : processStrategyMap.values())
    {
        delete strategy;
    }

    processStrategyMap.clear();

    qDebug() << "~Processor";
}

void Processor::process(const FrameData& frameData)
{
    FRAME_DATA_TYPE dataType = frameData.frameDataType;

    switch (dataType)
    {
    case TYPE_DEPTH:
        onProcessDepthFrame(frameData);
        break;
    case TYPE_RGB:
        processStrategyMap[STRATEGY_RGB]->process(frameData);
        break;
    case TYPE_DEPTH_RGB: 
        onProcessPairFrame(frameData);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

void Processor::onProcessPairFrame(const FrameData& frameData)
{
    if (calcPointCloud)
    {
        processStrategyMap[STRATEGY_CLOUD_POINT]->process(frameData);
    }
    else 
    {
        processStrategyMap[STRATEGY_DEPTH]->process(frameData);
        processStrategyMap[STRATEGY_RGB]->process(frameData);
    }
}

void Processor::onProcessDepthFrame(const FrameData& frameData)
{
    if (calcPointCloud)
    {
        processStrategyMap[STRATEGY_CLOUD_POINT]->process(frameData);
    }
    else 
    {
        processStrategyMap[STRATEGY_DEPTH]->process(frameData);
    }
}

void Processor::initialize()
{
    Q_ASSERT(processStrategyMap.size() == 0);
    processStrategyMap.insert(STRATEGY_DEPTH, new DepthProcessStrategy());
    processStrategyMap.insert(STRATEGY_RGB, new RgbProcessStrategy());
    processStrategyMap.insert(STRATEGY_CLOUD_POINT, new PointCloudProcessStrategy());

    bool suc = true;

    for (auto strategy : processStrategyMap.values())
    {
        suc &= (bool)connect(strategy, &ProcessStrategy::output2DUpdated, this, &Processor::output2DUpdated);
        suc &= (bool)connect(strategy, &ProcessStrategy::output3DUpdated, this, &Processor::output3DUpdated);    
    }

    Q_ASSERT(suc);
}

void Processor::setCamera(std::shared_ptr<ICSCamera> camera)
{
    for (auto strategy : processStrategyMap.values())
    {
        strategy->setCamera(camera);
    }
}

void Processor::onShowDepthCoordChanged(bool show)
{
    if (processStrategyMap.contains(STRATEGY_DEPTH))
    {
        processStrategyMap[STRATEGY_DEPTH]->setProperty("calcDepthCoord", show);
    }
}

void Processor::onShowDepthCoordPosChanged(QPointF pos)
{
    if (processStrategyMap.contains(STRATEGY_DEPTH))
    {
        processStrategyMap[STRATEGY_DEPTH]->setProperty("depthCoordCalcPos", pos);
    }
}

void Processor::onShowRender3DChanged(bool show)
{
    calcPointCloud = show;
}

void Processor::onShow3DWithTextureChanged(bool withTexture)
{
    if (processStrategyMap.contains(STRATEGY_CLOUD_POINT))
    {
        processStrategyMap[STRATEGY_CLOUD_POINT]->setProperty("withTexture", withTexture);
    }
}

void Processor::onCameraParaUpdated(int paraId)
{
    for (auto strategy : processStrategyMap.values())
    {
        strategy->setCameraParaState(true);
    }
}