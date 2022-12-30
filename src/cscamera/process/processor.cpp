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

#include "process/processor.h"
#include "process/processstrategy.h"

#include <QDebug>
#include <QMutexLocker>
using namespace cs;

Processor::Processor()
{
}

Processor::~Processor()
{
    processStrategys.clear();
    qDebug() << "~Processor";
}

void Processor::addProcessStrategy(ProcessStrategy* strategy)
{
    if (strategy == nullptr)
    {
        qWarning() << "strategy is null.";
        return;
    }

    QMutexLocker locker(&mutex);
    if (processStrategys.contains(strategy))
    {
        qWarning() << "Already contained strategy : " << strategy;
    }
    else 
    {
        processStrategys.push_back(strategy);
    }
}

void Processor::removeProcessStrategy(ProcessStrategy* strategy)
{
    if (strategy == nullptr)
    {
        qWarning() << "strategy is null.";
        return;
    }

    QMutexLocker locker(&mutex);
    if (!processStrategys.contains(strategy))
    {
        qWarning() << "processStrategys do not contain strategy type: " << strategy->getProcessStraType();
    }
    else 
    {
        processStrategys.removeAll(strategy);
    }
}

void Processor::addProcessEndLisener(ProcessEndListener* listener)
{
    if (listener == nullptr)
    {
        qWarning() << "listener is null.";
        return;
    }

    QMutexLocker locker(&mutex);
    if (processEndLiseners.contains(listener))
    {
        qWarning() << "Already contained listener : " << listener;
    }
    else
    {
        processEndLiseners.push_back(listener);
    }
}

void Processor::removeProcessEndLisener(ProcessEndListener* listener)
{
    if (listener == nullptr)
    {
        qWarning() << "strategy is null.";
        return;
    }

    QMutexLocker locker(&mutex);
    if (!processEndLiseners.contains(listener))
    {
        qWarning() << "processEndLiseners do not contain listener: " << listener;
    }
    else
    {
        processEndLiseners.removeAll(listener);
    }
}

void Processor::process(const FrameData& frameData)
{
    QMutexLocker locker(&mutex);

    OutputDataPort outputDataPort(frameData);
    for (auto stra : processStrategys)
    {
        if (stra->isStrategyEnable())
        {
            stra->process(frameData, outputDataPort);
        }
    }

    // to do some thing after process, for example save data
    for (auto lisener : processEndLiseners)
    {
        lisener->process(outputDataPort);
    }
}