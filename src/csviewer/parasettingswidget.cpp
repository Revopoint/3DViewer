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

#include "parasettingswidget.h"
#include "./ui_parametersettings.h"

#include <QDebug>
#include <QVector>
#include <QPair>
#include <QVariant>
#include <QListView>
#include <QStringListModel>
#include <QTime>
#include <QThread>
#include <QDialog>
#include <QFileDialog>
#include <QDir>

#include <cameraproxy.h>
#include <icscamera.h>
#include <cameraparaid.h>

#include "csapplication.h"
#include "cswidgets/cscombobox.h"
#include "cswidgets/csline.h"
#include "cswidgets/csrangeedit.h"
#include "cswidgets/csslider.h"
#include "cswidgets/csspinbox.h"
#include "cswidgets/csswitchbutton.h"
#include "cswidgets/cstablewidget.h"

#include "hdrsettingsdialog.h"
#include "capturesettingdialog.h"

#define HDR_TABLE_COLS 3
using namespace cs::parameter;

ParaSettingsWidget::ParaSettingsWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ParameterSettingsWidget)
    , cameraPtr(cs::CSApplication::getInstance()->getCamera())
    , roiRectF(0,0,0,0)
    , captureSettingDialog(new CaptureSettingDialog)
{
    ui->setupUi(this);
    initWidget();
    initConnections();
}

ParaSettingsWidget::~ParaSettingsWidget()
{
    delete ui;
    delete hdrSettingsDialog;
    delete captureSettingDialog;
}

void ParaSettingsWidget::initDepthPara()
{
    // stream format
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_STREAM_FORMAT, QT_TR_NOOP("Stream Format:"), this));
    // resolution
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_RESOLUTION, QT_TR_NOOP("Resolution:"), this));
    // line
    addDepthDividLine();
       
    // depth range
    addDepthParaWidget(new CSRangeEdit(PARA_DEPTH_RANGE, QT_TR_NOOP("Depth Range:"), this));
    // line
    addDepthDividLine();

    // exposure
    addDepthParaWidget(new CSSlider(PARA_DEPTH_EXPOSURE, QT_TR_NOOP("Exposure Time(us):"), this));
    // gain
    addDepthParaWidget(new CSSpinBox(PARA_DEPTH_GAIN, QT_TR_NOOP("Gain:"), this));
    // auto exposure
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_AUTO_EXPOSURE, QT_TR_NOOP("Auto Exposure:"), this));
    // line
    addDepthDividLine();

    // Threshold
    addDepthParaWidget(new CSSlider(PARA_DEPTH_THRESHOLD, QT_TR_NOOP("Threshold:"), this));
    // line
    addDepthDividLine();

    // filter type
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_FILTER_TYPE, QT_TR_NOOP("Filter:"), this, Qt::Vertical));
    // filter
    addDepthParaWidget(new CSSlider(PARA_DEPTH_FILTER, "", this));
    // line
    addDepthDividLine();

    // fill hole
    addDepthParaWidget(new CSSwitchButton(PARA_DEPTH_FILL_HOLE, QT_TR_NOOP("Fill Hole:"), this));
    // line
    addDepthDividLine();

    // HDR settings
    initHDRParaWidgets();
}

void ParaSettingsWidget::initHDRParaWidgets()
{
    CSParaWidget* widgtes[] = {
        new CSComboBox(PARA_DEPTH_HDR_MODE, QT_TR_NOOP("HDR Mode:")),
        new CSComboBox(PARA_DEPTH_HDR_LEVEL, QT_TR_NOOP("HDR Level:")),
        new CSTableWidget(PARA_DEPTH_HDR_SETTINGS, HDR_TABLE_COLS, { QT_TR_NOOP("Number"), QT_TR_NOOP("Exposure"), QT_TR_NOOP("Gain")})
    };

    for (auto widget : widgtes)
    {
        auto paraId = widget->getParaId();
        Q_ASSERT(!paraWidgets.contains(paraId));
        paraWidgets[paraId] = widget;
    }

    hdrSettingsDialog = new HDRSettingsDialog(paraWidgets[PARA_DEPTH_HDR_MODE], paraWidgets[PARA_DEPTH_HDR_LEVEL], paraWidgets[PARA_DEPTH_HDR_SETTINGS]);
}

void ParaSettingsWidget::initRgbPara()
{
    // stream format
    addRgbParaWidget(new CSComboBox(PARA_RGB_STREAM_FORMAT, QT_TR_NOOP("Stream Format:"), this));
    // resolution
    addRgbParaWidget(new CSComboBox(PARA_RGB_RESOLUTION, QT_TR_NOOP("Resolution:"), this));
    // line
    addRgbDividLine();

    // auto exposure
    addRgbParaWidget(new CSSwitchButton(PARA_RGB_AUTO_EXPOSURE, QT_TR_NOOP("Auto Exposure:"), this));
    // exposure
    addRgbParaWidget(new CSSlider(PARA_RGB_EXPOSURE, QT_TR_NOOP("Exposure Time(us):"), this));
    // gain
    addRgbParaWidget(new CSSpinBox(PARA_RGB_GAIN, QT_TR_NOOP("Gain:"), this));
    // line
    addRgbDividLine();
 
    // auto white balance
    addRgbParaWidget(new CSSwitchButton(PARA_RGB_AUTO_WHITE_BALANCE, QT_TR_NOOP("Auto White Balance:"), this));
    // white balance
    addRgbParaWidget(new CSSlider(PARA_RGB_WHITE_BALANCE, QT_TR_NOOP("White Balance:"), this));
    // line
    addRgbDividLine();
}

void ParaSettingsWidget::initParaConnections()
{
    bool suc = true;
    suc &= (bool)connect(cameraPtr.get(), &cs::ICSCamera::cameraParaUpdated, this, &ParaSettingsWidget::onCameraParaUpdated);
    suc &= (bool)connect(cameraPtr.get(), &cs::ICSCamera::cameraParaRangeUpdated, this, &ParaSettingsWidget::onCameraParaRangeUpdated);
    suc &= (bool)connect(cameraPtr.get(), &cs::ICSCamera::cameraParaItemsUpdated, this, &ParaSettingsWidget::onCameraParaItemsUpdated);
    
    for (auto widget : paraWidgets.values())
    {
        suc &= (bool)connect(widget, &CSParaWidget::valueChanged, this, &ParaSettingsWidget::onParaValueChanged);
        Q_ASSERT(suc);
    }

    suc &= (bool)connect(hdrSettingsDialog, &HDRSettingsDialog::refreshHdrSetting, this, &ParaSettingsWidget::onRefreshHdrSetting);
    suc &= (bool)connect(hdrSettingsDialog, &HDRSettingsDialog::updateHdrSetting, this, &ParaSettingsWidget::onUpdateHdrSetting);
    suc &= (bool)connect(this, &ParaSettingsWidget::hdrModeChanged, hdrSettingsDialog, &HDRSettingsDialog::onHdrModeChanged);

    Q_ASSERT(suc);
}

void ParaSettingsWidget::initWidget()
{
    ui->depthCameraButton->setProperty("isCameraButton", true);
    ui->rgbCameraButton->setProperty("isCameraButton", true);
    ui->depthCameraButton->setChecked(true);

    ui->stackedWidget->setCurrentIndex(PAGE_DEPTH_CAMERA);

    ui->depthCameraButton->setChecked(true);
    ui->hdrSettingBtn->setProperty("isCSStyle", true);
    ui->roiEditBtn->setProperty("isCSStyle", true);
    ui->iconLabel->setPixmap(QPixmap::fromImage(QImage(":/resources/settings.png")));

    // control button
    ui->previewButton->setCheckable(true);
    ui->previewButton->setEnabled(false);
    ui->captureSingleButton->setEnabled(false);
    ui->captureMultipleButton->setEnabled(false);
    ui->singleShotCheckBox->setEnabled(false);
    ui->singleShotButton->setEnabled(ui->singleShotCheckBox->isChecked());

    //parameter widget
    initDepthPara();
    initRgbPara();
}

void ParaSettingsWidget::initConnections()
{
    bool suc = true;

    suc &= (bool)connect(ui->depthCameraButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickDepthButton);
    suc &= (bool)connect(ui->rgbCameraButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickRgbButton);
    suc &= (bool)connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &ParaSettingsWidget::onPageChanged);
    suc &= (bool)connect(ui->hdrSettingBtn, &QPushButton::clicked, this, &ParaSettingsWidget::onClickHdrButton);
    suc &= (bool)connect(ui->roiEditBtn, &QPushButton::toggled, this, &ParaSettingsWidget::onClickRoiEditButton);
    suc &= (bool)connect(ui->closeButton, &QPushButton::clicked, this, &ParaSettingsWidget::clickedCloseButton);

    suc &= (bool)connect(ui->previewButton, &QPushButton::clicked, this, &ParaSettingsWidget::onPreviewStateChanged);
    suc &= (bool)connect(ui->previewButton, &QPushButton::toggled, this, &ParaSettingsWidget::onPreviewButtonToggled);
    
    suc &= (bool)connect(ui->captureSingleButton,    &QPushButton::clicked,  this, &ParaSettingsWidget::onClickCaptureSingle);
    suc &= (bool)connect(ui->captureMultipleButton, &QPushButton::clicked,  this, &ParaSettingsWidget::onClickCaptureMultiple);

    suc &= (bool)connect(ui->singleShotButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickSingleShot);
    
    suc &= (bool)connect(ui->singleShotCheckBox, &QCheckBox::toggled, this, &ParaSettingsWidget::onSingleShotChanged);

    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app, &cs::CSApplication::cameraStateChanged, this, &ParaSettingsWidget::onCameraStateChanged);
    suc &= (bool)connect(this, &ParaSettingsWidget::translateSignal, this, &ParaSettingsWidget::onTranslate);
    suc &= (bool)connect(this, &ParaSettingsWidget::translateSignal, hdrSettingsDialog, &HDRSettingsDialog::onTranslate);

    Q_ASSERT(suc);

    // connections
    initParaConnections();
}

void ParaSettingsWidget::onClickDepthButton()
{
    //last state is not checked
    if (ui->depthCameraButton->isChecked())
    {
        ui->stackedWidget->setCurrentIndex(PAGE_DEPTH_CAMERA);
    }
    else 
    {
        ui->depthCameraButton->setChecked(true);
    }
}

void ParaSettingsWidget::onClickRgbButton()
{
    if (ui->rgbCameraButton->isChecked())
    {
        ui->stackedWidget->setCurrentIndex(PAGE_RGB_CAMERA);
    }
    else
    {
        ui->rgbCameraButton->setChecked(true);
    }
}

void ParaSettingsWidget::onClickHdrButton()
{
    hdrSettingsDialog->show();
}

void ParaSettingsWidget::onSingleShotChanged(bool checked)
{
    auto camera = cs::CSApplication::getInstance()->getCamera();
    ui->singleShotButton->setEnabled(checked);
    
    const int  triggerMode = checked ? TRIGGER_MODE_SOFTWAER : TRIGGER_MODE_OFF;
    camera->setCameraPara(PARA_TRIGGER_MODE, triggerMode);
    
    ui->roiEditWidget->setEnabled(!checked);
}

void ParaSettingsWidget::onClickSingleShot()
{
    auto camera = cs::CSApplication::getInstance()->getCamera();
    camera->softTrigger();
}

void ParaSettingsWidget::onClickRoiEditButton(bool checked)
{
    if (checked)
    {
        ui->roiEditBtn->setText(tr("Confirm"));
        emit showMessage(tr("Use the mouse to select the ROI area in the depth image, and then click the \"Confirm\" button"), 10000);
    }
    else 
    {
        ui->roiEditBtn->setText(tr("Edit ROI")); 
        emit showMessage("");

        //set parameter
        cameraPtr->setCameraPara(PARA_DEPTH_ROI, roiRectF);
    }
    roiRectF = QRectF(0, 0, 0, 0);
    emit roiStateChanged(checked);
}

void ParaSettingsWidget::onPageChanged(int idx)
{
    PARA_PAGE_ID curPage = (PARA_PAGE_ID)idx;

    switch (curPage)
    {
    case PAGE_DEPTH_CAMERA:
        ui->rgbCameraButton->setChecked(false);
        break;
    case PAGE_RGB_CAMERA:
        ui->depthCameraButton->setChecked(false);
        break;
    default:
        qDebug() << "invalid page index.";
        break;
    }
}

void ParaSettingsWidget::onCameraStateChanged(int state)
{
    updateWidgetsState(state);
    updateControlButtonState(state);

    CAMERA_STATE cameraState = (CAMERA_STATE)state;
    switch (cameraState)
    {
    case CAMERA_STARTED_STREAM:
        onUpdatedCameraParas();
        ui->previewButton->setChecked(true);
        break;
    case CAMERA_CONNECTED:
        {
            setEnabled(true);
            onUpdatedCameraParas();
            ui->previewButton->setChecked(false);
            break;
        }
    case CAMERA_DISCONNECTED:
    case CAMERA_DISCONNECTFAILED:
    case CAMERA_CONNECTFAILED:
        setEnabled(false);
        break;
    default:    
        ui->previewButton->setChecked(false);
        break;
    }

}

void ParaSettingsWidget::updateControlButtonState(int cameraState)
{
    bool enable = (cameraState == CAMERA_CONNECTED || cameraState == CAMERA_STARTED_STREAM || cameraState == CAMERA_PAUSED_STREAM
        || cameraState == CAMERA_STOPPED_STREAM);

    ui->previewButton->setEnabled(enable);
    ui->captureSingleButton->setEnabled(enable);
    ui->captureMultipleButton->setEnabled(enable);

    enable = (cameraState == CAMERA_STARTED_STREAM);

    if (cameraState == CAMERA_CONNECTED)
    {
        ui->singleShotCheckBox->setChecked(false);
    }

    ui->singleShotCheckBox->setEnabled(enable);
    ui->singleShotButton->setEnabled(enable && ui->singleShotCheckBox->isChecked());
}

void ParaSettingsWidget::updateWidgetsState(int cameraState)
{ 
    static QList<int> streamTypePara = 
    {
        PARA_DEPTH_STREAM_FORMAT,
        PARA_DEPTH_RESOLUTION,
        PARA_RGB_STREAM_FORMAT,
        PARA_RGB_RESOLUTION
    };

    // has rgb or not
    QVariant hasRgbV;
    cameraPtr->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    bool hasRgb = hasRgbV.toBool(); 
    ui->rgbCameraButton->setEnabled(hasRgb);
    ui->rgbParaWidget->setEnabled(hasRgb);

    for (auto widget : paraWidgets)
    {
        auto paraId = widget->getParaId();
        const bool isStreamTypePara = streamTypePara.contains(paraId);
        
        bool enable = (cameraState == CAMERA_CONNECTED || cameraState == CAMERA_PAUSED_STREAM || cameraState == CAMERA_STOPPED_STREAM) ? isStreamTypePara
            : (cameraState == CAMERA_STARTED_STREAM ? !isStreamTypePara : false);

        widget->setEnabled(enable);
    }

    // ROI
    ui->roiEditBtn->setEnabled(cameraState == CAMERA_STARTED_STREAM);
    ui->roiLabel->setEnabled(ui->roiEditBtn->isEnabled());

    // HDR
    ui->hdrSettingBtn->setEnabled(cameraState == CAMERA_STARTED_STREAM);
}

void ParaSettingsWidget::onUpdatedCameraParas()
{
    QTime time;
    time.start();

    // update range
    updateParaRanges();
    
    // update items
    updateParaItems();

    // update para value
    updateParaValues();

    int timeMs = time.elapsed();
    qInfo() << "spend time : " << timeMs;
}

void ParaSettingsWidget::updateParaRanges()
{
    for (auto widget : paraWidgets)
    {
        if (!widget->isEnabled())
        {
            continue;
        }

        QVariant min = 0, max = 0, step = 1;
        cameraPtr->getCameraParaRange((CAMERA_PARA_ID)widget->getParaId(), min, max, step);
        widget->setParaRange(min, max, step);
    }
}

void ParaSettingsWidget::updateParaValues()
{
    for (auto widget : paraWidgets)
    {
        if (!widget->isEnabled())
        {
            continue;
        }

        QVariant value;
        cameraPtr->getCameraPara((CAMERA_PARA_ID)widget->getParaId(), value);
        if (value == QVariant::Invalid)
        {
            continue;
        }

        widget->setValue(value);

        onParaLinkResponse((CAMERA_PARA_ID)widget->getParaId(), value);
    }

    //ROI
    if (ui->roiLabel->isEnabled())
    {
        QVariant value;
        cameraPtr->getCameraPara(PARA_DEPTH_ROI, value);
        if (value != QVariant::Invalid)
        {
            onCameraParaUpdated(PARA_DEPTH_ROI, value);
        }
    }
}

void ParaSettingsWidget::updateParaItems()
{
    for (auto widget : paraWidgets)
    {
        if (!widget->isEnabled())
        {
            continue;
        }

        QList<QPair<QString, QVariant>> list;
        cameraPtr->getCameraParaItems((CAMERA_PARA_ID)widget->getParaId(), list);

        if (!list.isEmpty())
        {
            widget->setItems(list);
        }
    }
}

void ParaSettingsWidget::addDepthParaWidget(CSParaWidget* widget)
{
    const auto& para = widget->getParaId();

    ui->depthParaLayout->addWidget(widget);

    Q_ASSERT(!paraWidgets.contains(para));
    paraWidgets[para] = widget;
}

void ParaSettingsWidget::addRgbParaWidget(CSParaWidget* widget)
{
    const auto& paraId = widget->getParaId();

    ui->rgbParaLayout->addWidget(widget);

    Q_ASSERT(!paraWidgets.contains(paraId));
    paraWidgets[paraId] = widget;
}

void ParaSettingsWidget::addDepthDividLine()
{
    ui->depthParaLayout->addWidget(new CSHLine(this));
}

void ParaSettingsWidget::addRgbDividLine()
{
    ui->rgbParaLayout->addWidget(new CSHLine(this));
}

// ui changed
void ParaSettingsWidget::onParaValueChanged(int paraId, QVariant value)
{
    cameraPtr->setCameraPara((CAMERA_PARA_ID)paraId, value);
    if (paraId == PARA_DEPTH_HDR_MODE || paraId == PARA_DEPTH_HDR_LEVEL)
    {
        emit hdrSettingsDialog->progressStateChanged(true);
    }
}

// camera para updated
void ParaSettingsWidget::onCameraParaUpdated(int paraId, QVariant value)
{
    if (paraWidgets.contains(paraId))
    {
        paraWidgets[paraId]->setValue(value);
        onParaLinkResponse(paraId, value);

        if (paraId == PARA_DEPTH_HDR_SETTINGS)
        {
            emit hdrSettingsDialog->progressStateChanged(false);
        }
    }
    else if (paraId == PARA_DEPTH_ROI)
    {
        QRectF rect = value.toRectF();

        QString str = QString("[%1, %2, %3, %4]").arg(QString::number(rect.left(),   'f', 2))
                                                 .arg(QString::number(rect.top(),    'f', 2))
                                                 .arg(QString::number(rect.right(),  'f', 2))
                                                 .arg(QString::number(rect.bottom(), 'f', 2));
        ui->roiLabel->setText(str);
    }
    else if (paraId == PARA_DEPTH_FRAMETIME || paraId == PARA_TRIGGER_MODE)
    {
    }
    else 
    {
        qDebug() << "unknow para : " << paraId;
        Q_ASSERT(false);
    }
}

void ParaSettingsWidget::onCameraParaRangeUpdated(int paraId)
{
    if (paraWidgets.contains(paraId))
    {
        QVariant min = 0, max = 0, step;
        cameraPtr->getCameraParaRange((CAMERA_PARA_ID)paraId, min, max, step);
        paraWidgets[paraId]->setParaRange(min, max, step);
    }
    else
    {
        qDebug() << "unknow para : " << paraId;
        Q_ASSERT(false);
    }
}

void ParaSettingsWidget::onCameraParaItemsUpdated(int paraId)
{
    if (paraWidgets.contains(paraId))
    {
        QList<QPair<QString, QVariant>> list;
        cameraPtr->getCameraParaItems((CAMERA_PARA_ID)paraId, list);

        if (!list.isEmpty())
        {
            paraWidgets[paraId]->setItems(list);
        }
    }
    else
    {
        qDebug() << "unknow para : " << paraId;
        Q_ASSERT(false);
    }
}

void ParaSettingsWidget::onParaLinkResponse(int paraId, QVariant value)
{
    CAMERA_PARA_ID cameraParaId = (CAMERA_PARA_ID)paraId;

    switch (paraId)
    {
    case PARA_DEPTH_AUTO_EXPOSURE:
        {        
            bool enable = !(value.toInt() > 0);
            paraWidgets[PARA_DEPTH_GAIN]->setEnabled(enable);
            paraWidgets[PARA_DEPTH_EXPOSURE]->setEnabled(enable);
        }
        break;
    case PARA_RGB_AUTO_EXPOSURE:
        paraWidgets[PARA_RGB_EXPOSURE]->setEnabled(!value.toBool());
        paraWidgets[PARA_RGB_GAIN]->setEnabled(!value.toBool());
        break;
    case PARA_RGB_AUTO_WHITE_BALANCE:
        paraWidgets[PARA_RGB_WHITE_BALANCE]->setEnabled(!value.toBool());
        break;
    case PARA_DEPTH_HDR_MODE:
        emit hdrModeChanged(value.toInt());
        break;
    case PARA_DEPTH_FILTER_TYPE:
        paraWidgets[PARA_DEPTH_FILTER]->setEnabled(!(value.toInt() == FILTER_CLOSE));
        break;
    default:
        break;
    }
}

void ParaSettingsWidget::onRefreshHdrSetting()
{
    emit hdrSettingsDialog->progressStateChanged(true);

    CAMERA_PARA_ID paraId = PARA_DEPTH_HDR_MODE;

    QVariant value;
    paraWidgets[paraId]->getValue(value);

    cameraPtr->setCameraPara(paraId, value);
}

void ParaSettingsWidget::onUpdateHdrSetting()
{
    emit hdrSettingsDialog->progressStateChanged(true);

    CAMERA_PARA_ID paraId = PARA_DEPTH_HDR_SETTINGS;
    QVariant value;
    paraWidgets[paraId]->getValue(value);
    cameraPtr->setCameraPara(paraId, value);
}

void ParaSettingsWidget::onRoiRectFUpdated(QRectF rect)
{
    roiRectF = rect;
}

void ParaSettingsWidget::onShow3DUpdate(bool show)
{
    ui->roiEditBtn->setEnabled(!show);
    ui->roiLabel->setEnabled(!show);
}

void ParaSettingsWidget::onTranslate()
{
    ui->retranslateUi(this);
    QList<int> refreshIds = { PARA_DEPTH_HDR_MODE , PARA_DEPTH_AUTO_EXPOSURE, PARA_DEPTH_FILTER_TYPE };

    for (auto widget : paraWidgets)
    {
        widget->retranslate("ParaSettingsWidget");

        if (refreshIds.contains(widget->getParaId()))
        {
            QList<QPair<QString, QVariant>> list;
            cameraPtr->getCameraParaItems((CAMERA_PARA_ID)widget->getParaId(), list);

            if (!list.isEmpty())
            {
                paraWidgets[widget->getParaId()]->setItems(list);
            }
        }
    }

    // control button
    ui->previewButton->setToolTip(tr("Start preview"));
    ui->captureSingleButton->setToolTip(tr("Capture single frame"));
    ui->captureMultipleButton->setToolTip(tr("Capture multiple frames"));
    ui->singleShotButton->setToolTip(tr("Single Shot"));
}

void ParaSettingsWidget::onPreviewButtonToggled(bool toggled)
{
    QString text = !toggled ? tr("Start preview") : tr("Pause preview");
    ui->previewButton->setToolTip(text);
}

void ParaSettingsWidget::onPreviewStateChanged(bool toggled)
{
    auto camera = cs::CSApplication::getInstance()->getCamera();
    const int value = camera->getCameraState();
    if (toggled)
    {
        if (value == CAMERA_PAUSED_STREAM)
        {
            emit cs::CSApplication::getInstance()->resumeStream();
        }
        else
        {
            emit cs::CSApplication::getInstance()->startStream();
        }
    }
    else
    {
        emit cs::CSApplication::getInstance()->pausedStream();
    }
}

void ParaSettingsWidget::onClickedRestartCamera()
{
    emit cs::CSApplication::getInstance()->restartCamera();
}

void ParaSettingsWidget::onClickedDisconnCamera()
{
    emit cs::CSApplication::getInstance()->disconnectCamera();
}

void ParaSettingsWidget::onClickCaptureSingle()
{
    qInfo() << "click capture single";
    QUrl url = QFileDialog::getSaveFileUrl(this, tr("Capture frame data"), QDir::currentPath());

    if (url.isValid())
    {
        QFileInfo fileInfo(url.toLocalFile());

        CameraCaptureConfig config;
        config.captureNumber = 1;
        config.captureDataTypes = { CAMERA_DATA_L, CAMERA_DATA_R, CAMERA_DATA_L, CAMERA_DATA_DEPTH, CAMERA_DATA_RGB, CAMERA_DTA_POINT_CLOUD };
        config.saveFormat = QString("Images(*.)");
        config.saveDir = fileInfo.absolutePath();
        config.saveName = fileInfo.fileName();

        cs::CSApplication::getInstance()->startCapture(config);
    }
    else
    {
        qInfo() << "Cancel capture";
    }
}

void ParaSettingsWidget::onClickCaptureMultiple()
{
    qInfo() << "click capture multiple";
    captureSettingDialog->show();
}
