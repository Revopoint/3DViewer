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

#include "cswidgets/csprogressbar.h"

#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QPainterPath>

CSProgressBar::CSProgressBar(QWidget* parent)
    : QDialog(parent)
    , m_startAngle(90)
{
    resize(160, 160);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setObjectName("CSProgressBar");

    m_timer.setInterval(30);

    connect(&m_timer, &QTimer::timeout, [=]() 
        {
            m_startAngle -= 20;
            if (m_startAngle < 0 )
            {
                m_startAngle = 360;
            }

            update();
        });

    setAttribute(Qt::WA_TranslucentBackground, true);
}

CSProgressBar::~CSProgressBar()
{
    m_timer.stop();
}

void CSProgressBar::paintEvent(QPaintEvent* ev)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect rec = rect();

    int minWidth = qMin(rec.width(), rec.height()) / 2;
    int thickness = minWidth / 6;
    rec.adjust(thickness, thickness, -thickness, -thickness);
    minWidth = qMin(rec.width(), rec.height()) / 2;

    QConicalGradient gradient(rec.center(), m_startAngle);
    gradient.setColorAt(0, QColor(0, 169, 255));
    gradient.setColorAt(0.4, Qt::white);
    gradient.setColorAt(0.6, Qt::white);
    gradient.setColorAt(1, QColor(0, 169, 255));

    QPainterPath path;
    path.addEllipse(rec.center(), minWidth, minWidth);

    QPainterPath clipPath;
    clipPath.addEllipse(rec.center(), minWidth - thickness, minWidth - thickness);
    path = path.subtracted(clipPath);

    painter.fillPath(path, gradient);
    
    /*
    int segNum = 8;   // number of segments
    int radius = minWidth + 1;
    int t = thickness / 4;  // inter segment width
    int cx = rec.center().x();
    int cy = rec.center().y();

    for (int i = 0; i < segNum; i++)
    {
        //calculate inter segment pos
        float angle = 180.0 / segNum * i + 90;
        float cos1 = qCos(qDegreesToRadians(angle));
        float sin1 = qSin(qDegreesToRadians(angle));
        float cos2 = qCos(qDegreesToRadians(angle + 180));
        float sin2 = qSin(qDegreesToRadians(angle + 180));
        float x1 = cx + radius * cos1;
        float y1 = cy - radius * sin1;
        float x2 = cx + radius * cos2;
        float y2 = cy - radius * sin2;

        QPainterPath line;
        line.moveTo(x1 - sin1 * t, y1 - cos1 * t);
        line.lineTo(x1 + sin1 * t, y1 + cos1 * t);
        line.lineTo(cx, cy);
        line.lineTo(x2 - sin2 * t, y2 - cos2 * t);
        line.lineTo(x2 + sin2 * t, y2 + cos2 * t);
        path = path.subtracted(line); // subtract inter segment
        painter.fillPath(line,Qt::white);
    }
    */
}

void CSProgressBar::open()
{
    QDialog::open();
    qDebug() << "start timer";
    m_timer.start();
}

void CSProgressBar::done(int ret)
{
    qDebug() << "stop timer";
    m_timer.stop();
    QDialog::done(ret);
}
