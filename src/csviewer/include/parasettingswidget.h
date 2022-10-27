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

QT_BEGIN_NAMESPACE
namespace Ui { class ParameterSettingsWidget; }
namespace cs { class ICSCamera; }

class CSParaWidget;
QT_END_NAMESPACE

class HDRSettingsDialog;
class CaptureSettingDialog;
class QSpacerItem;
class CSTextImageButton;
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

    void stopTimers();
private:
    Ui::ParameterSettingsWidget* ui;
    std::shared_ptr<cs::ICSCamera> cameraPtr;

    QMap<int, CSParaWidget*> paraWidgets;
    CaptureSettingDialog* captureSettingDialog;

    QSpacerItem* verticalSpacer = nullptr;

    QPushButton* hdrRefreshButton;
    QPushButton* hdrConfirmButton;
    QWidget* hdrButtonArea;

    CSTextImageButton* topItemButton;
    bool isSingleShotMode = false;

    QTimer* depthAutoExposureTimer;
    QTimer* rgbAutoExposureTimer;
    QTimer* autoWhiteBalanceTimer;
};


#endif //_CS_PARASETTINGSWIDGET_H
