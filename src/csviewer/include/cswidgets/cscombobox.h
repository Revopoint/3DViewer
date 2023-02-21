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

#ifndef _CS_CSCOMBOBOX_H
#define _CS_CSCOMBOBOX_H
#include <QComboBox>
#include <QLabel>

#include "cswidgets/csparawidget.h"

class CustomComboBox : public QComboBox
{
public:
    CustomComboBox(QWidget* parent = nullptr) : QComboBox(parent) {}
    void wheelEvent(QWheelEvent* e) override {}
};

class QPushButton;
class CSComboBox : public CSParaWidget
{
    Q_OBJECT
public:
    CSComboBox(int paraId, const char* title = "", QWidget* parent = nullptr, const char* tips = "");
    ~CSComboBox();
    void setItems(const QList<QPair<QString, QVariant>>&) override;
    void setValue(const QVariant& value) override;
    void getValue(QVariant& value) override;
    void retranslate(const char* context) override;
    void clearValues() override;
private slots:
    void onComboBoxIndexChanged(int index);
private:
    CustomComboBox* m_comboBox;
    QLabel* m_titleLabel;
    int m_lastIndex;

    QPushButton* m_tipsButton = nullptr;
    const char* m_tips;
};

#endif // _CS_CSCOMBOBOX_H