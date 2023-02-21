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

#ifndef _CS_PARASETTINGSWIDGET_H
#define _CS_PARASETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include <QList>
#include <QPair>
#include <QPushButton>
#include <memory>
#include <QVariant>
#include <QTimer>
#include <QThread>

#include <cstypes.h>

QT_BEGIN_NAMESPACE
namespace Ui { class ParameterSettingsWidget; }
namespace cs { class ICSCamera; }

class CSParaWidget;
QT_END_NAMESPACE

class HDRSettingsDialog;
class CaptureSettingDialog;
class QSpacerItem;
class CSTextImageButton;
class ParaMonitorThread;

class ParaSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    enum PARA_PAGE_ID
    {
        PAGE_DEPTH_CAMERA,
        PAGE_RGB_CAMERA,
        PAGE_COUNT
    };

    ParaSettingsWidget(QWidget* parent = nullptr);
    ~ParaSettingsWidget();
public slots:
    void onCameraStateChanged(int state);
    void onUpdatedCameraParas();
    //roi
    void onRoiRectFUpdated(QRectF rect);

    // control
    void onPreviewStateChanged(bool toggled);
    void onPreviewButtonToggled(bool toggled);
    void onClickedRestartCamera();
    void onClickedDisconnCamera();
    void onClickedStopStream();
    void onClickSingleShot();

    void onClickCaptureSingle();
    void onClickCaptureMultiple();

    void onCaptureStateChanged(int captureType, int state, QString message);
private slots:
    void onClickDepthButton();
    void onClickRgbButton();
    void onClickRoiEditButton();
    void onClickFullScreenButton();

    void onPageChanged(int idx);

    // ui changed
    void onParaValueChanged(int paraId, QVariant value);

    //camera para changed
    void onCameraParaUpdated(int paraId, QVariant value);
    void onCameraParaRangeUpdated(int paraId);
    void onCameraParaItemsUpdated(int paraId);
    
    // hdr
    void onRefreshHdrSetting();
    void onUpdateHdrSetting();

    void onTranslate();
signals:
    void roiStateChanged(bool edit, QRectF rect);
    void translateSignal();
    void hdrModeChanged(int mode);
    void clickedCloseButton();
    void showMessage(QString msg, int timeout = 0);
    
    void showProgressBar(bool show);
private:
    void initWidget();
    void initDepthPara();
    void initHDRParaWidgets();
    void initParaConnections();
    void initRgbPara();
    void initTopButton();

    void initConnections();
    void addDepthParaWidget(CSParaWidget* widget);
    void addRgbParaWidget(CSParaWidget* widget);
    void addDepthDividLine();
    void addRgbDividLine();
    void addHdrButtons();

    void updateParaRanges();
    void updateParaValues();
    void updateParaItems();

    void onParaLinkResponse(int paraId, QVariant);

    void updateWidgetsState(int cameraState);
    void updateControlButtonState(int cameraState);

    void stopParaMonitor();
    void startParaMonitor();
private:
    Ui::ParameterSettingsWidget* m_ui;
    std::shared_ptr<cs::ICSCamera> m_cameraPtr;

    QMap<int, CSParaWidget*> m_paraWidgets;
    CaptureSettingDialog* m_captureSettingDialog;

    QSpacerItem* m_verticalSpacer = nullptr;

    QPushButton* m_hdrRefreshButton;
    QPushButton* m_hdrConfirmButton;
    QWidget* m_hdrButtonArea;

    CSTextImageButton* m_topItemButton;
    bool m_isSingleShotMode = false;

    ParaMonitorThread* m_paraMonitorThread;

    CameraCaptureConfig m_captureConfig;
};

class ParaMonitorThread : public QThread
{
    Q_OBJECT
public:
    ParaMonitorThread();
    void run() override;

    void setAutoExposureDepth(bool enable);
    void setAutoExposureRgb(bool enable);
    void setAutoWhiteBalance(bool enable);
private:
    bool m_enableAutoExposureDepth = false;
    bool m_enableAutoExposureRgb = false;
    bool m_enableAutoWhiteBalance = false;
    std::shared_ptr<cs::ICSCamera> m_cameraPtr;
};

#endif //_CS_PARASETTINGSWIDGET_H
