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

#ifndef _CS_CSRANGEEDIT_H
#define _CS_CSRANGEEDIT_H

#include "cswidgets/csparawidget.h"

#include <QIntValidator>
#include <QLineEdit>
#include <QKeyEvent>

class QLabel;
class CSLineEdit;
class CSRangeEdit : public CSParaWidget
{
    Q_OBJECT
public:
    CSRangeEdit(int paraId, const char* title = "", QWidget* parent = nullptr);
    ~CSRangeEdit();
    void setParaRange(const QVariant& min, const QVariant& max, const QVariant& step) override;
    void setValue(const QVariant& value) override;
    void retranslate(const char* context) override;
    void clearValues() override;
private slots:
    void onMaxEditTextChanged();
    void onMinEditTextChanged();
    void onMinFocusOut();
    void onMaxFocusOut();
private:
    CSLineEdit* m_rangeMinEdit;
    CSLineEdit* m_rangeMaxEdit;
    QLabel* m_titleLabel;
    QIntValidator* m_intValidator;

    int m_rangeBottom;
    int m_rangeTop;
};

#endif // _CS_CSRANGEEDIT_H