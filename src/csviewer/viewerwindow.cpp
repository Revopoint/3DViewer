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

#include "viewerwindow.h"
#include "./ui_viewerwindow.h"

#include <QFile>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTranslator>
#include <QThread>
#include <QMessageBox>

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
#include "ipsettingdialog.h"
#include "cameraplayerdialog.h"
#include "formatconvertdialog.h"
#include "aboutdialog.h"

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
}

ViewerWindow::~ViewerWindow()
{
    qApp->removeTranslator(translator);
    delete ui;
    delete cameraInfoDialog;
    if (cameraPlayerDialog)
    {
        delete cameraPlayerDialog;
    }

    if (formatConvertDialog)
    {
        delete formatConvertDialog;
    }
}

void ViewerWindow::initConnections()
{
    bool suc = true;

    auto app = cs::CSApplication::getInstance();
    suc &= (bool)connect(app,  &cs::CSApplication::cameraStateChanged,   this, &ViewerWindow::onCameraStateChanged);
    suc &= (bool)connect(app,  &cs::CSApplication::removedCurrentCamera, this, &ViewerWindow::onRemovedCurrentCamera);

    suc &= (bool)connect(qApp, &QApplication::aboutToQuit,               this, &ViewerWindow::onAboutToQuit);
    suc &= (bool)connect(this, &ViewerWindow::windowLayoutChanged,       app,  &cs::CSApplication::onWindowLayoutChanged, Qt::QueuedConnection);

    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::showMessage,        this, &ViewerWindow::onShowStatusBarMessage);

    suc &= (bool)connect(ui->paraSettingWidget, &ParaSettingsWidget::roiStateChanged,    this, &ViewerWindow::onRoiEditStateChanged);

    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     this,                   &ViewerWindow::onTranslate,            Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     ui->cameraListWidget,   &CameraListWidget::translateSignal,    Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     ui->paraSettingWidget,  &ParaSettingsWidget::translateSignal,  Qt::QueuedConnection);

    suc &= (bool)connect(this, &ViewerWindow::renderWindowUpdated,           ui->renderWindow,   &RenderWindow::onRenderWindowsUpdated);
    suc &= (bool)connect(this, &ViewerWindow::windowLayoutModeUpdated,       ui->renderWindow,   &RenderWindow::onWindowLayoutModeUpdated);
    //suc &= (bool)connect(app, &cs::CSApplication::output3DUpdated,           ui->renderWindow,   &RenderWindow::onOutput3DUpdated);
    //suc &= (bool)connect(app, &cs::CSApplication::output2DUpdated,           ui->renderWindow,   &RenderWindow::onOutput2DUpdated);

    suc &= (bool)connect(ui->renderWindow, &RenderWindow::roiRectFUpdated,   this,               &ViewerWindow::onRoiRectFUpdated);
    suc &= (bool)connect(ui->renderWindow, &RenderWindow::renderExit,        this,               &ViewerWindow::onRenderExit, Qt::QueuedConnection);

    // menu
    suc &= (bool)connect(ui->menuLanguage,            &QMenu::triggered,     this, &ViewerWindow::onUpdateLanguage);
    suc &= (bool)connect(ui->actionRestartCamera,     &QAction::triggered,   this, &ViewerWindow::onTriggeredRestartCamera);
    suc &= (bool)connect(ui->actionExit,              &QAction::triggered,   this, &ViewerWindow::onAppExit);
    suc &= (bool)connect(ui->actionInfomation,        &QAction::triggered,   this, &ViewerWindow::onTriggeredInformation);
    suc &= (bool)connect(ui->actionOpenLogDir,        &QAction::triggered,   this, &ViewerWindow::onTriggeredLogDir);
    suc &= (bool)connect(ui->actionsetDefaultSaveDir, &QAction::triggered,   this, &ViewerWindow::onTriggeredDefaultSavePath);
    suc &= (bool)connect(ui->actionIpSetting,         &QAction::triggered,   this, &ViewerWindow::onTriggeredIpSetting);
    suc &= (bool)connect(ui->actionplayFile,          &QAction::triggered,   this, &ViewerWindow::onTriggeredLoadFile);
    suc &= (bool)connect(ui->actionConvertDepth2PC,   &QAction::triggered,   this, &ViewerWindow::onTriggeredFormatConvert);
    suc &= (bool)connect(ui->actionManual,            &QAction::triggered,   this, &ViewerWindow::onTriggeredManual);
    suc &= (bool)connect(ui->actionGithub,            &QAction::triggered,   this, &ViewerWindow::onTriggeredGithub);
    suc &= (bool)connect(ui->actionWebSite,           &QAction::triggered,   this, &ViewerWindow::onTriggeredWebsite);
    suc &= (bool)connect(ui->actionAbout,             &QAction::triggered,   this, &ViewerWindow::onTriggeredAbout);

    suc &= (bool)connect(ui->actionTile,              &QAction::triggered,   this, &ViewerWindow::onTriggeredWindowsTile);
    suc &= (bool)connect(ui->actionTabs,              &QAction::triggered,   this, &ViewerWindow::onTriggeredWindowsTabs);

    suc &= (bool)connect(ui->menuViews,  &QMenu::triggered,   this, &ViewerWindow::onWindowsMenuTriggered);
    suc &= (bool)connect(ui->menuAutoNameWhenCapturuing, &QMenu::triggered, this, &ViewerWindow::onAutoNameMenuTriggered);

    Q_ASSERT(suc);      
}

void ViewerWindow::initWindow()
{
    setWindowIcon(QIcon(":/resources/3DViewer.ico"));
    QString title = QString("%1 V%2").arg(APP_NAME).arg(APP_VERSION);
    setWindowTitle(title);
    showMaximized();

    ui->actionEnglish->setChecked(language == LANGUAGE_EN);
    ui->actionChinese->setChecked(language == LANGUAGE_ZH);

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    QString defaultSavePath = tr("Set default save path ") + "(" + config->getDefaultSavePath() + ")";
    ui->actionsetDefaultSaveDir->setText(defaultSavePath);

    bool autoName = config->getAutoNameWhenCapturing();
    ui->actionOff->setChecked(!autoName);
    ui->actionOn->setChecked(autoName);
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
        destoryRenderWindows();
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

    if (ipSettingDialog)
    {
        ipSettingDialog->onTranslate();
    }

    if (formatConvertDialog)
    {
        formatConvertDialog->onTranslate();
    }

    if (cameraPlayerDialog)
    {
        cameraPlayerDialog->onTranslate();
    }

    ui->renderWindow->onTranslate();

    if (aboutDialog)
    {
        aboutDialog->onTranslate();
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

void ViewerWindow::onCameraStreamStarted()
{
    // get camera parameter
    auto app = cs::CSApplication::getInstance();
    auto camera = app->getCamera();

    QVariant hasRgbV, hasIrV, hasDepthV;
    camera->getCameraPara(cs::parameter::PARA_HAS_RGB, hasRgbV);
    camera->getCameraPara(cs::parameter::PARA_DEPTH_HAS_IR, hasIrV);
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
        windowActions.push_back(new CSAction(CAMERA_DATA_POINT_CLOUD, "Point Cloud"));
    }

    if (hasRgbV.toBool())
    {
        windowActions.push_back( new CSAction(CAMERA_DATA_RGB, "RGB"));
    }

    ui->renderWindow->setShowTextureEnable(hasRgbV.toBool());

    updateWindowActions();
    onRenderWindowUpdated();

    bool suc = true;
    suc &= (bool)connect(app, &cs::CSApplication::output3DUpdated, ui->renderWindow, &RenderWindow::onOutput3DUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::output2DUpdated, ui->renderWindow, &RenderWindow::onOutput2DUpdated);

    Q_ASSERT(suc);
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

void ViewerWindow::onTriggeredWindowsTile()
{
    renderLayoutMode = LAYOUT_TILE;
    ui->actionTabs->setChecked(false);

    emit windowLayoutModeUpdated(LAYOUT_TILE);
    onWindowLayoutChanged();
}

void ViewerWindow::onTriggeredWindowsTabs()
{
    renderLayoutMode = LAYOUT_TAB;
    ui->actionTile->setChecked(false);

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
        ui->menuViews->addAction(action);

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

void ViewerWindow::destoryRenderWindows()
{
    for (auto action : windowActions)
    {
        ui->menuWindows->removeAction(action);
        delete action;
    }
    windowActions.clear();
    
    auto app = cs::CSApplication::getInstance();

    disconnect(app, &cs::CSApplication::output3DUpdated, ui->renderWindow, &RenderWindow::onOutput3DUpdated);
    disconnect(app, &cs::CSApplication::output2DUpdated, ui->renderWindow, &RenderWindow::onOutput2DUpdated);

    onRenderWindowUpdated();
}

void ViewerWindow::onTriggeredIpSetting()
{
    if (!ipSettingDialog) {
        ipSettingDialog = new IpSettingDialog(this);
        connect(ipSettingDialog, &IpSettingDialog::showMessage, this, &ViewerWindow::onShowStatusBarMessage);
    }

    ipSettingDialog->show();
}

void ViewerWindow::onTriggeredLoadFile()
{
    if (!cameraPlayerDialog)
    {
        cameraPlayerDialog = new CameraPlayerDialog();
        connect(cameraPlayerDialog, &CameraPlayerDialog::showMessage, this, &ViewerWindow::onShowStatusBarMessage, Qt::QueuedConnection);
    }

    cameraPlayerDialog->onLoadFile();
}

void ViewerWindow::onTriggeredFormatConvert()
{
    if (!formatConvertDialog)
    {
        formatConvertDialog = new FormatConvertDialog();
        connect(formatConvertDialog, &FormatConvertDialog::showMessage, this, &ViewerWindow::onShowStatusBarMessage, Qt::QueuedConnection);
    }

    formatConvertDialog->show();
}

void ViewerWindow::onAutoNameMenuTriggered(QAction* action)
{
    bool autoName = false;
    if (action == ui->actionOff)
    {
        bool isChekcked = ui->actionOff->isChecked();
        ui->actionOn->setChecked(!isChekcked);
        autoName = !isChekcked;
    }
    else 
    {
        bool isChekcked = ui->actionOn->isChecked();
        ui->actionOff->setChecked(!isChekcked);
        autoName = isChekcked;
    }

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    config->setAutoNameWhenCapturing(autoName);
}

// If the current language is Chinese, open the Chinese manual or English manual
void ViewerWindow::onTriggeredManual()
{
    QString manualPath = QString("%1/document/manual").arg(APP_PATH);
    if (language == LANGUAGE_ZH)
    {
        manualPath += "/Chinese/index.html";
    }
    else 
    {
        manualPath += "/English/index.html";
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(manualPath));
}

void ViewerWindow::onTriggeredAbout()
{
    if (!aboutDialog)
    {
        aboutDialog = new AboutDialog(this);
    }

    aboutDialog->show();
}

void ViewerWindow::onTriggeredGithub()
{
    QDesktopServices::openUrl(QUrl(GITHUB_URL));
}

void ViewerWindow::onTriggeredWebsite()
{
    QDesktopServices::openUrl(QUrl(WEBSITE_URL));
}

void ViewerWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("Tips"));
    msgBox.setText(tr("Confirm to exit the application ?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.button(QMessageBox::Yes)->setText(tr("Yes"));
    msgBox.button(QMessageBox::No)->setText(tr("No"));

    int button = msgBox.exec();
    if (button == QMessageBox::No)
    {
        event->ignore();
    }
    else if (button == QMessageBox::Yes)
    {
        event->accept();
    }
}