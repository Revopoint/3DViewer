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
    , m_hdrModeWidget(hdrMode)
    , m_hdrLevelWidget(hdrLevel)
    , m_hdrTableWidget(hdrSettings)
    , m_hdrMeterButton(new QPushButton(tr("Refresh"), this))
    , m_hdrOkButton(new QPushButton(tr("OK"), this))
    , m_circleProgressBar(new CSProgressBar(this))
{
    setWindowTitle(tr("HDR Setting"));
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    resize(400, 520);
    setModal(true);

    auto widgets = { m_hdrModeWidget, m_hdrLevelWidget, m_hdrTableWidget };
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
    int x = globalPos.x() + (width() / 2) - m_circleProgressBar->width() / 2;
    int y = globalPos.y() + (height() / 2) - m_circleProgressBar->height() / 2;
    m_circleProgressBar->setGeometry(x, y, m_circleProgressBar->width(), m_circleProgressBar->height());
}

void HDRSettingsDialog::initDialog()
{
    m_circleProgressBar->resize(100, 100);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setSpacing(10);
    rootLayout->setContentsMargins(10, 15, 10, 15);

    rootLayout->addWidget(m_hdrModeWidget);
    rootLayout->addWidget(m_hdrLevelWidget);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(m_hdrMeterButton);
    hLayout->setContentsMargins(20, 0, 20, 0);
    rootLayout->addItem(hLayout);

    m_hdrMeterButton->setProperty("isCSStyle", true);
    m_hdrOkButton->setProperty("isCSStyle", true);

    rootLayout->addWidget(m_hdrTableWidget);

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    hLayout2->addWidget(m_hdrOkButton);
    hLayout2->setContentsMargins(20, 0, 20, 0);
    rootLayout->addItem(hLayout2);
}

void HDRSettingsDialog::initConnections()
{
    bool suc = true;
    suc &= (bool)connect(m_hdrMeterButton, &QPushButton::clicked, this, &HDRSettingsDialog::refreshHdrSetting);
    suc &= (bool)connect(m_hdrOkButton, &QPushButton::clicked, this, &HDRSettingsDialog::updateHdrSetting);

    suc &= (bool)connect(this, &HDRSettingsDialog::progressStateChanged, this, [=](bool active) 
        {
            if (active)
            {
                updateProgressPosition();
                m_circleProgressBar->open();
            }
            else 
            {
                m_circleProgressBar->close();
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
        m_hdrLevelWidget->setEnabled(false);
        m_hdrTableWidget->setEnabled(false);
        m_hdrMeterButton->setEnabled(false);
        m_hdrOkButton->setEnabled(false);
        break;
    case HDR_MODE_MANUAL:
        m_hdrTableWidget->setEnabled(true);
        m_hdrLevelWidget->setEnabled(true);
        m_hdrMeterButton->setEnabled(false);
        m_hdrOkButton->setEnabled(true);
        break;
    default:
        m_hdrTableWidget->setEnabled(false);
        m_hdrLevelWidget->setEnabled(true);
        m_hdrMeterButton->setEnabled(true);
        m_hdrOkButton->setEnabled(false);
        break;
    }
}

void HDRSettingsDialog::onTranslate()
{
    m_hdrMeterButton->setText(tr("Refresh"));
    m_hdrOkButton->setText(tr("OK"));
    setWindowTitle(tr("HDR Setting"));
}