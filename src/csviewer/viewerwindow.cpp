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
#include "renderwidget.h"
#include "./app_version.h"
#include "camerainfodialog.h"
#include "csprogressbar.h"
#include "csmessagebox.h"
#include "appconfig.h"
#include "csaction.h"

#define GITHUB_URL "https://github.com/Revopoint/3DViewer"
#define WEBSITE_URL "https://www.revopoint3d.com/"

ViewerWindow::ViewerWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ViewerWindow)
    , translator(new QTranslator(this))
    , cameraInfoDialog(new CameraInfoDialog())
    , circleProgressBar(new CSProgressBar(this))
    , globalMessageBox(new CSMessageBox(this))
{
    // init translator
    qApp->installTranslator(translator);

    // set ui
    ui->setupUi(this);
    // init widgets
    initWindow();
    // set up signal connections
    initConnections();

    // set language
    language = (cs::CSApplication::getInstance()->getAppConfig()->getLanguage() == "zh") ? LANGUAGE_ZH : LANGUAGE_EN;

    ui->actionEnglish->setChecked(language == LANGUAGE_EN);
    ui->actionChinese->setChecked(language == LANGUAGE_ZH);

    onLanguageChanged();

    onCameraStreamStarted();
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
    suc &= (bool)connect(app,  &cs::CSApplication::removedCurrentCamera, this, &ViewerWindow::onRemovedCurrentCamera);
    suc &= (bool)connect(app, &cs::CSApplication::captureStateChanged,   this, &ViewerWindow::onCaptureStateChanged);

    suc &= (bool)connect(qApp, &QApplication::aboutToQuit,               this, &ViewerWindow::onAboutToQuit);
    suc &= (bool)connect(this, &ViewerWindow::windowLayoutChanged,       app,  &cs::CSApplication::onWindowLayoutChanged, Qt::QueuedConnection);

    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::showMessage,        this, &ViewerWindow::onShowStatusBarMessage);

    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::roiStateChanged,    this, &ViewerWindow::onRoiEditStateChanged);

    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     this,                   &ViewerWindow::onTranslate,            Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     ui->cameraListWidget,   &CameraListWidget::translateSignal,    Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     ui->paraSettingWidget,  &ParaSettingsWidget::translateSignal,  Qt::QueuedConnection);

    suc &= (bool)connect(this, &ViewerWindow::renderWindowUpdated,           ui->renderWindow,   &RenderWindow::onRenderWindowsUpdated);
    suc &= (bool)connect(this, &ViewerWindow::windowLayoutModeUpdated,       ui->renderWindow,   &RenderWindow::onWindowLayoutModeUpdated);
    suc &= (bool)connect(ui->renderWindow, &RenderWindow::roiRectFUpdated,   this,               &ViewerWindow::onRoiRectFUpdated);
    suc &= (bool)connect(ui->renderWindow, &RenderWindow::renderExit,        this,               &ViewerWindow::onRenderExit, Qt::QueuedConnection);

    // menu
    suc &= (bool)connect(ui->menuLanguage,            &QMenu::triggered,     this, &ViewerWindow::onUpdateLanguage);
    suc &= (bool)connect(ui->menuAbout,               &QMenu::triggered,     this, &ViewerWindow::onTriggeredAbout);
    suc &= (bool)connect(ui->actionRestartCamera,     &QAction::triggered,   this, &ViewerWindow::onTriggeredRestartCamera);
    suc &= (bool)connect(ui->actionExit,              &QAction::triggered,   this, &ViewerWindow::onAppExit);
    suc &= (bool)connect(ui->actionInfomation,        &QAction::triggered,   this, &ViewerWindow::onTriggeredInformation);
    suc &= (bool)connect(ui->actionOpenLogDir,        &QAction::triggered,   this, &ViewerWindow::onTriggeredLogDir);
    suc &= (bool)connect(ui->actionsetDefaultSaveDir, &QAction::triggered,   this, &ViewerWindow::onTriggeredDefaultSavePath);

    suc &= (bool)connect(ui->menuTile, &QMenu::triggered,   this, &ViewerWindow::onWindowsMenuTriggered);
    suc &= (bool)connect(ui->menuTabs,  &QMenu::triggered,   this, &ViewerWindow::onWindowsMenuTriggered);
    suc &= (bool)connect(ui->menuTile, &CSMenu::clicked,    this, &ViewerWindow::onTriggeredWindowsTitle);
    suc &= (bool)connect(ui->menuTabs,  &CSMenu::clicked,    this, &ViewerWindow::onTriggeredWindowsTab);

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

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    QString defaultSavePath = tr("Set default save path ") + "(" + config->getDefaultSavePath() + ")";
    ui->actionsetDefaultSaveDir->setText(defaultSavePath);
}

void ViewerWindow::onRenderPageChanged(int idx)
{
    emit show3DUpdated(idx == RENDER_PAGE_3D);
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
        ui->menuCamera->setEnabled(true);

        // update camera info dialog
        CSCameraInfo info = cs::CSApplication::getInstance()->getCamera()->getCameraInfo();
        cameraInfoDialog->updateCameraInfo(info);

        circleProgressBar->close();
        break;
    }
    case CAMERA_DISCONNECTED:
    case CAMERA_DISCONNECTFAILED:
    case CAMERA_CONNECTFAILED:
        circleProgressBar->close();
        ui->menuCamera->setEnabled(false);
        break;
    case CAMERA_STARTED_STREAM:
        onCameraStreamStarted();
    case CAMERA_STOPPED_STREAM:
        circleProgressBar->close();
        break;
    default:    
        break;
    }

    // update windows state
    ui->menuWindows->setEnabled(cameraState == CAMERA_STARTED_STREAM);

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

void ViewerWindow::onRoiEditStateChanged(bool edit, QRectF rect)
{
    ui->renderWindow->onRoiEditStateChanged(edit, rect);
}

void ViewerWindow::onRoiRectFUpdated(QRectF rect)
{
    ui->paraSettingWidget->onRoiRectFUpdated(rect);
}

void ViewerWindow::onUpdateLanguage(QAction* action)
{
    auto app = cs::CSApplication::getInstance();

    if (action == ui->actionChinese)
    {
        ui->actionEnglish->setChecked(false);
        language = LANGUAGE_ZH;
        app->getAppConfig()->setLanguage("zh");
    }
    else
    {
        ui->actionChinese->setChecked(false);
        language = LANGUAGE_EN;
        app->getAppConfig()->setLanguage("en");
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

void ViewerWindow::onTriggeredDefaultSavePath()
{
    qInfo() << "trigger default save path";

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Set default save path"), config->getDefaultSavePath());

    if (!dirPath.isEmpty())
    {
        config->setDefaultSavePath(dirPath);

        QString defaultSavePath = tr("Set default save path (") + dirPath + ")";
        ui->actionsetDefaultSaveDir->setText(defaultSavePath);
        
        onShowStatusBarMessage(defaultSavePath, 3000);
    }
    else
    {
        qInfo() << "Cancel set default save path";
    }
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

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    QString defaultSavePath = tr("Set default save path ") + "(" + config->getDefaultSavePath() + ")";
    ui->actionsetDefaultSaveDir->setText(defaultSavePath);
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

void ViewerWindow::onCameraStreamStarted()
{
    // get camera parameter
    auto camera = cs::CSApplication::getInstance()->getCamera();

    QVariant hasRgbV, hasIrV, hasDepthV;
    camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    camera->getCameraPara(cs::parameter::PARA_DEPTH_HAS_LR, hasIrV);
    camera->getCameraPara(cs::parameter::PARA_HAS_DEPTH, hasDepthV);

    for (auto action : windowActions)
    {
        ui->menuWindows->removeAction(action);
        delete action;
    }
    windowActions.clear();

    if (hasIrV.toBool())
    {
        windowActions.push_back(new CSAction(CAMERA_DATA_L, "IR(L)"));
        windowActions.push_back(new CSAction(CAMERA_DATA_R, "IR(R)"));
    }

    if (hasDepthV.toBool())
    {
        windowActions.push_back(new CSAction(CAMERA_DATA_DEPTH, "Depth"));
        windowActions.push_back(new CSAction(CAMERA_DTA_POINT_CLOUD, "Point Cloud"));
    }

    if (hasRgbV.toBool())
    {
        windowActions.push_back( new CSAction(CAMERA_DATA_RGB, "RGB"));
    }

    updateWindowActions();
    onRenderWindowUpdated();
}

void ViewerWindow::onWindowsMenuTriggered(QAction* action)
{
    if (action)
    {
        onRenderWindowUpdated();
    }
}

void ViewerWindow::onRenderWindowUpdated()
{
    QVector<int> windows;
    for (auto action : windowActions)
    {
        if (action->isChecked())
        {
            windows.push_back(action->getType());
        }
    }

    // notify to render window
    emit renderWindowUpdated(windows);
    onWindowLayoutChanged();

}

void ViewerWindow::onTriggeredWindowsTitle()
{
    renderLayoutMode = LAYOUT_TILE;
    emit windowLayoutModeUpdated(LAYOUT_TILE);
    onWindowLayoutChanged();
}

void ViewerWindow::onTriggeredWindowsTab()
{
    renderLayoutMode = LAYOUT_TAB;
    emit windowLayoutModeUpdated(LAYOUT_TAB);
    onWindowLayoutChanged();
}

void ViewerWindow::onWindowLayoutChanged()
{
    QVector<int> windows;
    for (auto action : windowActions)
    {
        if (action->isChecked())
        {
            windows.push_back(action->getType());
        }
    }

    emit windowLayoutChanged(windows);
}

void ViewerWindow::updateWindowActions()
{
    for (auto action : windowActions)
    {
        ui->menuTile->addAction(action);
        ui->menuTabs->addAction(action);

        action->setCheckable(true);
        action->setChecked(true);
    }
}

void ViewerWindow::onRenderExit(int renderId)
{
    for (auto action : windowActions) 
    {
        if (action->getType() == renderId)
        {
            action->setChecked(false);
            onRenderWindowUpdated();
            break;
        }
    }
}
