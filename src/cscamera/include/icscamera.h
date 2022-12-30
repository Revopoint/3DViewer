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

#ifndef _CS_ICSCAMERA_H
#define _CS_ICSCAMERA_H
#include "cscameraapi.h"
#include "cstypes.h"
#include "cameraparaid.h"

#include <QObject>
#include <QList>
#include <QPair>
#include <QSize>
#include <QVariant>

namespace cs {

using namespace parameter;

class CS_CAMERA_EXPORT ICSCamera : public QObject
{
    Q_OBJECT
public:
    ~ICSCamera();
    virtual bool startStream() = 0;
    virtual bool stopStream() = 0;
    virtual bool restartStream() = 0;
    virtual bool restartCamera() = 0;
    virtual bool disconnectCamera() = 0;
    virtual bool reconnectCamera() = 0;
    virtual bool pauseStream() = 0;
    virtual bool resumeStream() = 0;
    virtual bool softTrigger() = 0;
    virtual CSCameraInfo getCameraInfo() const = 0;
    virtual int getCameraState() const = 0;
    
    virtual void getCameraPara(CAMERA_PARA_ID paraId, QVariant& value) = 0;
    virtual void setCameraPara(CAMERA_PARA_ID paraId, QVariant value) = 0;
    virtual void getCameraParaRange(CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step) = 0;
    virtual void getCameraParaItems(CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list) = 0;
signals:
    void cameraStateChanged(int state);
    void framedDataUpdated(FrameData frameData);

    void cameraParaUpdated(int, QVariant);
    void cameraParaRangeUpdated(int);
    void cameraParaItemsUpdated(int);
};

}
#endif // _CS_ICSCAMERA_H