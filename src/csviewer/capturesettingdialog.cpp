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

#include "capturesettingdialog.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>

#include <icscamera.h>

#include "csapplication.h"
#include "appconfig.h"
#include "ui_capturesetting.h"

static QStringList captureSaveFormats = { "images", "raw" };

CaptureSettingDialog::CaptureSettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CaptureSettingWidget)
{
    ui->setupUi(this);
    dataTypeCheckBoxs = { ui->rgbCheckBox, ui->irCheckBox, ui->depthCheckBox, ui->pointCloudCheckBox, };

    // init default capture setting
    initDefaultCaptureConfig();

    // init ui
    initDialog();

    // init connections
    initConnections();
}

CaptureSettingDialog::~CaptureSettingDialog()
{
    delete ui;
}

void CaptureSettingDialog::showEvent(QShowEvent* event)
{
    auto camera = cs::CSApplication::getInstance()->getCamera();
    QVariant hasRgbV, hasIrV, hasDepthV;
    camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    camera->getCameraPara(cs::parameter::PARA_DEPTH_HAS_LR, hasIrV);
    camera->getCameraPara(cs::parameter::PARA_HAS_DEPTH, hasDepthV);

    const bool hasRgb = hasRgbV.toBool();
    const bool hasIr = hasIrV.toBool();
    const bool hasDepth = hasDepthV.toBool();

    ui->rgbCheckBox->setEnabled(hasRgb);
    ui->depthCheckBox->setEnabled(hasDepth);
    ui->irCheckBox->setEnabled(hasIr);
    ui->pointCloudCheckBox->setEnabled(hasDepth);

    QDialog::showEvent(event);
}

void CaptureSettingDialog::onStartCapture()
{
    qInfo() << "onStartCapture";

    QString openDir = cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath();
    QUrl url = QFileDialog::getSaveFileUrl(this, tr("Capture frame data"), openDir);

    if (url.isValid())
    {
        QFileInfo fileInfo(url.toLocalFile());

        captureConfig.saveDir = fileInfo.absolutePath();
        captureConfig.saveName = fileInfo.fileName();
        cs::CSApplication::getInstance()->startCapture(captureConfig);
    }
    else
    {
        qInfo() << "Cancel capture";
    }

}

void CaptureSettingDialog::onStopCapture()
{
    qInfo() << "onStopCapture";
    cs::CSApplication::getInstance()->stopCapture();
}

void CaptureSettingDialog::initDefaultCaptureConfig()
{
    captureConfig.captureNumber = defaultCaptureCount;
    captureConfig.captureDataTypes = { CAMERA_DATA_DEPTH, CAMERA_DATA_RGB };
    captureConfig.saveFormat = captureSaveFormats.first();
}

void CaptureSettingDialog::onDataTypeChanged()
{
    captureConfig.captureDataTypes.clear();

    if (ui->rgbCheckBox->isChecked())
    {
        captureConfig.captureDataTypes.push_back(CAMERA_DATA_RGB);
    }

    if (ui->irCheckBox->isChecked())
    {
        captureConfig.captureDataTypes.push_back(CAMERA_DATA_L);
        captureConfig.captureDataTypes.push_back(CAMERA_DATA_R);
    }

    if (ui->depthCheckBox->isChecked())
    {
        captureConfig.captureDataTypes.push_back(CAMERA_DATA_DEPTH);
    }

    if (ui->pointCloudCheckBox->isChecked())
    {
        captureConfig.captureDataTypes.push_back(CAMERA_DTA_POINT_CLOUD);
    }
}

void CaptureSettingDialog::onSaveFormatChanged(int index)
{
    if (index >=0 && index < captureSaveFormats.size())
    {
        captureConfig.saveFormat = captureSaveFormats.at(index);
    }
    else 
    {
        qWarning() << "onSaveFormatChanged, invalid index : " << index;
    }
}

void CaptureSettingDialog::onCaptureFrameNumberChanged()
{
    captureConfig.captureNumber = ui->frameNumberSlider->value();
    ui->frameNumberLineEdit->setText(QString::number(captureConfig.captureNumber));
}

void CaptureSettingDialog::onFrameNumberLineEditChanged()
{
    QString text = ui->frameNumberLineEdit->text();
    ui->frameNumberSlider->setValue(text.toInt());

    captureConfig.captureNumber = text.toInt();
}

void CaptureSettingDialog::initDialog()
{
    ui->startCaptureButton->setProperty("isCSStyle", true);
    ui->stopCaptureButton->setProperty("isCSStyle", true);

    ui->frameNumberSlider->setRange(minCaptureCount, maxCaptureCount);

    // default data types
    for (auto type : captureConfig.captureDataTypes)
    {
        switch (type)
        {
        case CAMERA_DATA_RGB:
            ui->rgbCheckBox->setChecked(true);
            break;
        case CAMERA_DATA_L:
        case CAMERA_DATA_R:
            ui->irCheckBox->setChecked(true);
            break;
        case CAMERA_DATA_DEPTH:
            ui->depthCheckBox->setChecked(true);
            break;
        case CAMERA_DTA_POINT_CLOUD:
            ui->pointCloudCheckBox->setChecked(true);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    // save format combobox
    ui->saveFormatComboBox->addItems(captureSaveFormats);
    
    ui->frameNumberLineEdit->setText(QString::number(captureConfig.captureNumber));
    ui->frameNumberSlider->setValue(captureConfig.captureNumber);
}

void CaptureSettingDialog::initConnections()
{
    bool suc = true;
    suc &= (bool)connect(ui->startCaptureButton, &QPushButton::clicked, this, &CaptureSettingDialog::onStartCapture);
    suc &= (bool)connect(ui->stopCaptureButton,  &QPushButton::clicked, this, &CaptureSettingDialog::onStopCapture);

    for (auto checkBox : dataTypeCheckBoxs)
    {
        suc &= (bool)connect(ui->rgbCheckBox, &QCheckBox::toggled, this, &CaptureSettingDialog::onDataTypeChanged);
    }

    suc &= (bool)connect(ui->saveFormatComboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CaptureSettingDialog::onSaveFormatChanged);
    suc &= (bool)connect(ui->frameNumberSlider,   &QSlider::sliderReleased,    this, &CaptureSettingDialog::onCaptureFrameNumberChanged);
    suc &= (bool)connect(ui->frameNumberLineEdit, &QLineEdit::editingFinished, this, &CaptureSettingDialog::onFrameNumberLineEditChanged);

    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app, &cs::CSApplication::captureNumberUpdated, this, &CaptureSettingDialog::onCaptureNumberUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::captureStateChanged,  this, &CaptureSettingDialog::onCaptureStateChanged);

    Q_ASSERT(suc);
}

void CaptureSettingDialog::onCaptureNumberUpdated(int captured, int dropped)
{
    QString msg = QString("captured %1 frames (%2 dropped)").arg(captured).arg(dropped);
    ui->captureInfo->setText(msg);
}

void CaptureSettingDialog::onCaptureStateChanged(int state, QString message)
{

}