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
    , m_ui(new Ui::CameraListWidget)
{
    setAttribute(Qt::WA_StyledBackground);
    m_ui->setupUi(this);
    iniWidget();
    initConnections();

    setObjectName("CameraListWidget");
}

CameraListWidget::~CameraListWidget()
{
    delete m_ui;
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

        // clear last
        for (int i = 0; i < m_ui->cameraListWidget->count(); i++)
        {
            QListWidgetItem* item = m_ui->cameraListWidget->item(i);
            QWidget* itemWidget = m_ui->cameraListWidget->itemWidget(item);
            CSListItem* csItem = qobject_cast<CSListItem*>(itemWidget);
            if (csItem)
            {
                csItem->setSelected(false);
            }
        }

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

    m_ui->cameraListWidget->clear();
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
        QListWidgetItem* item = m_ui->cameraListWidget->item(curSelect);
        QWidget* itemWidget = m_ui->cameraListWidget->itemWidget(item);
        CSListItem* csItem = qobject_cast<CSListItem*>(itemWidget);
        if (csItem)
        {
            csItem->setSelected(true);
        }
    }
}

void  CameraListWidget::addListWidgetItem(const QString& text)
{
    QIcon icon = QIcon(":/resources/tick.png");
    CSListItem* item = new CSListItem(icon, text, QT_TR_NOOP("Disconnect"), QT_TR_NOOP("Connect"),  "CameraListWidget", m_ui->cameraListWidget);
    m_ui->cameraListWidget->addItem(item->getListWidgetItem());
    m_ui->cameraListWidget->setItemWidget(item->getListWidgetItem(), item);

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

    // update current list widget 
    auto curCameraSerial = QString(cs::CSApplication::getInstance()->getCamera()->getCameraInfo().cameraInfo.serial);
    for (int i = 0; i < m_ui->cameraListWidget->count(); i++)
    {
        QListWidgetItem* item = m_ui->cameraListWidget->item(i);
        QWidget* itemWidget = m_ui->cameraListWidget->itemWidget(item);
        CSListItem* csItem = qobject_cast<CSListItem*>(itemWidget);
        if (csItem)
        {
            if (!curCameraSerial.isEmpty() && csItem->getText().contains(curCameraSerial))
            {
                csItem->setSelected(selected);
            }
            else 
            {
                csItem->setSelected(false);
            }     
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
    suc &= (bool)connect(m_ui->cameraListWidget, &QListWidget::currentRowChanged, this, &CameraListWidget::onCameraListClicked);

    Q_ASSERT(suc);
}

void CameraListWidget::iniWidget()
{
    m_ui->cameraListWidget->setFocusPolicy(Qt::NoFocus);
    initTopButton();
}

void CameraListWidget::onTranslate()
{
    m_ui->retranslateUi(this);
    m_topItemButton->retranslate("CameraListWidget");
}

void CameraListWidget::initTopButton()
{
    QIcon icon;
    QSize size(10, 10);

    icon.addFile(QStringLiteral(":/resources/double_arrow_down.png"), size, QIcon::Normal, QIcon::Off);
    icon.addFile(QStringLiteral(":/resources/double_arrow_left.png"), size, QIcon::Selected, QIcon::On);
    m_topItemButton = new CSTextImageButton(icon, QT_TR_NOOP("Camera List"), Qt::LeftToRight, m_ui->topItem);

    auto* layout = m_ui->topItem->layout();
    if (layout)
    {
        layout->addWidget(m_topItemButton);
    }

    connect(m_topItemButton, &CSTextImageButton::toggled, [=](bool checked)
        {
            if (checked)
            {
                m_ui->scrollArea->hide();
            }
            else
            {
                m_ui->scrollArea->show();
            }
        });
}
