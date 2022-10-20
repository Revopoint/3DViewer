/*******************************************************************************
* This file is part of the 3DViewer
*
* Copyright 2022-2026 (C) Revopoint3D AS
* All rights reserved.
*
* Revopoint3D Software License, v1.0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistribution in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of Revopoint3D AS nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* 4. This software, with or without modification, must not be used with any
* other 3D camera than from Revopoint3D AS.
*
* 5. Any software provided in binary form under this license must not be
* reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY REVOPOINT3D AS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL REVOPOINT3D AS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Info:  https://www.revopoint3d.com
******************************************************************************/

#include "cswidgets/csprogressbar.h"

#include <QPainter>
#include <QDebug>
#include <QtMath>
#include <QPainterPath>

CSProgressBar::CSProgressBar(QWidget* parent)
    : QDialog(parent)
    , startAngle(90)
{
    resize(160, 160);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setObjectName("CSProgressBar");

    timer.setInterval(30);

    connect(&timer, &QTimer::timeout, [=]() 
        {
            startAngle -= 20;
            if (startAngle < 0 )
            {
                startAngle = 360;
            }

            update();
        });

    setAttribute(Qt::WA_TranslucentBackground, true);
}

CSProgressBar::~CSProgressBar()
{
    timer.stop();
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

    QConicalGradient gradient(rec.center(), startAngle);
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
    timer.start();
}

void CSProgressBar::done(int ret)
{
    qDebug() << "stop timer";
    timer.stop();
    QDialog::done(ret);
}
