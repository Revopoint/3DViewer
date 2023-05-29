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

#ifndef _CS_CAMERAPROXY_H
#define _CS_CAMERAPROXY_H

#include "icscamera.h"
#include "cscameraapi.h"
#include "cameraparaid.h"

#include <QReadWriteLock>
#include <QPointF>

namespace cs {

class CS_CAMERA_EXPORT CameraProxy : public ICSCamera
{
    Q_OBJECT
public:
    CameraProxy();
    ~CameraProxy();

    void getCameraPara(parameter::CAMERA_PARA_ID paraId, QVariant& value) override;
    void setCameraPara(parameter::CAMERA_PARA_ID paraId, QVariant value) override;
    void getCameraParaRange(parameter::CAMERA_PARA_ID paraId, QVariant& min, QVariant& max, QVariant& step) override;
    void getCameraParaItems(parameter::CAMERA_PARA_ID paraId, QList<QPair<QString, QVariant>>& list) override;

    bool disconnectCamera() override;
    bool reconnectCamera() override;

    bool startStream() override;
    bool stopStream() override;
    bool restartStream()override;
    bool restartCamera() override;
    bool pauseStream() override;
    bool resumeStream() override;
    bool softTrigger() override;

    void bindCamera(ICSCamera* camera);
    void unBindCamera();
    CSCameraInfo getCameraInfo() const override;
    int getCameraState() const override;

private:
    ICSCamera* m_csCamera = nullptr;
    mutable QReadWriteLock m_lock;
};
}

#endif // _CS_CAMERAPROXY_H
