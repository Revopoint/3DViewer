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
    , m_cameraInfoUi(new Ui::CameraInfoWidget)
{
    m_cameraInfoUi->setupUi(this);
    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint);
    setModal(true);
}

CameraInfoDialog::~CameraInfoDialog()
{
    delete m_cameraInfoUi;
}

void CameraInfoDialog::updateCameraInfo(const CSCameraInfo& info)
{
    m_cameraInfoUi->modelLabel->setText(info.model);
    m_cameraInfoUi->snLabel->setText(info.cameraInfo.serial);
    m_cameraInfoUi->sdkVerLabel->setText(info.sdkVersion);
    m_cameraInfoUi->firmwareVerLabel->setText(info.cameraInfo.firmwareVersion);
    m_cameraInfoUi->algoVerLabel->setText(info.cameraInfo.algorithmVersion);

    CSConnectType connectMode = (CSConnectType)info.connectType;
    if (connectMode == CONNECT_TYPE_NET)
    {
        m_cameraInfoUi->connectionMode->setText("Network");
    }
    else
    {
        m_cameraInfoUi->connectionMode->setText("USB");
    }
}

void CameraInfoDialog::onTranslate()
{
    m_cameraInfoUi->retranslateUi(this);
}