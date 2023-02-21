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

#include "cswidgets/csline.h"

#include <QStyleOption>
#include <QPainter>
#include <QHBoxLayout>
#include <QVBoxLayout>

CSLine::CSLine(QWidget* parent)
    : QFrame(parent)
{
    setObjectName("CSLine");
    setAttribute(Qt::WA_StyledBackground, true);
    setFrameShadow(QFrame::Sunken);
    setFixedHeight(1);
}

void CSLine::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

CSHLine::CSHLine(QWidget* parent)
    : QWidget(parent)
    , m_line(new CSLine(this))
{
    m_line->setFrameShape(QFrame::HLine);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_line);
    layout->setContentsMargins(0, 15, 0, 15);
}

CSVLine::CSVLine(QWidget* parent)
    : QWidget(parent)
{
    m_line->setFrameShape(QFrame::VLine);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_line);
    layout->setContentsMargins(15, 0, 15, 0);
}