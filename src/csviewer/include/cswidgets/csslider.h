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

#ifndef _CS_CSSLIDER_H
#define _CS_CSSLIDER_H

#include "cswidgets/csparawidget.h"
#include <QSlider>

class CustomSlider : public QSlider
{
    Q_OBJECT
public:
    CustomSlider(Qt::Orientation orientation, QWidget* parent = nullptr) : QSlider(orientation, parent) {}
    void wheelEvent(QWheelEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
signals:
    void sliderClicked(int value);
};

class QLabel;
class CSLineEdit;
class QIntValidator;
class CSSlider : public CSParaWidget
{
    Q_OBJECT
public:
    CSSlider(QWidget* parent = nullptr);
    CSSlider(int paraId, const char* title = "", QWidget* parent = nullptr);
    ~CSSlider();
    void setParaRange(const QVariant& min, const QVariant& max, const QVariant& step) override;
    void setValue(const QVariant& value) override;
    void retranslate(const char* context) override;
    void clearValues() override;

private slots:
    void onLineEditTextChanged();
    void onSliderValueChanged();
    void onSliderClicked(int value);
    void onLinEditFocusOut();
private:
    QLabel* m_titleLabel;
    CSLineEdit* m_lineEdit;
    CustomSlider* m_slider;
    QLabel* m_minLabel;
    QLabel* m_maxLabel;
    QIntValidator* m_intValidator;
    
    int m_step;
};
#endif // _CS_CSSLIDER_H
