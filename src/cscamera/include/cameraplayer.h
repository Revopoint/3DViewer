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
    bool enablePointCloudTexture();
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

    CapturedZipParser* m_zipParser = nullptr;
    int m_currentFrame = 0;
    QVector<int> m_currentDataTypes;
    bool m_show3dTexture = true;

    QReadWriteLock m_lock;
};
}
#endif //_CS_CAMERA_PLAYER_H