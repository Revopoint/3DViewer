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

#ifndef _CS_CAMERALISTWIDGET_H
#define _CS_CAMERALISTWIDGET_H

#include <QWidget>
#include <QModelIndexList>

QT_BEGIN_NAMESPACE
namespace Ui { class CameraListWidget; }
QT_END_NAMESPACE

class QStandardItemModel;
class QListWidgetItem;
class CSTextImageButton;
class CameraListWidget : public QWidget
{
    Q_OBJECT
public:
    CameraListWidget(QWidget* parent = nullptr);
    ~CameraListWidget();
public slots:
    void onCameraListUpdated(const QStringList infoList);
    void onCameraStateChanged(int state);
    void onCameraListClicked(int rowIndex);

private slots:
    void onClickedCameraListItem(bool selected, QString text, QListWidgetItem* listItem);
signals:
    void connectCamera(QString serial);
    void disconnectCamera();
    void translateSignal();
private:
    void iniWidget();
    void initTopButton();
    void initConnections();
    void onTranslate();
    bool isNetConnect(QString info);
    void addListWidgetItem(const QString& text);
private:
    Ui::CameraListWidget* m_ui;
    CSTextImageButton* m_topItemButton;
};
#endif // _CS_CAMERALISTWIDGET_H