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

#ifndef _CS_PROCESSTHREAD_H
#define _CS_PROCESSTHREAD_H

#include <QThread>
#include <QMutex>
#include <QQueue>
#include <memory>

#include "cstypes.h"
#include "cscameraapi.h"

namespace cs 
{
class Processor;
class CS_CAMERA_EXPORT ProcessThread : public QThread
{
    Q_OBJECT
public:
    ProcessThread(std::shared_ptr<Processor> processor);
    ~ProcessThread();
    void run() override;
public slots:
    void onFrameDataUpdated(FrameData frameData);
private:
    QMutex m_mutex;
    QQueue<FrameData> m_cachedFrameData;
    std::shared_ptr<Processor> m_processorPtr;
};
}

#endif //_CS_PROCESSTHREAD_H