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

#ifndef _CS_CAMERA_PLAYER_H
#define _CS_CAMERA_PLAYER_H

#include <QThread>
#include <QString>
#include <QReadWriteLock> 

#include "cstypes.h"
#include "cscameraapi.h"
#include <hpp/Types.hpp>
#include <hpp/Processing.hpp>

namespace cs
{
class CapturedZipParser;
class CS_CAMERA_EXPORT CameraPlayer : public QThread
{
    Q_OBJECT
public:

    enum PLAYER_STATE
    {
        PLAYER_LOADING,
        PLAYER_READY,
        PLAYER_ERROR,
        PLAYER_SAVING,
        PLAYER_SAVE_SUCCESS,
        PLAYER_SAVE_FAILED
    };

    CameraPlayer(QObject* parent = nullptr);
    ~CameraPlayer();
    
    QVector<int> getDataTypes();
    int getFrameNumber();
    void currentDataTypesUpdated(QVector<int> dataTypes);
    void onShow3DTextureChanged(bool show);
public slots:
    void onLoadFile(QString file);
    void onPalyFrameUpdated(int curFrame, bool force = false);
    void onSaveCurrentFrame(QString filePath);
signals:
    void playerStateChanged(int state, QString msg);
    void output2DUpdated(OutputData2D outputData);
    void output3DUpdated(cs::Pointcloud pointCloud, const QImage& image);
private:
    void updateCurrentFrame();
    void updateCurrentImage(int type);
    void updateCurrentPointCloud();

private:

    CapturedZipParser* zipParser = nullptr;
    int currentFrame = 0;
    QVector<int> currentDataTypes;
    bool show3dTexture = false;

    QReadWriteLock lock;
};
}
#endif //_CS_CAMERA_PLAYER_H