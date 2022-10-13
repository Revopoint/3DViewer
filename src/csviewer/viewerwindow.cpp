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

#include "viewerwindow.h"
#include "./ui_viewerwindow.h"

#include <QFile>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTranslator>
#include <QDesktopServices>
#include <QThread>

#include <cstypes.h>
#include <icscamera.h>

#include "csapplication.h"
#include "renderwidget2d.h"
#include "./app_version.h"
#include "camerainfodialog.h"
#include "csprogressbar.h"
#include "csmessagebox.h"

#define GITHUB_URL "https://github.com/Revopoint/3DViewer"
#define WEBSITE_URL "https://www.revopoint3d.com/"

ViewerWindow::ViewerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ViewerWindow)
    , language(LANGUAGE_EN)
    , translator(new QTranslator(this))
    , cameraInfoDialog(new CameraInfoDialog())
    , circleProgressBar(new CSProgressBar(this))
    , globalMessageBox(new CSMessageBox(this))
{
    // init translator
    qApp->installTranslator(translator);
    QString qmFile = ( language == LANGUAGE_ZH ) ? "lang_zh.qm" : "lang_en.qm";
    qmFile = QString("%1/translations/%2").arg(QApplication::applicationDirPath()).arg(qmFile);
    if (!translator->load(QDir::cleanPath(qmFile)))
    {
        qDebug() << "load qm failed : " << qmFile;
    }

    // set ui
    ui->setupUi(this);
    // init widgets
    initWindow();
    // set up signal connections
    initConnections();

    render2dWidgets[CAMERA_DATA_L] = nullptr;
    render2dWidgets[CAMERA_DATA_R] = nullptr;
    render2dWidgets[CAMERA_DATA_DEPTH] = nullptr;
    render2dWidgets[CAMERA_DATA_RGB] = nullptr;

    emit translateSignal();
}

ViewerWindow::~ViewerWindow()
{
    qApp->removeTranslator(translator);
    delete ui;
    delete cameraInfoDialog;
}

void ViewerWindow::initConnections()
{
    bool suc = true;

    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app,  &cs::CSApplication::cameraStateChanged,   this, &ViewerWindow::onCameraStateChanged);
    suc &= (bool)connect(app,  &cs::CSApplication::output3DUpdated,      this, &ViewerWindow::onOutput3DUpdated);
    suc &= (bool)connect(app,  &cs::CSApplication::removedCurrentCamera, this, &ViewerWindow::onRemovedCurrentCamera);
    suc &= (bool)connect(app, &cs::CSApplication::captureStateChanged,   this, &ViewerWindow::onCaptureStateChanged);

    suc &= (bool)connect(qApp, &QApplication::aboutToQuit,               this, &ViewerWindow::onAboutToQuit);
    suc &= (bool)connect(this, &ViewerWindow::show3DUpdated,             app,  &cs::CSApplication::onShow3DUpdated);

    suc &= (bool)connect(ui->renderWidget3D,    &RenderWidget3D::show3DTextureChanged,   app,  &cs::CSApplication::onShow3DTextureChanged);

    suc &= (bool)connect(ui->modeSwitchButton,  &QPushButton::clicked,                   this, &ViewerWindow::onClickedModeSwitchButton);
    suc &= (bool)connect(ui->cameraListWidget,  &CameraListWidget::clickedCloseButton,   this, &ViewerWindow::onClickedModeSwitchButton);
    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::clickedCloseButton, this, &ViewerWindow::onClickedModeSwitchButton);
    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::showMessage,        this, &ViewerWindow::onShowStatusBarMessage);

    suc &= (bool)connect(ui->stackedWidget,     &QStackedWidget::currentChanged,         this, &ViewerWindow::onPageChanged);
    suc &= (bool)connect(ui->tabWidget,         &QTabWidget::currentChanged,             this, &ViewerWindow::onRenderPageChanged);
    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::roiStateChanged,    this, &ViewerWindow::onRoiEditStateChanged);

    suc &= (bool)connect(this, &ViewerWindow::translateSignal,   this,                   &ViewerWindow::onTranslate,            Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::renderInitialized, this,                   &ViewerWindow::onRenderInitialized,    Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,   ui->cameraListWidget,   &CameraListWidget::translateSignal,    Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,   ui->paraSettingWidget,  &ParaSettingsWidget::translateSignal,  Qt::QueuedConnection);

    // menu
    suc &= (bool)connect(ui->menuLanguage,            &QMenu::triggered,     this, &ViewerWindow::onUpdateLanguage);
    suc &= (bool)connect(ui->menuAbout,               &QMenu::triggered,     this, &ViewerWindow::onTriggeredAbout);
    suc &= (bool)connect(ui->actionRestartCamera,     &QAction::triggered,   this, &ViewerWindow::onTriggeredRestartCamera);
    suc &= (bool)connect(ui->actionExit,              &QAction::triggered,   this, &ViewerWindow::onAppExit);
    suc &= (bool)connect(ui->actionInfomation,        &QAction::triggered,   this, &ViewerWindow::onTriggeredInformation);
    suc &= (bool)connect(ui->actionOpenLogDir,        &QAction::triggered,   this, &ViewerWindow::onTriggeredLogDir);
  
    Q_ASSERT(suc);      
}

void ViewerWindow::initWindow()
{
    setWindowIcon(QIcon(":/resources/3DViewer.ico"));
    QString title = QString("%1 %2").arg(APP_NAME).arg(APP_VERSION);
    setWindowTitle(title);
    showMaximized();

    ui->actionEnglish->setChecked(language == LANGUAGE_EN);
    ui->actionChinese->setChecked(language == LANGUAGE_ZH);

    //set page
    ui->stackedWidget->setCurrentIndex(CAMERALIST_PAGE);

    //disable 3d tab
    ui->tabWidget->setTabEnabled(RENDER_PAGE_3D, false);

    QString defaultSavePath = tr("Set default save path (") + QDir::homePath() + ")";
    ui->actionsetDefaultSaveDir->setText(defaultSavePath);
}

void ViewerWindow::onClickedModeSwitchButton()
{
    const int curIdx = ui->stackedWidget->currentIndex();
    
    int idx = curIdx + 1;
    if (idx >= PAGE_COUNT)
    {
        idx = 0;
    }

    ui->stackedWidget->setCurrentIndex(idx);
}

void ViewerWindow::onPageChanged(int idx)
{
    STACK_WIDGET_PAGE_ID curPage = (STACK_WIDGET_PAGE_ID)idx;

    switch (curPage)
    {
    case PARASETTING_PAGE:
        ui->modeSwitchButton->setText(tr("Camera List"));
        break;
    case CAMERALIST_PAGE:
        ui->modeSwitchButton->setText(tr("Parameter Settings"));
        emit cs::CSApplication::getInstance()->queryCameras();
        break;
    default:
        qWarning() << "invalid page index.";
        break;
    }
}

// switch 2D/3D
void ViewerWindow::onRenderPageChanged(int idx)
{
    RENDER_PAGE_ID pageId = (RENDER_PAGE_ID)idx;
    ui->paraSettingWidget->onShow3DUpdate(pageId == RENDER_PAGE_3D);

    emit show3DUpdated(pageId == RENDER_PAGE_3D);
}

void ViewerWindow::onCameraStateChanged(int state)
{
    CAMERA_STATE cameraState = (CAMERA_STATE)state;
    switch (cameraState)
    {
    case CAMERA_CONNECTING:
    case CAMERA_RESTARTING_CAMERA:
        ui->menuCamera->setEnabled(false);
    case CAMERA_STARTING_STREAM:
    case CAMERA_STOPPING_STREAM:
    case CAMERA_DISCONNECTING:
        circleProgressBar->open();
        break;
    
    case CAMERA_CONNECTED:
    {
        ui->stackedWidget->setCurrentIndex(PARASETTING_PAGE);
        ui->menuCamera->setEnabled(true);

        // update camera info dialog
        CSCameraInfo info = cs::CSApplication::getInstance()->getCamera()->getCameraInfo();
        cameraInfoDialog->updateCameraInfo(info);

        circleProgressBar->close();
        destoryRender2dWidgets();

        break;
    }
    case CAMERA_DISCONNECTED:
    case CAMERA_DISCONNECTFAILED:
        destoryRender2dWidgets();
    case CAMERA_CONNECTFAILED:
        circleProgressBar->close();
        ui->menuCamera->setEnabled(false);
        break;
    case CAMERA_STARTED_STREAM:
        initRenderWidgets();
    case CAMERA_STOPPED_STREAM:
        circleProgressBar->close();
        break;
    default:    
        break;
    }

    updateStatusBar(state);
}

// update the prompt of bottom status bar when the camera status changed
void ViewerWindow::updateStatusBar(int state)
{
    CAMERA_STATE cameraState = (CAMERA_STATE)state;
    
    const int timeoutMS = 5000;

    switch (cameraState)
    {
    case CAMERA_CONNECTING:
        ui->statusBar->showMessage(tr("Connecting camera"), timeoutMS);
        break;
    case CAMERA_CONNECTED:
        ui->statusBar->showMessage(tr("Camera connected successfully"), timeoutMS);
        break;
    case CAMERA_CONNECTFAILED:
        ui->statusBar->showMessage(tr("Camera connection failed"), timeoutMS);
        break;
    case CAMERA_DISCONNECTING:
        ui->statusBar->showMessage(tr("Disconnecting camera"), timeoutMS);
        break;
    case CAMERA_DISCONNECTED:
        ui->statusBar->showMessage(tr("Camera disconnected successfully"), timeoutMS);
        break;
    case CAMERA_DISCONNECTFAILED:
        ui->statusBar->showMessage(tr("Camera disconnection failed"), timeoutMS);
        break;
    case CAMERA_STARTING_STREAM:
        ui->statusBar->showMessage(tr("Starting preview"), timeoutMS);
        break;
    case CAMERA_STARTED_STREAM:
        ui->statusBar->showMessage(tr("Start preview successfully"), timeoutMS);
        break;
    case CAMERA_PAUSING_STREAM:
        ui->statusBar->showMessage(tr("Pausing preview"), timeoutMS);
        break;
    case CAMERA_PAUSED_STREAM:
        ui->statusBar->showMessage(tr("Preview has been paused"), timeoutMS);
        break;
    case CAMERA_RESTARTING_CAMERA:
        ui->statusBar->showMessage(tr("Restarting camera"));
        break;    
    case CAMERA_STOPPING_STREAM:
        ui->statusBar->showMessage(tr("Stopping preview"), timeoutMS);
        break;
    case CAMERA_STOPPED_STREAM:
        ui->statusBar->showMessage(tr("Preview has been stopped"), timeoutMS);
        break;
    default:
        break;
    }
}

// render in UI thread
void ViewerWindow::onOutput2DUpdated(OutputData2D outputData)
{
    RenderWidget2D* widget = render2dWidgets[outputData.info.cameraDataType];
    if (widget)
    {
        widget->onRenderDataUpdated(outputData);
    }
}

void ViewerWindow::onOutput3DUpdated(cs::Pointcloud pointCloud, const QImage& image)
{
    ui->renderWidget3D->onRenderDataUpdated(pointCloud, image);
}

void ViewerWindow::destoryRender2dWidgets()
{
    disconnect(cs::CSApplication::getInstance(), &cs::CSApplication::output2DUpdated, this, &ViewerWindow::onOutput2DUpdated);

    for (auto key : render2dWidgets.keys())
    {
        if (render2dWidgets[key])
        {
            ui->render2dTopLayout->removeWidget(render2dWidgets[key]);
            ui->render2dBottomLayout->removeWidget(render2dWidgets[key]);

            delete render2dWidgets[key];
            render2dWidgets[key] = nullptr;
        }
    }
}

void ViewerWindow::initRenderConnections()
{
    bool suc = true;
    for (auto key : render2dWidgets.keys())
    {
        auto renderWidget = render2dWidgets[key];
        if (renderWidget)
        {
            if (key == CAMERA_DATA_DEPTH)
            {        
                suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(renderWidget), &DepthRenderWidget2D::roiRectFUpdated,  this, &ViewerWindow::onRoiRectFUpdated);
                suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(renderWidget), &DepthRenderWidget2D::showCoordChanged, cs::CSApplication::getInstance(), &cs::CSApplication::onShowCoordChanged);
            }
        }
    }

    suc &= (bool)connect(cs::CSApplication::getInstance(), &cs::CSApplication::output2DUpdated, this, &ViewerWindow::onOutput2DUpdated);
    Q_ASSERT(suc);
}

// exec in UI thread
void ViewerWindow::initRenderWidgets()
{
    // remove widget from layout
    destoryRender2dWidgets();

    // get camera parameter
    auto camera = cs::CSApplication::getInstance()->getCamera();
    QVariant hasRgbV, hasIrV, hasDepthV;
    camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    camera->getCameraPara(cs::parameter::PARA_DEPTH_HAS_LR, hasIrV);
    camera->getCameraPara(cs::parameter::PARA_HAS_DEPTH, hasDepthV);

    const bool hasRgb = hasRgbV.toBool();
    const bool hasIr = hasIrV.toBool();
    const bool hasDepth = hasDepthV.toBool();

    // has ir data
    if (hasIr)
    {
        render2dWidgets[CAMERA_DATA_L] = new RenderWidget2D(CAMERA_DATA_L, this);
        render2dWidgets[CAMERA_DATA_R] = new RenderWidget2D(CAMERA_DATA_R, this);

        // add to layout
        ui->render2dTopLayout->addWidget(render2dWidgets[CAMERA_DATA_L]);
        ui->render2dTopLayout->addWidget(render2dWidgets[CAMERA_DATA_R]);
    }

    // has depth data
    if (hasDepth)
    {
        render2dWidgets[CAMERA_DATA_DEPTH] = new DepthRenderWidget2D(CAMERA_DATA_DEPTH, this);
        ui->render2dBottomLayout->addWidget(render2dWidgets[CAMERA_DATA_DEPTH]);
    }

    // has RGB data 
    if (hasRgb)
    {
        render2dWidgets[CAMERA_DATA_RGB] = new RenderWidget2D(CAMERA_DATA_RGB, this);
        ui->render2dBottomLayout->addWidget(render2dWidgets[CAMERA_DATA_RGB]);
    }

    // make connections after render widget initialized
    emit renderInitialized();

    // update ui state
    ui->tabWidget->setTabEnabled(RENDER_PAGE_3D, hasDepth);
    ui->renderWidget3D->setEnabled(hasDepth);
    // enable texture when has rgb
    ui->renderWidget3D->setTextureEnable(hasRgb);
}

void ViewerWindow::onRenderInitialized()
{
    auto camera = cs::CSApplication::getInstance()->getCamera();
    // depth resolution
    QVariant depthRes;
    camera->getCameraPara(cs::parameter::PARA_DEPTH_RESOLUTION, depthRes);
    auto depthRatio  = depthRes.toSize().width() * 1.0f / depthRes.toSize().height();

    // set (width/height) ratio
    if (render2dWidgets[CAMERA_DATA_L])
    {
        render2dWidgets[CAMERA_DATA_L]->setWHRatio(depthRatio);
        render2dWidgets[CAMERA_DATA_R]->setWHRatio(depthRatio);
    }

    if (render2dWidgets[CAMERA_DATA_DEPTH])
    {
        render2dWidgets[CAMERA_DATA_DEPTH]->setWHRatio(depthRatio);
    }

    if (render2dWidgets[CAMERA_DATA_RGB])
    {
        // set (width/height) ratio
        QVariant rgbRes;
        camera->getCameraPara(cs::parameter::PARA_RGB_RESOLUTION, rgbRes);
        auto res = rgbRes.toSize();
        render2dWidgets[CAMERA_DATA_RGB]->setWHRatio(res.width() * 1.0 / res.height());
    }

    // set up signal connections
    initRenderConnections();
}

void ViewerWindow::onRoiEditStateChanged(bool edit)
{
    DepthRenderWidget2D* widget = qobject_cast<DepthRenderWidget2D*>(render2dWidgets[CAMERA_DATA_DEPTH]);;
    if (widget)
    {
        widget->onRoiEditStateChanged(edit);
    }
}

void ViewerWindow::onRoiRectFUpdated(QRectF rect)
{
    ui->paraSettingWidget->onRoiRectFUpdated(rect);
}

void ViewerWindow::onUpdateLanguage(QAction* action)
{
    if (action == ui->actionChinese)
    {
        ui->actionEnglish->setChecked(false);
        language = LANGUAGE_ZH;
    }
    else
    {
        ui->actionChinese->setChecked(false);
        language = LANGUAGE_EN;
    }

    onLanguageChanged();
    action->setChecked(true);
}

void ViewerWindow::onTriggeredAbout(QAction* action)
{
    if (action == ui->actionGithub)
    {
        QDesktopServices::openUrl(QUrl(GITHUB_URL));
    }
    else if (action == ui->actionWebSite)
    {
        QDesktopServices::openUrl(QUrl(WEBSITE_URL));
    } 
}

void ViewerWindow::onTriggeredRestartCamera()
{
    emit cs::CSApplication::getInstance()->restartCamera();
}

void ViewerWindow::onTriggeredInformation()
{
    cameraInfoDialog->show();
}

void ViewerWindow::onTriggeredLogDir()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(LOG_ROOT_DIR));
}

void ViewerWindow::onAppExit()
{
    qApp->exit();
}

void ViewerWindow::onAboutToQuit()
{
    cs::CSApplication::getInstance()->getCamera()->disconnectCamera();
}

void ViewerWindow::onLanguageChanged()
{
    QString qmFile;
    switch (language)
    {
    case LANGUAGE_ZH:
        qmFile = "lang_zh.qm";
        break;
    case LANGUAGE_EN:
        qmFile = "lang_en.qm";
        break;
    default:
        qmFile = "lang_en.qm";
        break;
    }

    qmFile = QString("%1/translations/%2").arg(QApplication::applicationDirPath()).arg(qmFile);      
    if (!translator->load(QDir::cleanPath(qmFile)))
    {
        qDebug() << "load qm failed : " << qmFile;
        return;
    }

    emit translateSignal();
}

void ViewerWindow::onTranslate()
{
    ui->retranslateUi(this);
    cameraInfoDialog->onTranslate();
    globalMessageBox->retranslate();
    ui->renderWidget3D->onTranslate();

    for (auto widget : render2dWidgets)
    {
        if (widget)
        {
            widget->onTranslate();
        }
    }
}

void ViewerWindow::onRemovedCurrentCamera(QString serial)
{
    globalMessageBox->updateMessage(tr("The current camera has been disconnected"));
    globalMessageBox->open();

    connect(globalMessageBox, &CSMessageBox::accepted, this, &ViewerWindow::onConfirmCameraRemoved, Qt::UniqueConnection);
}

void ViewerWindow::onConfirmCameraRemoved()
{
    disconnect(globalMessageBox, &CSMessageBox::accepted, this, &ViewerWindow::onConfirmCameraRemoved);
    ui->stackedWidget->setCurrentIndex(CAMERALIST_PAGE);
}

void ViewerWindow::onShowStatusBarMessage(QString msg, int timeout)
{
    if (msg.isEmpty())
    {
        ui->statusBar->clearMessage();
    }
    else 
    {
        ui->statusBar->showMessage(msg, timeout);
    }
}

void ViewerWindow::onCaptureStateChanged(int state, QString message)
{
    onShowStatusBarMessage(message, 5000);
}
