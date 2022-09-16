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

#include "cameralistwidget.h"

#include <QFile>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QImage>
#include <QDebug>
#include <QThread>

#include "./ui_cameralist.h"
#include "csapplication.h"
#include "icscamera.h"
#include "cstypes.h"

CameraListWidget::CameraListWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CameraListWidget)
{
    ui->setupUi(this);
    iniWidget();
    initConnections();
}

CameraListWidget::~CameraListWidget()
{
    delete ui;
}

void CameraListWidget::onClickedConnectButton()
{
    auto curItem = ui->cameraListWidget->currentItem();
    if (curItem)
    {
        auto serial = curItem->text();
        emit connectCamera(serial);
    }
}

void CameraListWidget::onDoubleClickedCameraListView(QListWidgetItem* item)
{
    onClickedConnectButton();
}

void CameraListWidget::onCameraListIndexChanged(int index)
{
    ui->connectCameraBtn->setEnabled(index >= 0);
}

bool CameraListWidget::isNetConnect(QString info)
{
    //E.g : xxxxxxxxxx(xxx.xxx.xxx.xxx)
    return info.contains(".");
}

void CameraListWidget::onCameraListUpdated(const QStringList infoList)
{
    const int size = infoList.size();
    auto curCameraSerial = QString(cs::CSApplication::getInstance()->getCamera()->getCameraInfo().cameraInfo.serial);

    ui->cameraListWidget->clear();
    int curSelect = -1;
    for (int i = 0; i < size; i++)
    {
        const QString& info = infoList.at(i);  
        QIcon icon = isNetConnect(info) ? QIcon(":/resources/internet.png") : QIcon(":/resources/usb.png");

        QListWidgetItem* item = new QListWidgetItem(icon, info);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QColor color = (i % 2 == 1) ? QColor(229, 229, 229) : QColor(213, 213, 213);
        item->setBackground(color);
        item->setSelected(true);

        ui->cameraListWidget->addItem(item);

        if (!curCameraSerial.isEmpty() && info.contains(curCameraSerial))
        {
            curSelect = i;
        }
    }

    const int modelCount = ui->cameraListWidget->count();
    if (modelCount > 0 && curSelect < 0)
    {
        ui->cameraListWidget->setCurrentRow(0);
    }
    else 
    {
        ui->cameraListWidget->setCurrentRow(curSelect);
    }
}

void CameraListWidget::onCameraStateChanged(int state)
{
    CAMERA_STATE cameraState = (CAMERA_STATE)state;
    switch (cameraState)
    {
    case CAMERA_DISCONNECTED:
    case CAMERA_CONNECTFAILED:
    case CAMERA_DISCONNECTFAILED:
        break;
    default:
        break;
    }
}

void CameraListWidget::initConnections()
{
    auto app = cs::CSApplication::getInstance();

    bool suc = true;
    suc &= (bool)connect(ui->connectCameraBtn, &QPushButton::clicked, this, &CameraListWidget::onClickedConnectButton);
    suc &= (bool)connect(app, &cs::CSApplication::cameraListUpdated, this, &CameraListWidget::onCameraListUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::cameraStateChanged, this, &CameraListWidget::onCameraStateChanged);

    suc &= (bool)connect(ui->cameraListWidget, &QListWidget::itemDoubleClicked, this, &CameraListWidget::onDoubleClickedCameraListView);
    suc &= (bool)connect(ui->cameraListWidget, &QListWidget::currentRowChanged, this, &CameraListWidget::onCameraListIndexChanged);

    suc &= (bool)connect(this, &CameraListWidget::connectCamera,   app, &cs::CSApplication::connectCamera);
    suc &= (bool)connect(this, &CameraListWidget::translateSignal, this, &CameraListWidget::onTranslate);
    suc &= (bool)connect(ui->closeButton, &QPushButton::clicked,   this, &CameraListWidget::clickedCloseButton);

    Q_ASSERT(suc);
}

void CameraListWidget::iniWidget()
{
    ui->connectCameraBtn->setProperty("isCSStyle", true);
    ui->cameraListIcon->setPixmap(QPixmap::fromImage(QImage(":/resources/camera_list.png")));
    ui->cameraListWidget->setFocusPolicy(Qt::NoFocus);
    ui->connectCameraBtn->setEnabled(false);
}

void CameraListWidget::onTranslate()
{
    ui->retranslateUi(this);
}
