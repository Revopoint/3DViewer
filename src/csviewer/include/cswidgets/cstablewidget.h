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

#ifndef _CS_CSTABLEWIDGET_H
#define _CS_CSTABLEWIDGET_H

#include "cswidgets/csparawidget.h"

class QTableWidget;
class CSTableWidget : public CSParaWidget
{
    Q_OBJECT
public:
    CSTableWidget(int paraId, int cols, QStringList titleLabels, QWidget* parent = nullptr);
    ~CSTableWidget();
    void setValue(const QVariant&) override;
    void getValue(QVariant&) override;
    void retranslate(const char* context) override;
    void clearValues() override;
private slots:

private:
    QTableWidget* m_tableWidget;
    QStringList m_headers;
};
#endif //_CS_CSTABLEWIDGET_H