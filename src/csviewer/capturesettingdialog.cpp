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
    , m_ui(new Ui::CaptureSettingWidget)
{
    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint);

    m_ui->setupUi(this);
    m_dataTypeCheckBoxs = { m_ui->rgbCheckBox, m_ui->irCheckBox, m_ui->depthCheckBox, m_ui->pointCloudCheckBox, };

    // init default capture setting
    initDefaultCaptureConfig();

    // init ui
    initDialog();

    // init connections
    initConnections();
}

CaptureSettingDialog::~CaptureSettingDialog()
{
    delete m_ui;
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

    for (auto checkBox : m_dataTypeCheckBoxs)
    {
        checkBox->setChecked(false);
    }

    m_ui->rgbCheckBox->setEnabled(hasRgb);
    m_ui->rgbCheckBox->setChecked(hasRgb);

    m_ui->depthCheckBox->setEnabled(hasDepth);
    m_ui->depthCheckBox->setChecked(hasDepth);


    m_ui->irCheckBox->setEnabled(hasIr);
    m_ui->pointCloudCheckBox->setEnabled(hasDepth);
    m_ui->captureInfo->setText("");

    m_ui->startCaptureButton->setEnabled(true);
    m_ui->stopCaptureButton->setEnabled(false);
    m_ui->statusBarText->setText("");

    QDialog::showEvent(event);
}

void CaptureSettingDialog::onStartCapture()
{
    qInfo() << "onStartCapture";

    if (m_captureConfig.captureDataTypes.size() <= 0)
    {
        qInfo() << "At least one data needs to be selected";

        m_ui->statusBarText->setText(tr("Capture failed ! At least one data needs to be selected"));
        return;
    }

    QString filters = "Zip file(*.zip)";
    QString openDir = QString("file:///%1").arg(cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath());
    QUrl url = QFileDialog::getSaveFileUrl(this, tr("Capture frame data"), openDir, filters);

    if (url.isValid())
    {
        QFileInfo fileInfo(url.toLocalFile());

        m_captureConfig.saveDir = fileInfo.absolutePath();

        QString name = fileInfo.fileName();

        m_captureConfig.saveName = name.endsWith(".zip") ? (name.replace(name.lastIndexOf(".zip"), 4, "")) : name;
        m_captureConfig.savePointCloudWithTexture = cs::CSApplication::getInstance()->getShow3DTexture();

        m_isCapturing = true;
        cs::CSApplication::getInstance()->startCapture(m_captureConfig);
        m_ui->captureInfo->setText("");

        m_ui->startCaptureButton->setEnabled(false);
        m_ui->stopCaptureButton->setEnabled(true);
    }
    else
    {
        qInfo() << "Cancel capture";
    }

}

void CaptureSettingDialog::onStopCapture()
{
    qInfo() << "onStopCapture";
    m_isCapturing = false;

    m_ui->startCaptureButton->setEnabled(true);
    m_ui->stopCaptureButton->setEnabled(false);
    cs::CSApplication::getInstance()->stopCapture();
}

void CaptureSettingDialog::initDefaultCaptureConfig()
{
    m_captureConfig.captureNumber = m_defaultCaptureCount;
    m_captureConfig.captureDataTypes = { CAMERA_DATA_DEPTH, CAMERA_DATA_RGB };
    m_captureConfig.saveFormat = captureSaveFormats.first();
    m_captureConfig.captureType = CAPTURE_TYPE_MULTIPLE;
}

void CaptureSettingDialog::onDataTypeChanged()
{
    m_captureConfig.captureDataTypes.clear();

    if (m_ui->rgbCheckBox->isChecked())
    {
        m_captureConfig.captureDataTypes.push_back(CAMERA_DATA_RGB);
    }

    if (m_ui->irCheckBox->isChecked())
    {
        m_captureConfig.captureDataTypes.push_back(CAMERA_DATA_L);
        m_captureConfig.captureDataTypes.push_back(CAMERA_DATA_R);
    }

    if (m_ui->depthCheckBox->isChecked())
    {
        m_captureConfig.captureDataTypes.push_back(CAMERA_DATA_DEPTH);
    }

    if (m_ui->pointCloudCheckBox->isChecked())
    {
        m_captureConfig.captureDataTypes.push_back(CAMERA_DATA_POINT_CLOUD);
    }
}

void CaptureSettingDialog::onSaveFormatChanged(int index)
{
    if (index >=0 && index < captureSaveFormats.size())
    {
        m_captureConfig.saveFormat = captureSaveFormats.at(index);
    }
    else 
    {
        qWarning() << "onSaveFormatChanged, invalid index : " << index;
    }
}

void CaptureSettingDialog::onCaptureFrameNumberChanged()
{
    auto tex = m_ui->frameNumberLineEdit->text();
    int pos = 0;
    if (m_intValidator->validate(tex, pos) != QIntValidator::Acceptable)
    {
        m_ui->frameNumberLineEdit->setText(QString::number(1));
        m_captureConfig.captureNumber = 1;
    }
    else 
    {
        m_captureConfig.captureNumber = tex.toInt();
    }
}

void CaptureSettingDialog::initDialog()
{
    m_ui->startCaptureButton->setProperty("isCSStyle", true);
    m_ui->stopCaptureButton->setProperty("isCSStyle", true);
    
    QString frameRange = QString("[%1 - %2]").arg(m_minCaptureCount).arg(m_maxCaptureCount);
    m_ui->frameNumberRange->setText(frameRange);

    m_intValidator = new QIntValidator(m_minCaptureCount, m_maxCaptureCount, this);
    m_ui->frameNumberLineEdit->setValidator(m_intValidator);

    // default data types
    for (auto type : m_captureConfig.captureDataTypes)
    {
        switch (type)
        {
        case CAMERA_DATA_RGB:
            m_ui->rgbCheckBox->setChecked(true);
            break;
        case CAMERA_DATA_L:
        case CAMERA_DATA_R:
            m_ui->irCheckBox->setChecked(true);
            break;
        case CAMERA_DATA_DEPTH:
            m_ui->depthCheckBox->setChecked(true);
            break;
        case CAMERA_DATA_POINT_CLOUD:
            m_ui->pointCloudCheckBox->setChecked(true);
            break;
        default:
            Q_ASSERT(false);
            break;
        }
    }

    // save format combobox
    m_ui->saveFormatComboBox->addItems(captureSaveFormats); 
    m_ui->frameNumberLineEdit->setText(QString::number(m_captureConfig.captureNumber));

    m_ui->saveFormatComboBox->setView(new QListView(m_ui->saveFormatComboBox));
}

void CaptureSettingDialog::initConnections()
{
    bool suc = true;
    suc &= (bool)connect(m_ui->startCaptureButton, &QPushButton::clicked, this, &CaptureSettingDialog::onStartCapture);
    suc &= (bool)connect(m_ui->stopCaptureButton,  &QPushButton::clicked, this, &CaptureSettingDialog::onStopCapture);

    for (auto checkBox : m_dataTypeCheckBoxs)
    {
        suc &= (bool)connect(checkBox, &QCheckBox::toggled, this, &CaptureSettingDialog::onDataTypeChanged);
    }

    suc &= (bool)connect(m_ui->saveFormatComboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CaptureSettingDialog::onSaveFormatChanged);
    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app, &cs::CSApplication::captureNumberUpdated, this, &CaptureSettingDialog::onCaptureNumberUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::captureStateChanged,  this, &CaptureSettingDialog::onCaptureStateChanged);

    suc &= (bool)connect(m_ui->frameNumberLineEdit,  &QLineEdit::editingFinished, this, &CaptureSettingDialog::onCaptureFrameNumberChanged);
    suc &= (bool)connect(m_ui->frameNumberLineEdit, &CSLineEdit::focusOutSignal,  this, &CaptureSettingDialog::onCaptureFrameNumberChanged);

    Q_ASSERT(suc);
}

void CaptureSettingDialog::onCaptureNumberUpdated(int captured, int dropped)
{
    QString msg = QString(tr("Captured %1 frames (%2 dropped)")).arg(captured).arg(dropped);
    m_ui->captureInfo->setText(msg);
}

void CaptureSettingDialog::onCaptureStateChanged(int captureType, int state, QString message)
{
    if (captureType != CAPTURE_TYPE_MULTIPLE)
    {
        return;
    }
    m_ui->statusBarText->setText(message);

    if (state == cs::CAPTURE_FINISHED)
    {
        m_isCapturing = false;
        m_ui->startCaptureButton->setEnabled(true);
        m_ui->stopCaptureButton->setEnabled(false);
    }
}

void CaptureSettingDialog::onTranslate()
{
    m_ui->retranslateUi(this);
}

void CaptureSettingDialog::reject()
{
    if (m_isCapturing)
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
        }
    }
    else
    {
        QDialog::reject();
    }
}
