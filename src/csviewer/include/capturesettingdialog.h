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

#ifndef _CS_CAPTURE_SETTING_DIALOG_H
#define _CS_CAPTURE_SETTING_DIALOG_H
#include <QDialog>
#include <QVector>
#include <cstypes.h>

namespace Ui {
    class CaptureSettingWidget;
}

class QCheckBox;
class QIntValidator;
class CaptureSettingDialog : public QDialog
{
    Q_OBJECT
public:
    CaptureSettingDialog(QWidget* parent = nullptr);
    ~CaptureSettingDialog();

    void showEvent(QShowEvent* event) override;
public slots:
    void onCaptureNumberUpdated(int number, int dropped);
    void onCaptureStateChanged(int captureType, int state, QString message);
    void onTranslate();

    void reject() override;
private slots:
    void onStartCapture();
    void onStopCapture();
    
    void onDataTypeChanged();
    void onSaveFormatChanged(int index);
    void onCaptureFrameNumberChanged();
private:
    void initDefaultCaptureConfig();
    void initDialog();
    void initConnections();
private:
    Ui::CaptureSettingWidget* m_ui;
    CameraCaptureConfig m_captureConfig;

    QIntValidator* m_intValidator = nullptr;

    // min / max capture count
    int m_minCaptureCount = 1;
    int m_maxCaptureCount = 10000;
    int m_defaultCaptureCount = 30;

    QVector<QCheckBox*> m_dataTypeCheckBoxs;
    bool m_isCapturing = false;
};

#endif //_CS_CAPTURE_SETTING_DIALOG_H
