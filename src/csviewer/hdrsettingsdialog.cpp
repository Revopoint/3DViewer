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