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
    , m_ui(new Ui::ViewerWindow)
    , m_translator(new QTranslator(this))
    , m_cameraInfoDialog(new CameraInfoDialog())
    , m_circleProgressBar(new CSProgressBar(this))
    , m_globalMessageBox(new CSMessageBox(this))
{
    // init translator
    qApp->installTranslator(m_translator);

    // set ui
    m_ui->setupUi(this);
    // init widgets
    initWindow();
    // set up signal connections
    initConnections();

    // set language
    m_language = (cs::CSApplication::getInstance()->getAppConfig()->getLanguage() == "zh") ? LANGUAGE_ZH : LANGUAGE_EN;

    m_ui->actionEnglish->setChecked(m_language == LANGUAGE_EN);
    m_ui->actionChinese->setChecked(m_language == LANGUAGE_ZH);

    onLanguageChanged();
}

ViewerWindow::~ViewerWindow()
{
    qApp->removeTranslator(m_translator);
    delete m_ui;
    delete m_cameraInfoDialog;
    if (m_cameraPlayerDialog)
    {
        delete m_cameraPlayerDialog;
    }

    if (m_formatConvertDialog)
    {
        delete m_formatConvertDialog;
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

    suc &= (bool)connect(m_ui->paraSettingWidget, &ParaSettingsWidget::showMessage,        this, &ViewerWindow::onShowStatusBarMessage);
    suc &= (bool)connect(m_ui->paraSettingWidget, &ParaSettingsWidget::roiStateChanged,    this, &ViewerWindow::onRoiEditStateChanged);
    suc &= (bool)connect(m_ui->paraSettingWidget, &ParaSettingsWidget::showProgressBar,    this, &ViewerWindow::showProgressBar, Qt::QueuedConnection);

    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     this,                   &ViewerWindow::onTranslate,            Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     m_ui->cameraListWidget,   &CameraListWidget::translateSignal,    Qt::QueuedConnection);
    suc &= (bool)connect(this, &ViewerWindow::translateSignal,     m_ui->paraSettingWidget,  &ParaSettingsWidget::translateSignal,  Qt::QueuedConnection);

    suc &= (bool)connect(this, &ViewerWindow::renderWindowUpdated,           m_ui->renderWindow,   &RenderWindow::onRenderWindowsUpdated);
    suc &= (bool)connect(this, &ViewerWindow::windowLayoutModeUpdated,       m_ui->renderWindow,   &RenderWindow::onWindowLayoutModeUpdated);

    suc &= (bool)connect(m_ui->renderWindow, &RenderWindow::roiRectFUpdated,   this,               &ViewerWindow::onRoiRectFUpdated);
    suc &= (bool)connect(m_ui->renderWindow, &RenderWindow::renderExit,        this,               &ViewerWindow::onRenderExit, Qt::QueuedConnection);

    // menu
    suc &= (bool)connect(m_ui->menuLanguage,            &QMenu::triggered,     this, &ViewerWindow::onUpdateLanguage);
    suc &= (bool)connect(m_ui->actionRestartCamera,     &QAction::triggered,   this, &ViewerWindow::onTriggeredRestartCamera);
    suc &= (bool)connect(m_ui->actionExit,              &QAction::triggered,   this, &ViewerWindow::onAppExit);
    suc &= (bool)connect(m_ui->actionInfomation,        &QAction::triggered,   this, &ViewerWindow::onTriggeredInformation);
    suc &= (bool)connect(m_ui->actionOpenLogDir,        &QAction::triggered,   this, &ViewerWindow::onTriggeredLogDir);
    suc &= (bool)connect(m_ui->actionsetDefaultSaveDir, &QAction::triggered,   this, &ViewerWindow::onTriggeredDefaultSavePath);
    suc &= (bool)connect(m_ui->actionIpSetting,         &QAction::triggered,   this, &ViewerWindow::onTriggeredIpSetting);
    suc &= (bool)connect(m_ui->actionplayFile,          &QAction::triggered,   this, &ViewerWindow::onTriggeredLoadFile);
    suc &= (bool)connect(m_ui->actionConvertDepth2PC,   &QAction::triggered,   this, &ViewerWindow::onTriggeredFormatConvert);
    suc &= (bool)connect(m_ui->actionManual,            &QAction::triggered,   this, &ViewerWindow::onTriggeredManual);
    suc &= (bool)connect(m_ui->actionGithub,            &QAction::triggered,   this, &ViewerWindow::onTriggeredGithub);
    suc &= (bool)connect(m_ui->actionWebSite,           &QAction::triggered,   this, &ViewerWindow::onTriggeredWebsite);
    suc &= (bool)connect(m_ui->actionAbout,             &QAction::triggered,   this, &ViewerWindow::onTriggeredAbout);

    suc &= (bool)connect(m_ui->actionTile,              &QAction::triggered,   this, &ViewerWindow::onTriggeredWindowsTile);
    suc &= (bool)connect(m_ui->actionTabs,              &QAction::triggered,   this, &ViewerWindow::onTriggeredWindowsTabs);

    suc &= (bool)connect(m_ui->menuViews,  &QMenu::triggered,   this, &ViewerWindow::onWindowsMenuTriggered);
    suc &= (bool)connect(m_ui->menuAutoNameWhenCapturuing, &QMenu::triggered, this, &ViewerWindow::onAutoNameMenuTriggered);

    suc &= (bool)connect(this, &ViewerWindow::showProgressBar, this, [=](bool show) 
        {
            if (show)
            {
                m_circleProgressBar->open();
            }
            else 
            {
                m_circleProgressBar->close();
            }
        });

    Q_ASSERT(suc);      
}

void ViewerWindow::initWindow()
{
    setWindowIcon(QIcon(":/resources/3DViewer.ico"));
    QString title = QString("%1 V%2").arg(APP_NAME).arg(APP_VERSION);
    setWindowTitle(title);
    showMaximized();

    m_ui->actionEnglish->setChecked(m_language == LANGUAGE_EN);
    m_ui->actionChinese->setChecked(m_language == LANGUAGE_ZH);

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    QString defaultSavePath = tr("Set default save path ") + "(" + config->getDefaultSavePath() + ")";
    m_ui->actionsetDefaultSaveDir->setText(defaultSavePath);

    bool autoName = config->getAutoNameWhenCapturing();
    m_ui->actionOff->setChecked(!autoName);
    m_ui->actionOn->setChecked(autoName);
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
        m_ui->menuCamera->setEnabled(false);
    case CAMERA_STARTING_STREAM:
    case CAMERA_STOPPING_STREAM:
    case CAMERA_DISCONNECTING:
        m_circleProgressBar->open();
        break;
    
    case CAMERA_CONNECTED:
    {
        m_ui->menuCamera->setEnabled(true);

        // update camera info dialog
        CSCameraInfo info = cs::CSApplication::getInstance()->getCamera()->getCameraInfo();
        m_cameraInfoDialog->updateCameraInfo(info);

        m_circleProgressBar->close();
        break;
    }
    case CAMERA_DISCONNECTED:
    case CAMERA_DISCONNECTFAILED:
    case CAMERA_CONNECTFAILED:
        m_circleProgressBar->close();
        m_ui->menuCamera->setEnabled(false);
        destoryRenderWindows();
        break;
    case CAMERA_START_STREAM_FAILED:
        m_circleProgressBar->close();
        destoryRenderWindows();
        break;
    case CAMERA_STARTED_STREAM:
        onCameraStreamStarted();
        break;
    case CAMERA_STOPPED_STREAM:
    case CAMERA_PAUSED_STREAM:
        onCameraStreamStopped();
        break;
    default:    
        break;
    }

    // update windows state
    m_ui->menuWindows->setEnabled(cameraState == CAMERA_STARTED_STREAM);

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
        m_ui->statusBar->showMessage(tr("Connecting camera"), timeoutMS);
        break;
    case CAMERA_CONNECTED:
        m_ui->statusBar->showMessage(tr("Camera connected successfully"), timeoutMS);
        break;
    case CAMERA_CONNECTFAILED:
        m_ui->statusBar->showMessage(tr("Camera connection failed"), timeoutMS);
        break;
    case CAMERA_DISCONNECTING:
        m_ui->statusBar->showMessage(tr("Disconnecting camera"), timeoutMS);
        break;
    case CAMERA_DISCONNECTED:
        m_ui->statusBar->showMessage(tr("Camera disconnected successfully"), timeoutMS);
        break;
    case CAMERA_DISCONNECTFAILED:
        m_ui->statusBar->showMessage(tr("Camera disconnection failed"), timeoutMS);
        break;
    case CAMERA_STARTING_STREAM:
        m_ui->statusBar->showMessage(tr("Starting preview"), timeoutMS);
        break;
    case CAMERA_STARTED_STREAM:
        m_ui->statusBar->showMessage(tr("Start preview successfully"), timeoutMS);
        break;
    case CAMERA_START_STREAM_FAILED:
        m_ui->statusBar->showMessage(tr("Start preview failed"), timeoutMS);
        break;
    case CAMERA_PAUSING_STREAM:
        m_ui->statusBar->showMessage(tr("Pausing preview"), timeoutMS);
        break;
    case CAMERA_PAUSED_STREAM:
        m_ui->statusBar->showMessage(tr("Preview has been paused"), timeoutMS);
        break;
    case CAMERA_RESTARTING_CAMERA:
        m_ui->statusBar->showMessage(tr("Restarting camera"));
        break;    
    case CAMERA_STOPPING_STREAM:
        m_ui->statusBar->showMessage(tr("Stopping preview"), timeoutMS);
        break;
    case CAMERA_STOPPED_STREAM:
        m_ui->statusBar->showMessage(tr("Preview has been stopped"), timeoutMS);
        break;
    default:
        break;
    }
}

void ViewerWindow::onRoiEditStateChanged(bool edit, QRectF rect)
{
    m_ui->renderWindow->onRoiEditStateChanged(edit, rect);
}

void ViewerWindow::onRoiRectFUpdated(QRectF rect)
{
    m_ui->paraSettingWidget->onRoiRectFUpdated(rect);
}

void ViewerWindow::onUpdateLanguage(QAction* action)
{
    auto app = cs::CSApplication::getInstance();

    if (action == m_ui->actionChinese)
    {
        m_ui->actionEnglish->setChecked(false);
        m_language = LANGUAGE_ZH;
        app->getAppConfig()->setLanguage("zh");
    }
    else
    {
        m_ui->actionChinese->setChecked(false);
        m_language = LANGUAGE_EN;
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
    m_cameraInfoDialog->show();
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
        m_ui->actionsetDefaultSaveDir->setText(defaultSavePath);
        
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
    cs::CSApplication::getInstance()->stop();
}

void ViewerWindow::onLanguageChanged()
{
    QString qmFile;
    switch (m_language)
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
    if (!m_translator->load(QDir::cleanPath(qmFile)))
    {
        qDebug() << "load qm failed : " << qmFile;
        return;
    }

    emit translateSignal();
}

void ViewerWindow::onTranslate()
{
    m_ui->retranslateUi(this);
    m_cameraInfoDialog->onTranslate();
    m_globalMessageBox->retranslate();

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    QString defaultSavePath = tr("Set default save path ") + "(" + config->getDefaultSavePath() + ")";
    m_ui->actionsetDefaultSaveDir->setText(defaultSavePath);

    if (m_ipSettingDialog)
    {
        m_ipSettingDialog->onTranslate();
    }

    if (m_formatConvertDialog)
    {
        m_formatConvertDialog->onTranslate();
    }

    if (m_cameraPlayerDialog)
    {
        m_cameraPlayerDialog->onTranslate();
    }

    m_ui->renderWindow->onTranslate();

    if (m_aboutDialog)
    {
        m_aboutDialog->onTranslate();
    }
}

void ViewerWindow::onRemovedCurrentCamera(QString serial)
{
    m_globalMessageBox->updateMessage(tr("The current camera has been disconnected"));
    m_globalMessageBox->open();

    connect(m_globalMessageBox, &CSMessageBox::accepted, this, &ViewerWindow::onConfirmCameraRemoved, Qt::UniqueConnection);
}

void ViewerWindow::onConfirmCameraRemoved()
{
    disconnect(m_globalMessageBox, &CSMessageBox::accepted, this, &ViewerWindow::onConfirmCameraRemoved);
}

void ViewerWindow::onShowStatusBarMessage(QString msg, int timeout)
{
    if (msg.isEmpty())
    {
        m_ui->statusBar->clearMessage();
    }
    else 
    {
        m_ui->statusBar->showMessage(msg, timeout);
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

    for (auto action : m_windowActions)
    {
        m_ui->menuWindows->removeAction(action);
        delete action;
    }
    m_windowActions.clear();

    if (hasIrV.toBool())
    {
        m_windowActions.push_back(new CSAction(CAMERA_DATA_L, "IR(L)"));
        m_windowActions.push_back(new CSAction(CAMERA_DATA_R, "IR(R)"));
    }

    if (hasDepthV.toBool())
    {
        m_windowActions.push_back(new CSAction(CAMERA_DATA_DEPTH, "Depth"));
        m_windowActions.push_back(new CSAction(CAMERA_DATA_POINT_CLOUD, "Point Cloud"));
    }

    if (hasRgbV.toBool())
    {
        m_windowActions.push_back( new CSAction(CAMERA_DATA_RGB, "RGB"));
    }

    m_ui->renderWindow->setShowTextureEnable(hasRgbV.toBool());

    updateWindowActions();
    onRenderWindowUpdated();
    
    bool suc = true;
    suc &= (bool)connect(app, &cs::CSApplication::output3DUpdated, m_ui->renderWindow, &RenderWindow::onOutput3DUpdated, Qt::QueuedConnection);
    suc &= (bool)connect(app, &cs::CSApplication::output2DUpdated, m_ui->renderWindow, &RenderWindow::onOutput2DUpdated, Qt::QueuedConnection);

    Q_ASSERT(suc);

    m_circleProgressBar->close();
}

void ViewerWindow::onCameraStreamStopped()
{
    m_circleProgressBar->close();
    auto app = cs::CSApplication::getInstance();
    disconnect(app, &cs::CSApplication::output3DUpdated, m_ui->renderWindow, &RenderWindow::onOutput3DUpdated);
    disconnect(app, &cs::CSApplication::output2DUpdated, m_ui->renderWindow, &RenderWindow::onOutput2DUpdated);
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
    for (auto action : m_windowActions)
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
    m_renderLayoutMode = LAYOUT_TILE;
    m_ui->actionTabs->setChecked(false);

    emit windowLayoutModeUpdated(LAYOUT_TILE);
    onWindowLayoutChanged();
}

void ViewerWindow::onTriggeredWindowsTabs()
{
    m_renderLayoutMode = LAYOUT_TAB;
    m_ui->actionTile->setChecked(false);

    emit windowLayoutModeUpdated(LAYOUT_TAB);
    onWindowLayoutChanged();
}

void ViewerWindow::onWindowLayoutChanged()
{
    QVector<int> windows;
    for (auto action : m_windowActions)
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
    for (auto action : m_windowActions)
    {
        m_ui->menuViews->addAction(action);

        action->setCheckable(true);
        action->setChecked(true);
    }
}

void ViewerWindow::onRenderExit(int renderId)
{
    for (auto action : m_windowActions) 
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
    for (auto action : m_windowActions)
    {
        m_ui->menuWindows->removeAction(action);
        delete action;
    }
    m_windowActions.clear();
    
    auto app = cs::CSApplication::getInstance();

    disconnect(app, &cs::CSApplication::output3DUpdated, m_ui->renderWindow, &RenderWindow::onOutput3DUpdated);
    disconnect(app, &cs::CSApplication::output2DUpdated, m_ui->renderWindow, &RenderWindow::onOutput2DUpdated);

    onRenderWindowUpdated();
}

void ViewerWindow::onTriggeredIpSetting()
{
    if (!m_ipSettingDialog) {
        m_ipSettingDialog = new IpSettingDialog(this);
        connect(m_ipSettingDialog, &IpSettingDialog::showMessage, this, &ViewerWindow::onShowStatusBarMessage);
    }

    m_ipSettingDialog->show();
}

void ViewerWindow::onTriggeredLoadFile()
{
    if (!m_cameraPlayerDialog)
    {
        m_cameraPlayerDialog = new CameraPlayerDialog();
        connect(m_cameraPlayerDialog, &CameraPlayerDialog::showMessage, this, &ViewerWindow::onShowStatusBarMessage, Qt::QueuedConnection);
    }

    m_cameraPlayerDialog->onLoadFile();
}

void ViewerWindow::onTriggeredFormatConvert()
{
    if (!m_formatConvertDialog)
    {
        m_formatConvertDialog = new FormatConvertDialog();
        connect(m_formatConvertDialog, &FormatConvertDialog::showMessage, this, &ViewerWindow::onShowStatusBarMessage, Qt::QueuedConnection);
    }

    m_formatConvertDialog->show();
}

void ViewerWindow::onAutoNameMenuTriggered(QAction* action)
{
    bool autoName = false;
    if (action == m_ui->actionOff)
    {
        bool isChekcked = m_ui->actionOff->isChecked();
        m_ui->actionOn->setChecked(!isChekcked);
        autoName = !isChekcked;
    }
    else 
    {
        bool isChekcked = m_ui->actionOn->isChecked();
        m_ui->actionOff->setChecked(!isChekcked);
        autoName = isChekcked;
    }

    auto config = cs::CSApplication::getInstance()->getAppConfig();
    config->setAutoNameWhenCapturing(autoName);
}

// If the current language is Chinese, open the Chinese manual or English manual
void ViewerWindow::onTriggeredManual()
{
    QString manualPath = QString("%1/document/manual").arg(APP_PATH);
    if (m_language == LANGUAGE_ZH)
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
    if (!m_aboutDialog)
    {
        m_aboutDialog = new AboutDialog(this);
    }

    m_aboutDialog->show();
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