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
    m_processStrategys.clear();
    qDebug() << "~Processor";
}

void Processor::addProcessStrategy(ProcessStrategy* strategy)
{
    if (strategy == nullptr)
    {
        qWarning() << "strategy is null.";
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (m_processStrategys.contains(strategy))
    {
        qWarning() << "Already contained strategy : " << strategy;
    }
    else 
    {
        m_processStrategys.push_back(strategy);
    }
}

void Processor::removeProcessStrategy(ProcessStrategy* strategy)
{
    if (strategy == nullptr)
    {
        qWarning() << "strategy is null.";
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (!m_processStrategys.contains(strategy))
    {
        qWarning() << "processStrategys do not contain strategy type: " << strategy->getProcessStraType();
    }
    else 
    {
        m_processStrategys.removeAll(strategy);
    }
}

void Processor::addProcessEndLisener(ProcessEndListener* listener)
{
    if (listener == nullptr)
    {
        qWarning() << "listener is null.";
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (m_processEndLiseners.contains(listener))
    {
        qWarning() << "Already contained listener : " << listener;
    }
    else
    {
        m_processEndLiseners.push_back(listener);
    }
}

void Processor::removeProcessEndLisener(ProcessEndListener* listener)
{
    if (listener == nullptr)
    {
        qWarning() << "strategy is null.";
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (!m_processEndLiseners.contains(listener))
    {
        qWarning() << "processEndLiseners do not contain listener: " << listener;
    }
    else
    {
        m_processEndLiseners.removeAll(listener);
    }
}

void Processor::process(const FrameData& frameData)
{
    QMutexLocker locker(&m_mutex);

    OutputDataPort outputDataPort(frameData);
    for (auto stra : m_processStrategys)
    {
        if (stra->isStrategyEnable())
        {
            stra->process(frameData, outputDataPort);
        }
    }

    // to do some thing after process, for example save data
    for (auto lisener : m_processEndLiseners)
    {
        lisener->process(outputDataPort);
    }
}