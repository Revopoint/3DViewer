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

#include "cswidgets/csroi.h"

#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCursor>
#include <QMouseEvent>
#include <QDebug>

#include "cswidgets/csline.h"

enum ROI_POSITION
{
    LEFT_TOP = 0,
    LEFT_CENTER,
    LEFT_BOTTOM,
    CENTER_TOP,
    CENTER_BOTTOM,
    RIGHT_TOP,
    RIGHT_CENTER,
    RIGHT_BOTTOM,
};

CSROIWidget::CSROIWidget(QWidget* parent)
    : QWidget(parent)
    , roiRect(QRectF(0.0, 0.0, 1.0, 1.0))
    , roiOffset(10, 10, 10, 10)
{
    setAttribute(Qt::WA_StyledBackground, true);
    //setVisible(false);
    roiRectLast = roiRect;

    buttonArea = new QWidget(this);
    buttonArea->setObjectName("RoiButtonArea");
    QHBoxLayout* hlayout = new QHBoxLayout(buttonArea);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSpacing(0);

    cancelButton = new QPushButton(buttonArea);
    okButton = new QPushButton(buttonArea);

    cancelButton->setObjectName("CancelButton");
    okButton->setObjectName("OkButton");

    QWidget* line = new QWidget(buttonArea);
    line->setObjectName("Line");
    line->setFixedHeight(15);
    line->setFixedWidth(1);

    hlayout->addWidget(cancelButton);
    hlayout->addWidget(line);
    hlayout->addWidget(okButton);

    connect(cancelButton, &QPushButton::clicked, this, [=]()
    {
        roiRect = roiRectLast;
        setVisible(false);
        emit roiVisialeChanged(false);
    });

    connect(okButton, &QPushButton::clicked, this, [=]()
    {
        roiRectLast = roiRect;
        emit roiValueUpdated(roiRect);

    });
}

CSROIWidget::~CSROIWidget()
{

}

void CSROIWidget::updateRoiRectF(QRectF rect)
{
    roiRect = rect;
}

void CSROIWidget::setOffset(QMargins offset)
{
    roiOffset = offset;
}

void CSROIWidget::paintEvent(QPaintEvent* event)
{
    // draw roi
    drawRoi();

    // update button position
    updateButtonsPos();
    QWidget::paintEvent(event);
}

void CSROIWidget::updateButtonsPos()
{
    int w = width() - roiOffset.left() - roiOffset.right();
    int h = height() - roiOffset.top() - roiOffset.bottom();

    int right = w * roiRect.right() + roiOffset.left();
    int bottom = h * roiRect.bottom() + roiOffset.top();

    int x = right - buttonArea->width();
    int y = bottom + 10;

    buttonArea->setGeometry(x, y, buttonArea->width(), buttonArea->height());
}

void CSROIWidget::drawRoi()
{
    QPainter painter(this);

    QColor color(0, 169, 255);
    painter.setPen(QPen(color, 2));

    int w = width() - roiOffset.left() - roiOffset.right();
    int h = height() - roiOffset.top() - roiOffset.bottom();

    painter.translate(roiOffset.left(), roiOffset.top());

    int left = w * roiRect.left();
    int top = h * roiRect.top();
    int right = w * roiRect.right();
    int bottom = h * roiRect.bottom();

    painter.drawRect(QRect(left, top , right - left, bottom - top));

    int d = 6;
    int r = d / 2;
    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(Qt::white);
    // left top
    painter.drawEllipse(left-r, top-r, d, d);

    // center top
    painter.drawEllipse((left + right) / 2 -r, top-r, d, d);

    // right top
    painter.drawEllipse(right-r, top-r, d, d);

    // left bottom
    painter.drawEllipse(left-r, bottom-r, d, d);

    // center bottom
    painter.drawEllipse((left + right) / 2-r, bottom-r, d, d);

    // right bottom
    painter.drawEllipse(right-r, bottom-r, d, d);

    // left center
    painter.drawEllipse(left-r, (bottom+top) / 2 - r, d, d);

    // right center
    painter.drawEllipse(right-r, (bottom+top) / 2 -r, d, d);
}

void CSROIWidget::mouseReleaseEvent(QMouseEvent* event)
{
    pressPositon = -1;
    lastPosition = QPoint(0, 0);

    //QWidget::mouseReleaseEvent(event);
}

void CSROIWidget::mousePressEvent(QMouseEvent* event)
{
    //QWidget::mousePressEvent(event);
    //QPoint pos = QCursor().pos();
    pressPositon = -1;

    int w = width() - roiOffset.left() - roiOffset.right();
    int h = height() - roiOffset.top() - roiOffset.bottom();

    int left = w * roiRect.left() + roiOffset.left();
    int top = h * roiRect.top() + roiOffset.top();
    int right = w * roiRect.right() + roiOffset.left();
    int bottom = h * roiRect.bottom() + roiOffset.top();

    static int width = 30;
    static int halfWidth = width / 2;

    QPoint pt = this->mapFromGlobal(event->globalPos());
    lastPosition = pt;

    if (left == right && top == bottom)
    {
        pressPositon = RIGHT_BOTTOM;
        return;
    }

    //left top
    int x = left - halfWidth; x = x < 0 ? 0 : x;
    int y =  top - halfWidth; y = y < 0 ? 0 : y;
    QRect rect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon = LEFT_TOP;
        return;
    }

    // center top
    x = (left + right) / 2 - halfWidth; x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon =CENTER_TOP;
        return;
    }

    // right top
    x = right - halfWidth; x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon = RIGHT_TOP;
        return;
    }

    // right center
    y = (top + bottom) / 2 - halfWidth; y = y < 0 ? 0 : y;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon =RIGHT_CENTER;
        return;
    }

    // left center
    x = left - halfWidth;x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon =LEFT_CENTER;
        return;
    }

    // left bottom
    y = bottom - halfWidth; y = y < 0 ? 0 : y;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon = LEFT_BOTTOM;
        return;
    }

    // center bottom
    x = (left + right) / 2 - halfWidth; x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon = CENTER_BOTTOM;
        return;
    }

    // right bottom
    x = right - halfWidth;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        pressPositon = RIGHT_BOTTOM;
        return;
    }
}

void CSROIWidget::mouseMoveEvent(QMouseEvent* event)
{
     auto pos = event->pos();
     QPoint curPos = event->pos();
     QPoint dPos = curPos - lastPosition;

     //qDebug() << "pos x = " << pos.x() << ", y = " << pos.y();
     auto dx = (dPos.x()) * 1.0 / (width() - roiOffset.left() - roiOffset.right());
     auto dy = (dPos.y()) * 1.0 / (height() - roiOffset.top() - roiOffset.bottom());
     
     //roiRect
     lastPosition = curPos;
     if(pressPositon < 0)
     {
         translateRoi(dx, dy);
         return;
     }

     auto left = roiRect.left();
     auto top = roiRect.top();
     auto right = roiRect.right();
     auto bottom = roiRect.bottom();

     auto x = left;
     auto y = top;

     if(pressPositon == LEFT_TOP)
     {
        x += dx;
        y += dy;

        x = x > right ? right : x;
        x = x < 0 ? 0 : x;

        y = y < 0 ? 0 : y;
        y = y > bottom ? bottom : y;

        roiRect.setLeft(x);
        roiRect.setTop(y);
     }
     else if(pressPositon == LEFT_CENTER)
     {
         x += dx;

         x = x > right ? right : x;
         x = x < 0 ? 0 : x;

         roiRect.setLeft(x);
     }
     else if(pressPositon == LEFT_BOTTOM)
     {
         y = bottom;
         x += dx;
         y += dy;

         x = x > right ? right : x;
         x = x < 0 ? 0 : x;

         y = y > 1 ? 1 : y;
         y = y < top ? top : y;

         roiRect.setLeft(x);
         roiRect.setBottom(y);
     }
     else if(pressPositon == CENTER_TOP)
     {
         y += dy;
         y = y < 0 ? 0 : y;
         y = y > bottom ? bottom : y;
         roiRect.setTop(y);
     }
     else if(pressPositon == CENTER_BOTTOM)
     {
         y = bottom;
         y += dy;
         y = y > 1 ? 1: y;
         y = y < top ? top : y;
         roiRect.setBottom(y);
     }
     else if(pressPositon == RIGHT_TOP)
     {
         x = right;
         x += dx;
         y += dy;

         x = x > 1 ? 1 : x;
         x = x < left ? left : x;

         y = y < 0 ? 0 : y;
         y = y > bottom ? bottom : y;

         roiRect.setRight(x);
         roiRect.setTop(y);
     }
     else if(pressPositon == RIGHT_CENTER)
     {
         x = right;
         x += dx;

         x = x > 1 ? 1 : x;
         x = x < left? left : x;

         roiRect.setRight(x);
     }
     else if(pressPositon == RIGHT_BOTTOM)
     {
         x = right;
         y = bottom;
         x += dx;
         y += dy;

         x = x > 1 ? 1 : x;
         x = x < left ? left : x;

         y = y >1 ? 1 : y;
         y = y <top ? top : y;

         roiRect.setRight(x);
         roiRect.setBottom(y);
     }

     update();
}

void CSROIWidget::translateRoi(float x, float y)
{
    //qDebug() << "dx : " << x << ", dy : "<< y;
    auto left = roiRect.left();
    auto top = roiRect.top();
    auto right = roiRect.right();
    auto bottom = roiRect.bottom();

    if (x > 0)
    {
        qreal r = right + x;
        r = r > 1 ? 1 : r;
        left += (r - right);
        right = r;
    }
    else 
    {
        qreal l = left + x;
        l = l < 0 ? 0 : l;
        right += (l - left);
        left = l;
    }

    if (y > 0)
    {
        qreal b = bottom + y;
        b = b > 1 ? 1 : b;
        top += (b - bottom);
        bottom = b;
    }
    else 
    {
        qreal t = top + y;
        t = t < 0 ? 0 : t;
        bottom += (t - top);
        top = t;
    }

    roiRect.setTopLeft(QPointF(left, top));
    roiRect.setBottomRight(QPointF(right, bottom));

    update();
}
