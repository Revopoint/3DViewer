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

#include "cameraplayerdialog.h"
#include "ui_cameraplayer.h"

#include <QFileDialog>
#include <QDebug>
#include <QIntValidator>

#include <cameraplayer.h>
#include "appconfig.h"
#include "csapplication.h"
#include "csprogressbar.h"

CameraPlayerDialog::CameraPlayerDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::CameraPlayerWidget)
    , m_cameraPlayer(nullptr)
    , m_circleProgressBar(new CSProgressBar(this))
{
    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint  & Qt::WindowMinMaxButtonsHint);

    m_ui->setupUi(this);
    m_ui->frameEdit->setValidator(new QIntValidator(-10000000, 10000000, this));
    m_ui->captureSingleButton->setToolTip(tr("Save current frame"));

    m_dataTypeCheckBoxs[(int)CAMERA_DATA_DEPTH] = m_ui->checkBoxDepth;
    m_dataTypeCheckBoxs[(int)CAMERA_DATA_RGB] = m_ui->checkBoxRgb;
    m_dataTypeCheckBoxs[(int)CAMERA_DATA_POINT_CLOUD] = m_ui->checkBoxPC;
    m_dataTypeCheckBoxs[(int)(CAMERA_DATA_R)] = m_ui->checkBoxIrR;
    m_dataTypeCheckBoxs[(int)(CAMERA_DATA_L)] = m_ui->checkBoxIrL;

    bool suc = true;
    for (auto checkBox : m_dataTypeCheckBoxs.values())
    {
        suc &= (bool)connect(checkBox, &QCheckBox::toggled, this, &CameraPlayerDialog::onToggledCheckBox);
    }

    //suc &= (bool)connect(ui->frameNumberSlider, &QSlider::sliderReleased, this, &CameraPlayerDialog::onSliderValueChanged);
    suc &= (bool)connect(m_ui->frameNumberSlider, &QSlider::valueChanged, this, &CameraPlayerDialog::onSliderValueChanged);

    suc &= (bool)connect(m_ui->frameIncrease, &QPushButton::clicked, this, [=]()
        {
            m_ui->frameNumberSlider->setValue(m_ui->frameNumberSlider->value() + 1);
            onSliderValueChanged();
        });

    suc &= (bool)connect(m_ui->frameDecrease, &QPushButton::clicked, this, [=]()
        {
            m_ui->frameNumberSlider->setValue(m_ui->frameNumberSlider->value() - 1);
            onSliderValueChanged();
        });

    suc &= (bool)connect(m_ui->frameEdit, &QLineEdit::editingFinished, this, &CameraPlayerDialog::onLineEditFinished);
    //suc &= (bool)connect(ui->frameEdit, &CSLineEdit::focusOutSignal, this, &CameraPlayerDialog::onLineEditFinished);

    suc &= (bool)connect(m_ui->playerRenderWindow,  &RenderWindow::renderExit, this, &CameraPlayerDialog::onRenderExit);
    suc &= (bool)connect(m_ui->captureSingleButton, &QPushButton::clicked,     this, &CameraPlayerDialog::onClickedSave);

    suc &= (bool)connect(this, &CameraPlayerDialog::output2DUpdated, m_ui->playerRenderWindow, &RenderWindow::onOutput2DUpdated);
    suc &= (bool)connect(this, &CameraPlayerDialog::output3DUpdated, m_ui->playerRenderWindow, &RenderWindow::onOutput3DUpdated);
    suc &= (bool)connect(cs::CSApplication::getInstance(), &cs::CSApplication::show3DTextureChanged, this, &CameraPlayerDialog::onShowTextureUpdated);

    
    Q_ASSERT(suc);
}

CameraPlayerDialog::~CameraPlayerDialog()
{
    if (m_cameraPlayer)
    {
        m_cameraPlayer->quit();
        m_cameraPlayer->wait();
       
        delete m_cameraPlayer;
        m_cameraPlayer = nullptr;
    }
    delete m_ui;
}

void CameraPlayerDialog::onLoadFile()
{
    qInfo() << "click load file";
    QString openDir = QString("file:///%1").arg(cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath());

    QString filters = "Zip file(*.zip)";
    QUrl url = QFileDialog::getOpenFileUrl(this, tr("Load captured file"), openDir, filters);

    if (url.isValid())
    {
        QString filePath = url.toLocalFile();
        if (!m_cameraPlayer)
        {
            m_cameraPlayer = new cs::CameraPlayer();

            connect(m_cameraPlayer, &cs::CameraPlayer::playerStateChanged, this, &CameraPlayerDialog::onPlayerStateChanged);    
            connect(m_cameraPlayer, &cs::CameraPlayer::output2DUpdated,    this, &CameraPlayerDialog::output2DUpdated);
            connect(m_cameraPlayer, &cs::CameraPlayer::output3DUpdated,    this, &CameraPlayerDialog::output3DUpdated);
            connect(this, &CameraPlayerDialog::loadFile, m_cameraPlayer,   &cs::CameraPlayer::onLoadFile);
            connect(this, &CameraPlayerDialog::currentFrameUpdated,      m_cameraPlayer, &cs::CameraPlayer::onPalyFrameUpdated);
            connect(this, &CameraPlayerDialog::saveCurrentFrame,         m_cameraPlayer, &cs::CameraPlayer::onSaveCurrentFrame);
        }

        emit loadFile(filePath);
    }
    else
    {
        qInfo() << "Cancel select file";
    }
}

void CameraPlayerDialog::onPlayerStateChanged(int state, QString msg)
{
    emit showMessage(msg, 5000);
    
    cs::CameraPlayer::PLAYER_STATE playState = (cs::CameraPlayer::PLAYER_STATE)state;
    switch (playState)
    {
    case cs::CameraPlayer::PLAYER_LOADING:
        break;
    case cs::CameraPlayer::PLAYER_READY:
        onPlayReady();
        break;
    case cs::CameraPlayer::PLAYER_ERROR:
        break;
    case cs::CameraPlayer::PLAYER_SAVE_SUCCESS:
    case cs::CameraPlayer::PLAYER_SAVE_FAILED:
        m_circleProgressBar->close();
        break;
    case cs::CameraPlayer::PLAYER_SAVING:
        m_circleProgressBar->open();
        break;
    default:
        break;
    }
}

void CameraPlayerDialog::onPlayReady()
{
    qInfo() << "on play ready";
    Q_ASSERT(m_cameraPlayer);

    auto dataTypes = m_cameraPlayer->getDataTypes();

    QVector<int> windows;
    for (auto type : dataTypes)
    {
        windows.push_back(type);
    }

    m_ui->playerRenderWindow->setShowTextureEnable(m_cameraPlayer->enablePointCloudTexture());
    m_ui->playerRenderWindow->onRenderWindowsUpdated(windows);

    // update check boxs
    for (auto checkBox : m_dataTypeCheckBoxs.values())
    {
        checkBox->setChecked(false);
        checkBox->setEnabled(false);
    }

    for (auto dataTye : m_dataTypeCheckBoxs.keys())
    {
        if (windows.contains(dataTye))
        {
            m_dataTypeCheckBoxs[dataTye]->setEnabled(true);
            m_dataTypeCheckBoxs[dataTye]->setChecked(true);
        }
    }

    m_cameraPlayer->currentDataTypesUpdated(windows);
    int frameNumber = m_cameraPlayer->getFrameNumber();
    updateFrameRange(frameNumber);
     
    m_ui->playerRenderWindow->hideRenderFps();

    show();
}

void CameraPlayerDialog::updateFrameRange(int frameNumer)
{
    m_ui->frameNumberSlider->setRange(1, frameNumer);
    m_ui->frameMinText->setText("1");
    m_ui->frameMaxText->setText(QString::number(frameNumer));
    m_ui->frameEdit->setText("1");
    m_ui->frameNumberSlider->setValue(1);

    emit currentFrameUpdated(1);
}

void CameraPlayerDialog::onToggledCheckBox()
{
    QVector<int> windows;
    for (auto dataTye : m_dataTypeCheckBoxs.keys())
    {
        if (m_dataTypeCheckBoxs[dataTye]->isEnabled() && m_dataTypeCheckBoxs[dataTye]->isChecked())
        {
            windows.push_back(dataTye);
        }
    }

    m_cameraPlayer->currentDataTypesUpdated(windows);
    m_ui->playerRenderWindow->onRenderWindowsUpdated(windows);
    m_ui->playerRenderWindow->hideRenderFps();

    emit currentFrameUpdated(m_ui->frameNumberSlider->value(), true);
}

void CameraPlayerDialog::onSliderValueChanged()
{
    //qDebug() << "onSliderValueChanged, value = " << ui->frameNumberSlider->value();
    QString text = QString::number(m_ui->frameNumberSlider->value());
    m_ui->frameEdit->setText(text);

    emit currentFrameUpdated(m_ui->frameNumberSlider->value());
}

void CameraPlayerDialog::onLineEditFinished()
{
    QString text = m_ui->frameEdit->text();
    int number = text.toInt();

    int min = m_ui->frameNumberSlider->minimum();
    int max = m_ui->frameNumberSlider->maximum();
    
    if (number >= min && number <= max)
    {
        m_ui->frameNumberSlider->setValue(number);
        emit currentFrameUpdated(m_ui->frameNumberSlider->value());
    }
    else 
    {
        m_ui->frameEdit->setText(QString::number(m_ui->frameNumberSlider->value()));
        emit currentFrameUpdated(m_ui->frameNumberSlider->value());
    }
}

void CameraPlayerDialog::onRenderExit(int renderId)
{
    for (auto dataTye : m_dataTypeCheckBoxs.keys())
    {
        if (dataTye == renderId)
        {
            m_dataTypeCheckBoxs[dataTye]->setChecked(false);
            onToggledCheckBox();
            break;
        }
    }
}

void CameraPlayerDialog::onShowTextureUpdated(bool texture)
{
    if (m_cameraPlayer)
    {
        m_cameraPlayer->onShow3DTextureChanged(texture);
        emit currentFrameUpdated(m_ui->frameNumberSlider->value(), true);
    }
}

void CameraPlayerDialog::onClickedSave()
{
    QString openDir = QString("file:///%1").arg(cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath());

    qInfo() << "click capture single";
    QUrl url = QFileDialog::getSaveFileUrl(this, tr("Save current frame"), openDir);

    if (url.isValid())
    {
        emit saveCurrentFrame(url.toLocalFile());
    }
    else
    {
        qInfo() << "Cancel capture";
    }
}

void CameraPlayerDialog::onTranslate()
{
    m_ui->retranslateUi(this);
    m_ui->captureSingleButton->setToolTip(tr("Save current frame"));
}