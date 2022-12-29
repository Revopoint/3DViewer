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

#include "capturesettingdialog.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QListView>
#include <QFile>
#include <QIntValidator>
#include <QMessageBox>

#include <icscamera.h>

#include "csapplication.h"
#include "appconfig.h"
#include "ui_capturesetting.h"

#include <cameracapturetool.h>

static QStringList captureSaveFormats = { "images", "raw" };

CaptureSettingDialog::CaptureSettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CaptureSettingWidget)
{
    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint);

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
    camera->getCameraPara(cs::parameter::PARA_DEPTH_HAS_IR, hasIrV);
    camera->getCameraPara(cs::parameter::PARA_HAS_DEPTH, hasDepthV);

    const bool hasRgb = hasRgbV.toBool();
    const bool hasIr = hasIrV.toBool();
    const bool hasDepth = hasDepthV.toBool();

    for (auto checkBox : dataTypeCheckBoxs)
    {
        checkBox->setChecked(false);
    }

    ui->rgbCheckBox->setEnabled(hasRgb);
    ui->rgbCheckBox->setChecked(hasRgb);

    ui->depthCheckBox->setEnabled(hasDepth);
    ui->depthCheckBox->setChecked(hasDepth);


    ui->irCheckBox->setEnabled(hasIr);
    ui->pointCloudCheckBox->setEnabled(hasDepth);
    ui->captureInfo->setText("");

    ui->startCaptureButton->setEnabled(true);
    ui->stopCaptureButton->setEnabled(false);
    ui->statusBarText->setText("");

    QDialog::showEvent(event);
}

void CaptureSettingDialog::onStartCapture()
{
    qInfo() << "onStartCapture";

    if (captureConfig.captureDataTypes.size() <= 0)
    {
        qInfo() << "At least one data needs to be selected";

        ui->statusBarText->setText(tr("Capture failed ! At least one data needs to be selected"));
        return;
    }

    QString filters = "Zip file(*.zip)";
    QString openDir = QString("file:///%1").arg(cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath());
    QUrl url = QFileDialog::getSaveFileUrl(this, tr("Capture frame data"), openDir, filters);

    if (url.isValid())
    {
        QFileInfo fileInfo(url.toLocalFile());

        captureConfig.saveDir = fileInfo.absolutePath();

        QString name = fileInfo.fileName();

        captureConfig.saveName = name.endsWith(".zip") ? (name.replace(name.lastIndexOf(".zip"), 4, "")) : name;
        captureConfig.savePointCloudWithTexture = cs::CSApplication::getInstance()->getShow3DTexture();

        isCapturing = true;
        cs::CSApplication::getInstance()->startCapture(captureConfig);
        ui->captureInfo->setText("");

        ui->startCaptureButton->setEnabled(false);
        ui->stopCaptureButton->setEnabled(true);
    }
    else
    {
        qInfo() << "Cancel capture";
    }

}

void CaptureSettingDialog::onStopCapture()
{
    qInfo() << "onStopCapture";
    isCapturing = false;

    ui->startCaptureButton->setEnabled(true);
    ui->stopCaptureButton->setEnabled(false);
    cs::CSApplication::getInstance()->stopCapture();
}

void CaptureSettingDialog::initDefaultCaptureConfig()
{
    captureConfig.captureNumber = defaultCaptureCount;
    captureConfig.captureDataTypes = { CAMERA_DATA_DEPTH, CAMERA_DATA_RGB };
    captureConfig.saveFormat = captureSaveFormats.first();
    captureConfig.captureType = CAPTURE_TYPE_MULTIPLE;
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
        captureConfig.captureDataTypes.push_back(CAMERA_DATA_POINT_CLOUD);
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
    auto tex = ui->frameNumberLineEdit->text();
    int pos = 0;
    if (intValidator->validate(tex, pos) != QIntValidator::Acceptable)
    {
        ui->frameNumberLineEdit->setText(QString::number(1));
        captureConfig.captureNumber = 1;
    }
    else 
    {
        captureConfig.captureNumber = tex.toInt();
    }
}

void CaptureSettingDialog::initDialog()
{
    ui->startCaptureButton->setProperty("isCSStyle", true);
    ui->stopCaptureButton->setProperty("isCSStyle", true);
    
    QString frameRange = QString("[%1 - %2]").arg(minCaptureCount).arg(maxCaptureCount);
    ui->frameNumberRange->setText(frameRange);

    intValidator = new QIntValidator(minCaptureCount, maxCaptureCount, this);
    ui->frameNumberLineEdit->setValidator(intValidator);

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
        case CAMERA_DATA_POINT_CLOUD:
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

    ui->saveFormatComboBox->setView(new QListView(ui->saveFormatComboBox));
}

void CaptureSettingDialog::initConnections()
{
    bool suc = true;
    suc &= (bool)connect(ui->startCaptureButton, &QPushButton::clicked, this, &CaptureSettingDialog::onStartCapture);
    suc &= (bool)connect(ui->stopCaptureButton,  &QPushButton::clicked, this, &CaptureSettingDialog::onStopCapture);

    for (auto checkBox : dataTypeCheckBoxs)
    {
        suc &= (bool)connect(checkBox, &QCheckBox::toggled, this, &CaptureSettingDialog::onDataTypeChanged);
    }

    suc &= (bool)connect(ui->saveFormatComboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CaptureSettingDialog::onSaveFormatChanged);
    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app, &cs::CSApplication::captureNumberUpdated, this, &CaptureSettingDialog::onCaptureNumberUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::captureStateChanged,  this, &CaptureSettingDialog::onCaptureStateChanged);

    suc &= (bool)connect(ui->frameNumberLineEdit,  &QLineEdit::editingFinished, this, &CaptureSettingDialog::onCaptureFrameNumberChanged);
    suc &= (bool)connect(ui->frameNumberLineEdit, &CSLineEdit::focusOutSignal,  this, &CaptureSettingDialog::onCaptureFrameNumberChanged);

    Q_ASSERT(suc);
}

void CaptureSettingDialog::onCaptureNumberUpdated(int captured, int dropped)
{
    QString msg = QString(tr("Captured %1 frames (%2 dropped)")).arg(captured).arg(dropped);
    ui->captureInfo->setText(msg);
}

void CaptureSettingDialog::onCaptureStateChanged(int captureType, int state, QString message)
{
    if (captureType != CAPTURE_TYPE_MULTIPLE)
    {
        return;
    }
    ui->statusBarText->setText(message);

    if (state == cs::CAPTURE_FINISHED)
    {
        isCapturing = false;
        ui->startCaptureButton->setEnabled(true);
        ui->stopCaptureButton->setEnabled(false);
    }
}

void CaptureSettingDialog::onTranslate()
{
    ui->retranslateUi(this);
}

void CaptureSettingDialog::reject()
{
    if (isCapturing)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Tips"));
        msgBox.setText(tr("Capturing, are you sure to stop capture now ?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.button(QMessageBox::Yes)->setText(tr("Yes"));
        msgBox.button(QMessageBox::No)->setText(tr("No"));

        int button = msgBox.exec();
        if (button == QMessageBox::No)
        {

        }
        else if (button == QMessageBox::Yes)
        {
            onStopCapture();
            QDialog::reject();
        }
    }
    else
    {
        QDialog::reject();
    }
}
