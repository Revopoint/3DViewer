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

#include "camerainfodialog.h"
#include "./ui_camerainfo.h"

CameraInfoDialog::CameraInfoDialog(QWidget* parent)
    : QDialog(parent)
    , cameraInfoUi(new Ui::CameraInfoWidget)
{
    cameraInfoUi->setupUi(this);
    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint);
    setModal(true);
}

CameraInfoDialog::~CameraInfoDialog()
{
    delete cameraInfoUi;
}

void CameraInfoDialog::updateCameraInfo(const CSCameraInfo& info)
{
    cameraInfoUi->modelLabel->setText(info.model);
    cameraInfoUi->snLabel->setText(info.cameraInfo.serial);
    cameraInfoUi->sdkVerLabel->setText(info.sdkVersion);
    cameraInfoUi->firmwareVerLabel->setText(info.cameraInfo.firmwareVersion);
    cameraInfoUi->algoVerLabel->setText(info.cameraInfo.algorithmVersion);

    CSConnectType connectMode = (CSConnectType)info.connectType;
    if (connectMode == CONNECT_TYPE_NET)
    {
        cameraInfoUi->connectionMode->setText("Network");
    }
    else
    {
        cameraInfoUi->connectionMode->setText("USB");
    }
}

void CameraInfoDialog::onTranslate()
{
    cameraInfoUi->retranslateUi(this);
}