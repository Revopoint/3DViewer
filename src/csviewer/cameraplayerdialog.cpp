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
    QString openDir = cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath();

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
    QString openDir = cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath();

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