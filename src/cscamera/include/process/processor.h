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

#ifndef _CS_PROCESSOR_H
#define _CS_PROCESSOR_H
#include <QObject>
#include <QVector>
#include <QMutex>
#include <memory>

#include "cscameraapi.h"
#include "cstypes.h"
#include "outputdataport.h"

namespace cs
{
class ProcessStrategy;

class CS_CAMERA_EXPORT Processor: public QObject
{
    Q_OBJECT
public:
    class ProcessEndListener : public QObject
    {
    public:
        virtual void process(const OutputDataPort& outputDataPort) = 0;
    };

    Processor();
    ~Processor();

    void process(const FrameData& frameData);

    void addProcessStrategy(ProcessStrategy* strategy);
    void removeProcessStrategy(ProcessStrategy* strategy);

    void addProcessEndLisener(ProcessEndListener* listener);
    void removeProcessEndLisener(ProcessEndListener* listener);
private:
    QVector<ProcessStrategy*> m_processStrategys; 
    QMutex m_mutex;
    QVector<ProcessEndListener*> m_processEndLiseners;
};
}

#endif //_CS_PROCESSOR_H