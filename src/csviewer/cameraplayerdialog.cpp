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

#include <cameraplayer.h>
#include "appconfig.h"
#include "csapplication.h"
#include "csprogressbar.h"

CameraPlayerDialog::CameraPlayerDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CameraPlayerWidget)
    , cameraPlayer(nullptr)
    , circleProgressBar(new CSProgressBar(this))
{
    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint  & Qt::WindowMinMaxButtonsHint);

    ui->setupUi(this);
    ui->frameEdit->setValidator(new QIntValidator(-10000000, 10000000, this));
    ui->captureSingleButton->setToolTip(tr("Save current frame"));

    dataTypeCheckBoxs[(int)CAMERA_DATA_DEPTH] = ui->checkBoxDepth;
    dataTypeCheckBoxs[(int)CAMERA_DATA_RGB] = ui->checkBoxRgb;
    dataTypeCheckBoxs[(int)CAMERA_DATA_POINT_CLOUD] = ui->checkBoxPC;
    dataTypeCheckBoxs[(int)(CAMERA_DATA_R)] = ui->checkBoxIrR;
    dataTypeCheckBoxs[(int)(CAMERA_DATA_L)] = ui->checkBoxIrL;

    bool suc = true;
    for (auto checkBox : dataTypeCheckBoxs.values())
    {
        suc &= (bool)connect(checkBox, &QCheckBox::toggled, this, &CameraPlayerDialog::onToggledCheckBox);
    }

    suc &= (bool)connect(ui->frameNumberSlider, &QSlider::sliderReleased, this, &CameraPlayerDialog::onSliderValueChanged);

    suc &= (bool)connect(ui->frameIncrease, &QPushButton::clicked, this, [=]()
        {
            ui->frameNumberSlider->setValue(ui->frameNumberSlider->value() + 1);
            onSliderValueChanged();
        });

    suc &= (bool)connect(ui->frameDecrease, &QPushButton::clicked, this, [=]()
        {
            ui->frameNumberSlider->setValue(ui->frameNumberSlider->value() - 1);
            onSliderValueChanged();
        });

    suc &= (bool)connect(ui->frameEdit, &QLineEdit::editingFinished, this, &CameraPlayerDialog::onLineEditFinished);
    //suc &= (bool)connect(ui->frameEdit, &CSLineEdit::focusOutSignal, this, &CameraPlayerDialog::onLineEditFinished);

    suc &= (bool)connect(ui->playerRenderWindow,  &RenderWindow::renderExit, this, &CameraPlayerDialog::onRenderExit);
    suc &= (bool)connect(ui->captureSingleButton, &QPushButton::clicked,     this, &CameraPlayerDialog::onClickedSave);

    suc &= (bool)connect(this, &CameraPlayerDialog::output2DUpdated, ui->playerRenderWindow, &RenderWindow::onOutput2DUpdated);
    suc &= (bool)connect(this, &CameraPlayerDialog::output3DUpdated, ui->playerRenderWindow, &RenderWindow::onOutput3DUpdated);
    suc &= (bool)connect(cs::CSApplication::getInstance(), &cs::CSApplication::show3DTextureChanged, this, &CameraPlayerDialog::onShowTextureUpdated);

    
    Q_ASSERT(suc);
}

CameraPlayerDialog::~CameraPlayerDialog()
{
    if (cameraPlayer)
    {
        cameraPlayer->quit();
        cameraPlayer->wait();
       
        delete cameraPlayer;
        cameraPlayer = nullptr;
    }
    delete ui;
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
        if (!cameraPlayer)
        {
            cameraPlayer = new cs::CameraPlayer();

            connect(cameraPlayer, &cs::CameraPlayer::playerStateChanged, this, &CameraPlayerDialog::onPlayerStateChanged);    
            connect(cameraPlayer, &cs::CameraPlayer::output2DUpdated,    this, &CameraPlayerDialog::output2DUpdated);
            connect(cameraPlayer, &cs::CameraPlayer::output3DUpdated,    this, &CameraPlayerDialog::output3DUpdated);
            connect(this, &CameraPlayerDialog::loadFile, cameraPlayer,   &cs::CameraPlayer::onLoadFile);
            connect(this, &CameraPlayerDialog::currentFrameUpdated,      cameraPlayer, &cs::CameraPlayer::onPalyFrameUpdated);
            connect(this, &CameraPlayerDialog::saveCurrentFrame,         cameraPlayer, &cs::CameraPlayer::onSaveCurrentFrame);
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
        circleProgressBar->close();
        break;
    case cs::CameraPlayer::PLAYER_SAVING:
        circleProgressBar->open();
        break;
    default:
        break;
    }
}

void CameraPlayerDialog::onPlayReady()
{
    qInfo() << "on play ready";
    Q_ASSERT(cameraPlayer);

    auto dataTypes = cameraPlayer->getDataTypes();

    QVector<int> windows;
    for (auto type : dataTypes)
    {
        windows.push_back(type);
    }

    ui->playerRenderWindow->setShowTextureEnable(cameraPlayer->enablePointCloudTexture());
    ui->playerRenderWindow->onRenderWindowsUpdated(windows);

    // update check boxs
    for (auto checkBox : dataTypeCheckBoxs.values())
    {
        checkBox->setChecked(false);
        checkBox->setEnabled(false);
    }

    for (auto dataTye : dataTypeCheckBoxs.keys())
    {
        if (windows.contains(dataTye))
        {
            dataTypeCheckBoxs[dataTye]->setEnabled(true);
            dataTypeCheckBoxs[dataTye]->setChecked(true);
        }
    }

    cameraPlayer->currentDataTypesUpdated(windows);
    int frameNumber = cameraPlayer->getFrameNumber();
    updateFrameRange(frameNumber);
     
    ui->playerRenderWindow->hideRenderFps();

    show();
}

void CameraPlayerDialog::updateFrameRange(int frameNumer)
{
    ui->frameNumberSlider->setRange(1, frameNumer);
    ui->frameMinText->setText("1");
    ui->frameMaxText->setText(QString::number(frameNumer));
    ui->frameEdit->setText("1");
    ui->frameNumberSlider->setValue(1);

    emit currentFrameUpdated(1);
}

void CameraPlayerDialog::onToggledCheckBox()
{
    QVector<int> windows;
    for (auto dataTye : dataTypeCheckBoxs.keys())
    {
        if (dataTypeCheckBoxs[dataTye]->isEnabled() && dataTypeCheckBoxs[dataTye]->isChecked())
        {
            windows.push_back(dataTye);
        }
    }

    cameraPlayer->currentDataTypesUpdated(windows);
    ui->playerRenderWindow->onRenderWindowsUpdated(windows);
    ui->playerRenderWindow->hideRenderFps();

    emit currentFrameUpdated(ui->frameNumberSlider->value(), true);
}

void CameraPlayerDialog::onSliderValueChanged()
{
    //qDebug() << "onSliderValueChanged, value = " << ui->frameNumberSlider->value();
    QString text = QString::number(ui->frameNumberSlider->value());
    ui->frameEdit->setText(text);

    emit currentFrameUpdated(ui->frameNumberSlider->value());
}

void CameraPlayerDialog::onLineEditFinished()
{
    QString text = ui->frameEdit->text();
    int number = text.toInt();

    int min = ui->frameNumberSlider->minimum();
    int max = ui->frameNumberSlider->maximum();
    
    if (number >= min && number <= max)
    {
        ui->frameNumberSlider->setValue(number);
        emit currentFrameUpdated(ui->frameNumberSlider->value());
    }
    else 
    {
        ui->frameEdit->setText(QString::number(ui->frameNumberSlider->value()));
        emit currentFrameUpdated(ui->frameNumberSlider->value());
    }
}

void CameraPlayerDialog::onRenderExit(int renderId)
{
    for (auto dataTye : dataTypeCheckBoxs.keys())
    {
        if (dataTye == renderId)
        {
            dataTypeCheckBoxs[dataTye]->setChecked(false);
            onToggledCheckBox();
            break;
        }
    }
}

void CameraPlayerDialog::onShowTextureUpdated(bool texture)
{
    if (cameraPlayer)
    {
        cameraPlayer->onShow3DTextureChanged(texture);
        emit currentFrameUpdated(ui->frameNumberSlider->value(), true);
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
    ui->retranslateUi(this);
    ui->captureSingleButton->setToolTip(tr("Save current frame"));
}