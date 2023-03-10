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

#ifndef _CS_VIEWERWINDOW_H
#define _CS_VIEWERWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QCloseEvent>
#include <cstypes.h>
#include <hpp/Processing.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { 
    class ViewerWindow; 
}
QT_END_NAMESPACE

namespace cs {
    class ProcessStrategy;
}

class RenderWidget2D;
class QFileDialog;
class QTranslator;
class CameraInfoDialog;
class CSProgressBar;
class CSMessageBox;
class CSAction;
class IpSettingDialog;
class CameraPlayerDialog;
class FormatConvertDialog;
class AboutDialog;

class ViewerWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum RENDER_PAGE_ID
    {
        RENDER_PAGE_2D = 0,
        RENDER_PAGE_3D = 1,
        RENDER_PAGE_COUNT
    };

    enum CS_LANGUAGE
    {
        LANGUAGE_EN = 0,
        LANGUAGE_ZH,
        LANGUAGE_COUNT
    };

    ViewerWindow(QWidget* parent = nullptr);
    ~ViewerWindow();

    void closeEvent(QCloseEvent* event) override;
private slots:
    void onRenderPageChanged(int idx);
    void onCameraStateChanged(int state);

    void onRoiEditStateChanged(bool edit, QRectF rect);
    void onRoiRectFUpdated(QRectF rect);
    void onShowStatusBarMessage(QString msg, int timeout);

    // start the stream
    void onCameraStreamStarted();
    void onCameraStreamStopped();
private slots:
    // menu
    void onUpdateLanguage(QAction* action);
    void onTriggeredRestartCamera();
    void onTriggeredInformation();
    void onTriggeredLogDir();
    void onTriggeredDefaultSavePath();
    void onTriggeredIpSetting();
    void onTriggeredLoadFile();
    void onTriggeredFormatConvert();
    void onTriggeredManual();
    void onTriggeredAbout();
    void onTriggeredGithub();
    void onTriggeredWebsite();

    void onAppExit();
private slots:
    void onTranslate();
    void onRemovedCurrentCamera(QString serial);
    void onConfirmCameraRemoved();
    void onAboutToQuit();
    void onRenderWindowUpdated();
    void onWindowsMenuTriggered(QAction* action);
    void onTriggeredWindowsTile();
    void onTriggeredWindowsTabs();
    void onAutoNameMenuTriggered(QAction* action);

    void onRenderExit(int renderId);
    void onWindowLayoutChanged();
signals:
    void translateSignal(); 
    void show3DUpdated(bool show);
    void renderWindowUpdated(QVector<int> windows);
    void windowLayoutModeUpdated(int mode);
    void windowLayoutChanged(QVector<int> windows);

    void showProgressBar(bool show);
private:
    void initWindow();
    void initConnections();
    void onLanguageChanged();
    void updateStatusBar(int state);
    void updateWindowActions();
    void destoryRenderWindows();
private:
    Ui::ViewerWindow* m_ui;
    CS_LANGUAGE m_language;
    QTranslator* m_translator;

    CameraInfoDialog* m_cameraInfoDialog;
    CSProgressBar* m_circleProgressBar;
    CSMessageBox* m_globalMessageBox;

    QVector<CSAction*> m_windowActions;
    WINDOWLAYOUT_MODE m_renderLayoutMode = LAYOUT_TILE;

    // set ip dialog
    IpSettingDialog* m_ipSettingDialog = nullptr;
    CameraPlayerDialog* m_cameraPlayerDialog = nullptr;
    FormatConvertDialog* m_formatConvertDialog = nullptr;

    // about dialog
    AboutDialog* m_aboutDialog = nullptr;
};
#endif // _CS_VIEWERWINDOW_H
