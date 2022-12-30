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

#include "hdrsettingsdialog.h"

#include <QVariant>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QTableWidget>
#include <QVBoxLayout> 
#include <QHeaderView>
#include <QTableWidgetItem>

#include <cameraparaid.h>
#include <cstypes.h>
#include "cswidgets/cscombobox.h"
#include "cswidgets/csprogressbar.h"

using namespace cs::parameter;
#define HDR_TABLE_ROWS 10
#define HDR_TABLE_COLS 3

HDRSettingsDialog::HDRSettingsDialog(CSParaWidget* hdrMode, CSParaWidget* hdrLevel, CSParaWidget* hdrSettings, QWidget* parent)
    : QDialog(parent)
    , hdrModeWidget(hdrMode)
    , hdrLevelWidget(hdrLevel)
    , hdrTableWidget(hdrSettings)
    , hdrMeterButton(new QPushButton(tr("Refresh"), this))
    , hdrOkButton(new QPushButton(tr("OK"), this))
    , circleProgressBar(new CSProgressBar(this))
{
    setWindowTitle(tr("HDR Setting"));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    resize(400, 520);
    setModal(true);

    auto widgets = { hdrModeWidget, hdrLevelWidget, hdrTableWidget };
    for (auto widget : widgets)
    {
        widget->setParent(this);
    }

    initDialog();
    initConnections();
}

HDRSettingsDialog::~HDRSettingsDialog()
{

}

void HDRSettingsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    updateProgressPosition();
}

void HDRSettingsDialog::resizeEvent(QResizeEvent* event)
{
    updateProgressPosition();
}

void HDRSettingsDialog::updateProgressPosition()
{
    QPoint globalPos = this->mapToGlobal(QPoint(0, 0));
    int x = globalPos.x() + (width() / 2) - circleProgressBar->width() / 2;
    int y = globalPos.y() + (height() / 2) - circleProgressBar->height() / 2;
    circleProgressBar->setGeometry(x, y, circleProgressBar->width(), circleProgressBar->height());
}

void HDRSettingsDialog::initDialog()
{
    circleProgressBar->resize(100, 100);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setSpacing(10);
    rootLayout->setContentsMargins(10, 15, 10, 15);

    rootLayout->addWidget(hdrModeWidget);
    rootLayout->addWidget(hdrLevelWidget);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(hdrMeterButton);
    hLayout->setContentsMargins(20, 0, 20, 0);
    rootLayout->addItem(hLayout);

    hdrMeterButton->setProperty("isCSStyle", true);
    hdrOkButton->setProperty("isCSStyle", true);

    rootLayout->addWidget(hdrTableWidget);

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    hLayout2->addWidget(hdrOkButton);
    hLayout2->setContentsMargins(20, 0, 20, 0);
    rootLayout->addItem(hLayout2);
}

void HDRSettingsDialog::initConnections()
{
    bool suc = true;
    suc &= (bool)connect(hdrMeterButton, &QPushButton::clicked, this, &HDRSettingsDialog::refreshHdrSetting);
    suc &= (bool)connect(hdrOkButton, &QPushButton::clicked, this, &HDRSettingsDialog::updateHdrSetting);

    suc &= (bool)connect(this, &HDRSettingsDialog::progressStateChanged, this, [=](bool active) 
        {
            if (active)
            {
                updateProgressPosition();
                circleProgressBar->open();
            }
            else 
            {
                circleProgressBar->close();
            }
        });

    Q_ASSERT(suc);
}

void HDRSettingsDialog::onHdrModeChanged(int mode)
{
    CAMERA_HDR_MODE hdrMode = (CAMERA_HDR_MODE)mode;
    switch (hdrMode)
    {
    case HDR_MODE_CLOSE:
        hdrLevelWidget->setEnabled(false);
        hdrTableWidget->setEnabled(false);
        hdrMeterButton->setEnabled(false);
        hdrOkButton->setEnabled(false);
        break;
    case HDR_MODE_MANUAL:
        hdrTableWidget->setEnabled(true);
        hdrLevelWidget->setEnabled(true);
        hdrMeterButton->setEnabled(false);
        hdrOkButton->setEnabled(true);
        break;
    default:
        hdrTableWidget->setEnabled(false);
        hdrLevelWidget->setEnabled(true);
        hdrMeterButton->setEnabled(true);
        hdrOkButton->setEnabled(false);
        break;
    }
}

void HDRSettingsDialog::onTranslate()
{
    hdrMeterButton->setText(tr("Refresh"));
    hdrOkButton->setText(tr("OK"));
    setWindowTitle(tr("HDR Setting"));
}