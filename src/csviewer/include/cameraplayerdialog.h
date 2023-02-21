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

#ifndef _CS_CAMERA_PLAYER_DIALOG
#define _CS_CAMERA_PLAYER_DIALOG

#include <QDialog>
#include <QCheckBox>
#include <QMap>
#include <QImage>
#include <hpp/Processing.hpp>
#include <cstypes.h>

namespace Ui 
{
    class CameraPlayerWidget;
}

namespace cs 
{
    class CameraPlayer;
}

class CSProgressBar;
class CameraPlayerDialog : public QDialog
{
    Q_OBJECT
public:
    CameraPlayerDialog(QWidget* parent = nullptr);
    ~CameraPlayerDialog();
    void onTranslate();
public slots:
    void onPlayerStateChanged(int state, QString msg);
    void onLoadFile();
    void onRenderExit(int renderId);
    void onShowTextureUpdated(bool texture);
signals:
    void showMessage(QString msg, int time);
    void loadFile(QString file);
    void currentFrameUpdated(int curFrame, bool updateForce = false);
    void output2DUpdated(OutputData2D outputData);
    void output3DUpdated(cs::Pointcloud pointCloud, const QImage& image);
    void saveCurrentFrame(QString filePath);
private:
    void onPlayReady();
    void updateFrameRange(int frameNumer);
private slots:
    void onToggledCheckBox();
    void onSliderValueChanged();
    void onLineEditFinished();
    void onClickedSave();
private:
    Ui::CameraPlayerWidget* m_ui;
    cs::CameraPlayer* m_cameraPlayer;

    QMap<int, QCheckBox*> m_dataTypeCheckBoxs;
    CSProgressBar* m_circleProgressBar;
};

#endif  // _CS_CAMERA_PLAYER_DIALOG