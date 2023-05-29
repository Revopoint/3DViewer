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

#ifndef _CS_CSSPINBOX_H
#define _CS_CSSPINBOX_H

#include "cswidgets/csparawidget.h"
#include <QSpinBox>

class CustomSpinBox : public QSpinBox
{
public:
    CustomSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {}
    void wheelEvent(QWheelEvent* e) override {}
};

class QLabel;
class CSSpinBox : public CSParaWidget
{
    Q_OBJECT
public:
    CSSpinBox(int paraId, const char* title = "", QWidget* parent = nullptr);
    ~CSSpinBox();
    void setParaRange(const QVariant& min, const QVariant& max, const QVariant& step) override;
    void setValue(const QVariant& value) override;
    void retranslate(const char* context) override;
    void clearValues() override;
private slots:
    void onSpinBoxValueChanged(int);
private:
    QLabel* m_titleLabel;
    CustomSpinBox* m_spinBox;
};

#endif //_CS_CSSPINBOX_H