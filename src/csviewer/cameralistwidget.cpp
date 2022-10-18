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

#include "csimagebutton.h"
#include "cslistitem.h"

CameraListWidget::CameraListWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CameraListWidget)
{
    setAttribute(Qt::WA_StyledBackground);
    ui->setupUi(this);
    iniWidget();
    initConnections();
}

CameraListWidget::~CameraListWidget()
{
    delete ui;
}

void CameraListWidget::onClickedCameraListItem(bool selected, QString serial, QListWidgetItem* listItem)
{
    if (selected)
    {
        // disconnect camera
        emit disconnectCamera();
    }
    else 
    {
        // connect to camera
        int row = ui->cameraListWidget->currentRow();
        if (row >= 0)
        {
            QListWidgetItem* item = ui->cameraListWidget->item(row);
            QWidget* itemWidget = ui->cameraListWidget->itemWidget(item);
            CSListItem* csItem = qobject_cast<CSListItem*>(itemWidget);
            if (csItem)
            {
                csItem->setSelected(false);
            }
        }
        int idx = ui->cameraListWidget->row(listItem);
        ui->cameraListWidget->setCurrentRow(idx);

        emit connectCamera(serial);
    }
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
        addListWidgetItem(info);

        if (!curCameraSerial.isEmpty() && info.contains(curCameraSerial))
        {
            curSelect = i;
        }
    }

    if (curSelect >= 0)
    {
        QListWidgetItem* item = ui->cameraListWidget->item(curSelect);
        QWidget* itemWidget = ui->cameraListWidget->itemWidget(item);
        CSListItem* csItem = qobject_cast<CSListItem*>(itemWidget);
        if (csItem)
        {
            ui->cameraListWidget->setCurrentRow(curSelect);
            csItem->setSelected(true);
        }
    }
}

void  CameraListWidget::addListWidgetItem(const QString& text)
{
    QIcon icon = QIcon(":/resources/tick.png");
    CSListItem* item = new CSListItem(icon, text, ui->cameraListWidget);
    item->setActionText(tr("Disconnect"), true);
    item->setActionText(tr("Connect"), false);

    ui->cameraListWidget->addItem(item->getListWidgetItem());
    ui->cameraListWidget->setItemWidget(item->getListWidgetItem(), item);

    connect(item, &CSListItem::toggled, this, &CameraListWidget::onClickedCameraListItem);
}

void CameraListWidget::onCameraStateChanged(int state)
{
    CAMERA_STATE cameraState = (CAMERA_STATE)state;
    
    bool selected = false;
    switch (cameraState)
    {
    case CAMERA_CONNECTED:
        selected = true;
        break;
    case CAMERA_DISCONNECTED:
    case CAMERA_DISCONNECTFAILED:
    case CAMERA_CONNECTFAILED:
        selected = false;
        break;
    default:
        return;
    }

    // update current list widget item
    int curRow = ui->cameraListWidget->currentRow();
    if (curRow >= 0)
    {
        QListWidgetItem* item = ui->cameraListWidget->item(curRow);
        QWidget* itemWidget = ui->cameraListWidget->itemWidget(item);
        CSListItem* csItem = qobject_cast<CSListItem*>(itemWidget);
        if (csItem)
        {
            csItem->setSelected(selected);
        }

        if (!selected)
        {
            // unselect all items
            ui->cameraListWidget->setCurrentRow(-1);
        }
    }
}

void CameraListWidget::onCameraListClicked(int rowIndex)
{

}

void CameraListWidget::initConnections()
{
    auto app = cs::CSApplication::getInstance();

    bool suc = true;
    suc &= (bool)connect(app, &cs::CSApplication::cameraListUpdated, this, &CameraListWidget::onCameraListUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::cameraStateChanged, this, &CameraListWidget::onCameraStateChanged);

    suc &= (bool)connect(this, &CameraListWidget::connectCamera,    app, &cs::CSApplication::connectCamera);
    suc &= (bool)connect(this, &CameraListWidget::disconnectCamera, app, &cs::CSApplication::disconnectCamera);

    suc &= (bool)connect(this, &CameraListWidget::translateSignal, this, &CameraListWidget::onTranslate);
    suc &= (bool)connect(ui->cameraListWidget, &QListWidget::currentRowChanged, this, &CameraListWidget::onCameraListClicked);

    Q_ASSERT(suc);
}

void CameraListWidget::iniWidget()
{
    ui->cameraListWidget->setFocusPolicy(Qt::NoFocus);
    initTopButton();
}

void CameraListWidget::onTranslate()
{
    ui->retranslateUi(this);
}

void CameraListWidget::initTopButton()
{
    QIcon icon;
    QSize size(10, 10);

    icon.addFile(QStringLiteral(":/resources/double_arrow_down.png"), size, QIcon::Normal, QIcon::Off);
    icon.addFile(QStringLiteral(":/resources/double_arrow_left.png"), size, QIcon::Selected, QIcon::On);
    CSTextImageButton* topButton = new CSTextImageButton(icon, "Camera List", Qt::LeftToRight, ui->topItem);

    auto* layout = ui->topItem->layout();
    if (layout)
    {
        layout->addWidget(topButton);
    }

    connect(topButton, &CSTextImageButton::toggled, [=](bool checked)
        {
            if (checked)
            {
                ui->scrollArea->hide();
            }
            else
            {
                ui->scrollArea->show();
            }
        });
}
