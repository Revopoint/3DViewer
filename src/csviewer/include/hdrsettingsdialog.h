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

#ifndef _CS_HDRSETTINGSDIALOG_H
#define _CS_HDRSETTINGSDIALOG_H
#include <QDialog>

class CSParaWidget;
class QPushButton;
class CSProgressBar;
class HDRSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    HDRSettingsDialog(CSParaWidget* hdrMode, CSParaWidget* hdrLevel, CSParaWidget* hdrSettings, QWidget* parent = nullptr);
    ~HDRSettingsDialog();
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

public slots:
    void onHdrModeChanged(int mode);
    void onTranslate();
private:
    void initDialog();
    void initConnections();
    void updateProgressPosition();
signals:
    void refreshHdrSetting();
    void updateHdrSetting();
    void progressStateChanged(bool active);
private:
    CSParaWidget* m_hdrModeWidget;
    CSParaWidget* m_hdrLevelWidget;
    CSParaWidget* m_hdrTableWidget;
    QPushButton* m_hdrMeterButton;
    QPushButton* m_hdrOkButton;
    CSProgressBar* m_circleProgressBar;
};

#endif // _CS_HDRSETTINGSDIALOG_H