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

#ifndef _CS_CSPARATWIDGET_H
#define _CS_CSPARATWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QApplication>

class CSParaWidget : public QWidget
{
    Q_OBJECT
public:
    CSParaWidget(int paraId, const char* name = "", QWidget * parent = nullptr);
    ~CSParaWidget();
   
    virtual void setParaRange(const QVariant&, const QVariant&, const QVariant&) {}
    virtual void setItems(const QList<QPair<QString, QVariant>>&) {}
    virtual void setValue(const QVariant&) {}
    virtual void getValue(QVariant&) {}
    virtual void retranslate(const char* context) {}

    virtual void clearValues() {}
    int getParaId() const;
protected:
    void paintEvent(QPaintEvent* event) override;
signals:
    void valueChanged(int, QVariant value);
protected:
    int m_paraId;
    const char* m_paraName;
};

#endif //_CS_CSPARATWIDGET_H