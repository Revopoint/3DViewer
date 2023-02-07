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
#include <QSpacerItem>
#include <QDir>
#include <QDesktopServices>

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
#include "cswidgets/csroieditwidget.h"

#include "hdrsettingsdialog.h"
#include "capturesettingdialog.h"
#include "appconfig.h"
#include "csimagebutton.h"

#define HDR_TABLE_COLS 3
using namespace cs::parameter;

ParaSettingsWidget::ParaSettingsWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ParameterSettingsWidget)
    , cameraPtr(cs::CSApplication::getInstance()->getCamera())
    , captureSettingDialog(new CaptureSettingDialog)
    , paraMonitorThread(new ParaMonitorThread())
{
    setAttribute(Qt::WA_StyledBackground, true);
    ui->setupUi(this);
    initWidget();
    initConnections();

    captureConfig.captureType = CAPTURE_TYPE_SINGLE;
    captureConfig.captureNumber = 1;
    captureConfig.captureDataTypes = { CAMERA_DATA_L, CAMERA_DATA_R, CAMERA_DATA_DEPTH, CAMERA_DATA_RGB, CAMERA_DATA_POINT_CLOUD };
    captureConfig.saveFormat = QString("images");
}

ParaSettingsWidget::~ParaSettingsWidget()
{
    stopParaMonitor();

    delete ui;
    delete captureSettingDialog;
}

void ParaSettingsWidget::initDepthPara()
{
    // stream format
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_STREAM_FORMAT, QT_TR_NOOP("Stream Format"), this));
    // resolution
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_RESOLUTION, QT_TR_NOOP("Resolution"), this));
    // line
    addDepthDividLine();
       
    // depth range
    addDepthParaWidget(new CSRangeEdit(PARA_DEPTH_RANGE, QT_TR_NOOP("Depth Range(mm)"), this));
    // line
    addDepthDividLine();

    // auto exposure
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_AUTO_EXPOSURE, QT_TR_NOOP("Auto Exposure"), this, 
        QT_TR_NOOP("Speed First: The frame rate of this mode is fast, and the image quality may deteriorate.\n"
        "Quality First: This mode has good image quality and may reduce the frame rate.\n"
        "Foreground: This mode gives priority to adjusting the image quality of closer objects,\n"
        "and the image quality of objects farther away may become worse.")));
    // exposure
    addDepthParaWidget(new CSSlider(PARA_DEPTH_EXPOSURE, QT_TR_NOOP("Exposure Time(us)"), this));
    // gain
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_GAIN, QT_TR_NOOP("Gain"), this));
    // line
    addDepthDividLine();

    // Threshold
    addDepthParaWidget(new CSSlider(PARA_DEPTH_THRESHOLD, QT_TR_NOOP("Threshold(Gray Level)"), this));
    // line
    addDepthDividLine();

    // filter type
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_FILTER_TYPE, QT_TR_NOOP("Filter Type")));
    // filter
    addDepthParaWidget(new CSSlider(PARA_DEPTH_FILTER, QT_TR_NOOP("Filter Level"), this));
    // line
    addDepthDividLine();

    // fill hole
    addDepthParaWidget(new CSSwitchButton(PARA_DEPTH_FILL_HOLE, QT_TR_NOOP("Fill Hole"), this));
    // line
    addDepthDividLine();

    // ROI
    addDepthParaWidget(new CSRoiEditWidget(PARA_DEPTH_ROI, QT_TR_NOOP("ROI"), this));
    // line
    addDepthDividLine();

    // HDR settings
    addDepthParaWidget(new CSComboBox(PARA_DEPTH_HDR_MODE, QT_TR_NOOP("HDR Mode"), this));
    addDepthParaWidget(new CSSlider(PARA_DEPTH_HDR_LEVEL, QT_TR_NOOP("HDR Level"), this));
    addDepthParaWidget(new CSTableWidget(PARA_DEPTH_HDR_SETTINGS, HDR_TABLE_COLS, { QT_TR_NOOP("Number"), QT_TR_NOOP("Exposure"), QT_TR_NOOP("Gain")}));
    addHdrButtons();

    hdrButtonArea->setVisible(false);
    paraWidgets[PARA_DEPTH_HDR_SETTINGS]->setVisible(false);
}

void ParaSettingsWidget::addHdrButtons()
{
    hdrButtonArea = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(hdrButtonArea);
    layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->setContentsMargins(0, 0, 20, 0);
    layout->setSpacing(10);

    hdrRefreshButton = new QPushButton(tr("Refresh"), hdrButtonArea);
    hdrConfirmButton = new QPushButton(tr("OK"), hdrButtonArea);
    hdrRefreshButton->setProperty("isCSStyle", true);
    hdrConfirmButton->setProperty("isCSStyle", true);

    layout->addWidget(hdrRefreshButton);
    layout->addWidget(hdrConfirmButton);

    ui->depthParaLayout->addWidget(hdrButtonArea);
    connect(hdrRefreshButton, &QPushButton::clicked, this, &ParaSettingsWidget::onRefreshHdrSetting);
    connect(hdrConfirmButton, &QPushButton::clicked, this, &ParaSettingsWidget::onUpdateHdrSetting);
}

void ParaSettingsWidget::initHDRParaWidgets()
{
    CSParaWidget* widgtes[] = {
        new CSComboBox(PARA_DEPTH_HDR_MODE, QT_TR_NOOP("HDR Mode")),
        new CSComboBox(PARA_DEPTH_HDR_LEVEL, QT_TR_NOOP("HDR Level")),
        new CSTableWidget(PARA_DEPTH_HDR_SETTINGS, HDR_TABLE_COLS, { QT_TR_NOOP("Number"), QT_TR_NOOP("Exposure"), QT_TR_NOOP("Gain")})
    };

    for (auto widget : widgtes)
    {
        auto paraId = widget->getParaId();
        Q_ASSERT(!paraWidgets.contains(paraId));
        paraWidgets[paraId] = widget;
    }
}

void ParaSettingsWidget::initRgbPara()
{
    // stream format
    addRgbParaWidget(new CSComboBox(PARA_RGB_STREAM_FORMAT, QT_TR_NOOP("Stream Format"), this));
    // resolution
    addRgbParaWidget(new CSComboBox(PARA_RGB_RESOLUTION, QT_TR_NOOP("Resolution"), this));
    // line
    addRgbDividLine();

    // auto exposure
    addRgbParaWidget(new CSSwitchButton(PARA_RGB_AUTO_EXPOSURE, QT_TR_NOOP("Auto Exposure"), this));
    // exposure
    addRgbParaWidget(new CSSlider(PARA_RGB_EXPOSURE, QT_TR_NOOP("Exposure Time(us)"), this));
    // gain
    addRgbParaWidget(new CSComboBox(PARA_RGB_GAIN, QT_TR_NOOP("Gain"), this));
    // line
    addRgbDividLine();
 
    // auto white balance
    addRgbParaWidget(new CSSwitchButton(PARA_RGB_AUTO_WHITE_BALANCE, QT_TR_NOOP("Auto White Balance"), this));
    // white balance
    addRgbParaWidget(new CSSlider(PARA_RGB_WHITE_BALANCE, QT_TR_NOOP("White Balance"), this));
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
    // roi
    CSRoiEditWidget* roiWidget = qobject_cast<CSRoiEditWidget*>(paraWidgets[PARA_DEPTH_ROI]);
    if (roiWidget)
    {
        suc &= (bool)connect(roiWidget, &CSRoiEditWidget::clickedFullScreen, this, &ParaSettingsWidget::onClickFullScreenButton);
        suc &= (bool)connect(roiWidget, &CSRoiEditWidget::clickedEditRoi,    this, &ParaSettingsWidget::onClickRoiEditButton);
    }

    Q_ASSERT(suc);
}

void ParaSettingsWidget::initWidget()
{
    ui->depthCameraButton->setProperty("isCameraButton", true);
    ui->rgbCameraButton->setProperty("isCameraButton", true);  
    ui->stackedWidget->setCurrentIndex(PAGE_DEPTH_CAMERA);

    // control button
    ui->previewButton->setEnabled(false);
    ui->captureSingleButton->setEnabled(false);
    ui->captureMultipleButton->setEnabled(false);
    ui->singleShotButton->setEnabled(false);
    ui->stopPreviewButton->setEnabled(false);

    initTopButton();

    //parameter widget
    initDepthPara();
    initRgbPara();

    ui->depthCameraButton->setChecked(true);
    ui->scrollArea->setEnabled(false);
}

void ParaSettingsWidget::initConnections()
{
    bool suc = true;

    suc &= (bool)connect(ui->depthCameraButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickDepthButton);
    suc &= (bool)connect(ui->rgbCameraButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickRgbButton);
    suc &= (bool)connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &ParaSettingsWidget::onPageChanged);

    suc &= (bool)connect(ui->previewButton, &QPushButton::clicked, this, &ParaSettingsWidget::onPreviewStateChanged);
    suc &= (bool)connect(ui->previewButton, &QPushButton::toggled, this, &ParaSettingsWidget::onPreviewButtonToggled);
    suc &= (bool)connect(ui->stopPreviewButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickedStopStream);
    
    suc &= (bool)connect(ui->captureSingleButton,    &QPushButton::clicked,  this, &ParaSettingsWidget::onClickCaptureSingle);
    suc &= (bool)connect(ui->captureMultipleButton, &QPushButton::clicked,  this, &ParaSettingsWidget::onClickCaptureMultiple);

    suc &= (bool)connect(ui->singleShotButton, &QPushButton::clicked, this, &ParaSettingsWidget::onClickSingleShot);

    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app, &cs::CSApplication::cameraStateChanged, this, &ParaSettingsWidget::onCameraStateChanged);
    suc &= (bool)connect(app, &cs::CSApplication::captureStateChanged, this, &ParaSettingsWidget::onCaptureStateChanged);
    suc &= (bool)connect(this, &ParaSettingsWidget::translateSignal, this, &ParaSettingsWidget::onTranslate);

    Q_ASSERT(suc);

    // connections
    initParaConnections();
}

void ParaSettingsWidget::initTopButton()
{
    QIcon icon;
    QSize size(10, 10);

    icon.addFile(QStringLiteral(":/resources/double_arrow_down.png"), size, QIcon::Normal, QIcon::Off);
    icon.addFile(QStringLiteral(":/resources/double_arrow_left.png"), size, QIcon::Selected, QIcon::On);
    topItemButton = new CSTextImageButton(icon, QT_TR_NOOP("Parameter Settings"), Qt::LeftToRight, ui->cameraTopItem);

    auto* layout = ui->cameraTopItem->layout();
    if (layout)
    {
        layout->addWidget(topItemButton);
    }

    connect(topItemButton, &CSTextImageButton::toggled, [=](bool checked)
        {
            if (checked)
            {
                ui->scrollArea->hide();
                ui->cameraButtonArea->hide();
                verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

                int idx = ui->verticalLayout->indexOf(ui->bottomControlArea);
                ui->verticalLayout->insertItem(idx, verticalSpacer);
            }
            else
            {
                ui->verticalLayout->removeItem(verticalSpacer);
                verticalSpacer = nullptr;
                ui->scrollArea->show();
                ui->cameraButtonArea->show();
            }
        });
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

void ParaSettingsWidget::onClickSingleShot()
{
    auto camera = cs::CSApplication::getInstance()->getCamera();

    //QVariant value;
    //camera->getCameraPara(PARA_TRIGGER_MODE, value);
    //if (value.toInt() == TRIGGER_MODE_OFF)
    if(!isSingleShotMode)
    {
        const int triggerMode = TRIGGER_MODE_SOFTWAER;
        camera->setCameraPara(PARA_TRIGGER_MODE, triggerMode);

        ui->previewButton->setChecked(false);
        ui->captureMultipleButton->setEnabled(false);
        isSingleShotMode = true;
        emit showMessage(tr("Entered single shot mode! You can click the button to get the next frame."), 5000);
    }
    else
    {
        camera->softTrigger();
    }
}

void ParaSettingsWidget::onClickRoiEditButton()
{
    QVariant value;
    cameraPtr->getCameraPara(PARA_DEPTH_ROI, value);
    QRectF rect = value.toRectF();

    emit roiStateChanged(true, rect);
}

void ParaSettingsWidget::onClickFullScreenButton()
{
    QRectF rect(0.0, 0.0, 1.0,1.0);
    QVariant value = QVariant::fromValue<QRectF>(rect);

    cameraPtr->setCameraPara(PARA_DEPTH_ROI, value);

    emit roiStateChanged(true, rect);
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
        isSingleShotMode = false;

        startParaMonitor();
        break;
    case CAMERA_CONNECTED:
        {
            ui->cameraButtonArea->setEnabled(true);
            ui->scrollArea->setEnabled(true);
            onUpdatedCameraParas();
            isSingleShotMode = false;
            break;
        }
    case CAMERA_DISCONNECTED:
    case CAMERA_DISCONNECTFAILED:
    case CAMERA_CONNECTFAILED:
        ui->cameraButtonArea->setEnabled(false);
        ui->scrollArea->setEnabled(false);
        isSingleShotMode = false;
        stopParaMonitor();
        break;
    default:    
        stopParaMonitor();
        ui->previewButton->setChecked(false);
        break;
    }
}

void ParaSettingsWidget::updateControlButtonState(int cameraState)
{
    bool enable = (cameraState == CAMERA_CONNECTED || cameraState == CAMERA_STARTED_STREAM || cameraState == CAMERA_PAUSED_STREAM
        || cameraState == CAMERA_STOPPED_STREAM);

    ui->previewButton->setEnabled(enable);

    enable = (cameraState == CAMERA_STARTED_STREAM);
    ui->captureSingleButton->setEnabled(enable);
    ui->captureMultipleButton->setEnabled(enable);
    ui->singleShotButton->setEnabled(enable);

    ui->stopPreviewButton->setEnabled((cameraState == CAMERA_STARTED_STREAM) || (cameraState == CAMERA_PAUSED_STREAM));
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

    if (cameraState == CAMERA_STARTED_STREAM)
    {
        QVariant hasDepthV;
        cameraPtr->getCameraPara(cs::parameter::PARA_HAS_DEPTH, hasDepthV);
        bool hasDepth = hasDepthV.toBool();

        paraWidgets[PARA_DEPTH_RANGE]->setEnabled(hasDepth);
    }

    if (cameraState == CAMERA_DISCONNECTFAILED || cameraState == CAMERA_DISCONNECTED || cameraState == CAMERA_CONNECTFAILED)
    {
        // clear parameter values
        for (auto widget : paraWidgets)
        {
            widget->clearValues();
        }
    }
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

    }
}

// camera para updated
void ParaSettingsWidget::onCameraParaUpdated(int paraId, QVariant value)
{
    if (paraWidgets.contains(paraId))
    {
        if (!value.isValid())
        {
            return;
        }

        paraWidgets[paraId]->setValue(value);
        onParaLinkResponse(paraId, value);

        if (paraId == PARA_DEPTH_ROI)
        {
            QRectF rect = value.toRectF();

            QString str = QString("[%1, %2, %3, %4]").arg(QString::number(rect.left(), 'f', 3))
                .arg(QString::number(rect.top(), 'f', 3))
                .arg(QString::number(rect.right(), 'f', 3))
                .arg(QString::number(rect.bottom(), 'f', 3));

            emit showMessage(QString(tr("Set ROI ")) + str);
        }
    }
    else if (paraId == PARA_DEPTH_FRAMETIME || paraId == PARA_CAMERA_IP)
    {
        
    }
    else if (paraId == PARA_TRIGGER_MODE)
    {
        if (value.toInt() == TRIGGER_MODE_SOFTWAER)
        {
           // ui->previewButton->setChecked(false);
        }
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
    switch (paraId)
    {
    case PARA_DEPTH_AUTO_EXPOSURE:
        {        
            bool enable = !(value.toInt() > 0);
            paraWidgets[PARA_DEPTH_GAIN]->setEnabled(enable);
            paraWidgets[PARA_DEPTH_EXPOSURE]->setEnabled(enable);

            paraMonitorThread->setAutoExposureDepth(!enable);
        }
        break;
    case PARA_RGB_AUTO_EXPOSURE:
        paraWidgets[PARA_RGB_EXPOSURE]->setEnabled(!value.toBool());
        paraWidgets[PARA_RGB_GAIN]->setEnabled(!value.toBool());

        paraMonitorThread->setAutoExposureRgb(value.toBool());
        break;
    case PARA_RGB_AUTO_WHITE_BALANCE:
        paraWidgets[PARA_RGB_WHITE_BALANCE]->setEnabled(!value.toBool());

        paraMonitorThread->setAutoWhiteBalance(value.toBool());

        break;
    case PARA_DEPTH_HDR_MODE: {
        bool isClosed = (value.toInt() == HDR_MODE_CLOSE);
        hdrButtonArea->setVisible(!isClosed);

        paraWidgets[PARA_DEPTH_HDR_SETTINGS]->setVisible(!isClosed); 
        paraWidgets[PARA_DEPTH_HDR_LEVEL]->setEnabled(!isClosed);

        if (value.toInt() == HDR_MODE_MANUAL)
        {
            hdrConfirmButton->setEnabled(true);
        }
        else 
        {
            hdrConfirmButton->setEnabled(false);
        }
        break;
    }
    case PARA_DEPTH_FILTER_TYPE:
        paraWidgets[PARA_DEPTH_FILTER]->setEnabled(!(value.toInt() == FILTER_CLOSE));
        break;
    default:
        break;
    }
}

void ParaSettingsWidget::onRefreshHdrSetting()
{
    CAMERA_PARA_ID paraId = PARA_DEPTH_HDR_MODE;

    QVariant value;
    paraWidgets[paraId]->getValue(value);

    cameraPtr->setCameraPara(paraId, value);
}

void ParaSettingsWidget::onUpdateHdrSetting()
{
    CAMERA_PARA_ID paraId = PARA_DEPTH_HDR_SETTINGS;
    QVariant value;
    paraWidgets[paraId]->getValue(value);
    cameraPtr->setCameraPara(paraId, value);
}

void ParaSettingsWidget::onRoiRectFUpdated(QRectF rect)
{
    cameraPtr->setCameraPara(PARA_DEPTH_ROI, rect);
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

    topItemButton->retranslate("ParaSettingsWidget");
    // control button
    ui->previewButton->setToolTip(tr("Start preview"));
    ui->captureSingleButton->setToolTip(tr("Capture single frame"));
    ui->captureMultipleButton->setToolTip(tr("Capture multiple frames"));
    ui->singleShotButton->setToolTip(tr("Single Shot"));
    ui->stopPreviewButton->setToolTip(tr("Stop preview"));

    hdrRefreshButton->setText(tr("Refresh"));
    hdrConfirmButton->setText(tr("OK"));
       
    captureSettingDialog->onTranslate();
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
        if (isSingleShotMode)
        {
            camera->setCameraPara(PARA_TRIGGER_MODE, TRIGGER_MODE_OFF);
            isSingleShotMode = false;
            ui->captureMultipleButton->setEnabled(true);
        }
        else if (value == CAMERA_PAUSED_STREAM)
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

void ParaSettingsWidget::onClickedStopStream()
{
    if (isSingleShotMode)
    {
        auto camera = cs::CSApplication::getInstance()->getCamera();

        camera->setCameraPara(PARA_TRIGGER_MODE, TRIGGER_MODE_OFF);
        isSingleShotMode = false;
        ui->captureMultipleButton->setEnabled(true);
    }

    stopParaMonitor();
    emit cs::CSApplication::getInstance()->stopStream();
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
    auto config = cs::CSApplication::getInstance()->getAppConfig();
    bool autoName = config->getAutoNameWhenCapturing();
    QString openDir = config->getDefaultSavePath();

    // set current output data
    cs::CSApplication::getInstance()->setCurOutputData(captureConfig);
    
    if (autoName)
    {
        QDateTime now = QDateTime::currentDateTime();
        QString time = now.toString("yyyyMMddHHmmsszzz");

        captureConfig.saveDir = openDir;
        captureConfig.saveName = time;
        captureConfig.savePointCloudWithTexture = cs::CSApplication::getInstance()->getShow3DTexture();

        cs::CSApplication::getInstance()->startCapture(captureConfig, true);
    }
    else 
    {
        qInfo() << "click capture single";
        QUrl url = QFileDialog::getSaveFileUrl(this, tr("Capture frame data"), openDir);

        if (url.isValid())
        {
            QFileInfo fileInfo(url.toLocalFile());

            captureConfig.saveDir = fileInfo.absolutePath();
            captureConfig.saveName = fileInfo.fileName();
            captureConfig.savePointCloudWithTexture = cs::CSApplication::getInstance()->getShow3DTexture();

            cs::CSApplication::getInstance()->startCapture(captureConfig, false);
        }
        else
        {
            qInfo() << "Cancel capture";
        }
    }
}

void ParaSettingsWidget::onClickCaptureMultiple()
{
    qInfo() << "click capture multiple";
    captureSettingDialog->show();
}


void ParaSettingsWidget::startParaMonitor()
{
    paraMonitorThread->start();
}

void ParaSettingsWidget::stopParaMonitor()
{
    paraMonitorThread->requestInterruption();
    paraMonitorThread->wait();
}

void ParaSettingsWidget::onCaptureStateChanged(int captureType, int state, QString message)
{
    if (captureType != CAPTURE_TYPE_SINGLE)
    {
        return;
    }

    QString dir = QString("file:///%1").arg(captureConfig.saveDir);
    QDesktopServices::openUrl(QUrl(dir));
}

ParaMonitorThread::ParaMonitorThread()
    :cameraPtr(cs::CSApplication::getInstance()->getCamera())
{

}

void ParaMonitorThread::run()
{
    const int interval = 100;
    int count = 0;

    while (!isInterruptionRequested())
    {
        QThread::msleep(interval);
        if (count >= 30)
        {
            if (enableAutoExposureDepth)
            {
                QVariant value;
                cameraPtr->getCameraPara(PARA_DEPTH_EXPOSURE, value);
                emit cameraPtr->cameraParaUpdated(PARA_DEPTH_EXPOSURE, value);

                cameraPtr->getCameraPara(PARA_DEPTH_GAIN, value);
                emit cameraPtr->cameraParaUpdated(PARA_DEPTH_GAIN, value);
            }

            if (enableAutoExposureRgb)
            {
                QVariant value;
                cameraPtr->getCameraPara(PARA_RGB_GAIN, value);
                emit cameraPtr->cameraParaUpdated(PARA_RGB_GAIN, value);

                cameraPtr->getCameraPara(PARA_RGB_EXPOSURE, value);
                emit cameraPtr->cameraParaUpdated(PARA_RGB_EXPOSURE, value);
            }

            if (enableAutoWhiteBalance)
            {
                QVariant value;
                cameraPtr->getCameraPara(PARA_RGB_WHITE_BALANCE, value);
                emit cameraPtr->cameraParaUpdated(PARA_RGB_WHITE_BALANCE, value);
            }

            count = 0;
        }

        count++;
    }
}

void ParaMonitorThread::setAutoExposureDepth(bool enable)
{
    enableAutoExposureDepth = enable;
}

void ParaMonitorThread::setAutoExposureRgb(bool enable)
{
    enableAutoExposureRgb = enable;
}

void ParaMonitorThread::setAutoWhiteBalance(bool enable)
{
    enableAutoWhiteBalance = enable;
}
