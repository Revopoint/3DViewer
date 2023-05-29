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

#ifndef _CS_CSROIEDITWIDGET_H
#define _CS_CSROIEDITWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QPushButton>

#include "csparawidget.h"

class CSRoiEditWidget : public CSParaWidget
{
    Q_OBJECT
public:
    CSRoiEditWidget(int paraId, const char* name = "", QWidget * parent = nullptr);
    ~CSRoiEditWidget();
    void setValue(const QVariant&) override {}
    void getValue(QVariant&) override {}
    void retranslate(const char* context);
signals:
    void clickedFullScreen();
    void clickedEditRoi();
private:
    QPushButton* m_fullScreenButton;
    QPushButton* m_roiEditButton;
};

#endif //_CS_CSPARATWIDGET_H
