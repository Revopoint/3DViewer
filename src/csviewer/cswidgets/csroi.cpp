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
    , m_roiRect(QRectF(0.0, 0.0, 1.0, 1.0))
    , m_roiOffset(10, 10, 10, 10)
{
    setAttribute(Qt::WA_StyledBackground, true);
    //setVisible(false);
    m_roiRectLast = m_roiRect;

    m_buttonArea = new QWidget(this);
    m_buttonArea->setObjectName("RoiButtonArea");
    QHBoxLayout* hlayout = new QHBoxLayout(m_buttonArea);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSpacing(0);

    m_cancelButton = new QPushButton(m_buttonArea);
    m_okButton = new QPushButton(m_buttonArea);

    m_cancelButton->setObjectName("CancelButton");
    m_okButton->setObjectName("OkButton");

    QWidget* line = new QWidget(m_buttonArea);
    line->setObjectName("Line");
    line->setFixedHeight(15);
    line->setFixedWidth(1);

    hlayout->addWidget(m_cancelButton);
    hlayout->addWidget(line);
    hlayout->addWidget(m_okButton);

    connect(m_cancelButton, &QPushButton::clicked, this, [=]()
    {
        m_roiRect = m_roiRectLast;
        setVisible(false);
        emit roiVisialeChanged(false);
    });

    connect(m_okButton, &QPushButton::clicked, this, [=]()
    {
        m_roiRectLast = m_roiRect;
        emit roiValueUpdated(m_roiRect);

    });
}

CSROIWidget::~CSROIWidget()
{

}

void CSROIWidget::updateRoiRectF(QRectF rect)
{
    m_roiRect = rect;
}

void CSROIWidget::setOffset(QMargins offset)
{
    m_roiOffset = offset;
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
    int w = width() - m_roiOffset.left() - m_roiOffset.right();
    int h = height() - m_roiOffset.top() - m_roiOffset.bottom();

    int right = w * m_roiRect.right() + m_roiOffset.left();
    int bottom = h * m_roiRect.bottom() + m_roiOffset.top();

    int x = right - m_buttonArea->width();
    int y = bottom + 10;

    m_buttonArea->setGeometry(x, y, m_buttonArea->width(), m_buttonArea->height());
}

void CSROIWidget::drawRoi()
{
    QPainter painter(this);

    QColor color(0, 169, 255);
    painter.setPen(QPen(color, 2));

    int w = width() - m_roiOffset.left() - m_roiOffset.right();
    int h = height() - m_roiOffset.top() - m_roiOffset.bottom();

    painter.translate(m_roiOffset.left(), m_roiOffset.top());

    int left = w * m_roiRect.left();
    int top = h * m_roiRect.top();
    int right = w * m_roiRect.right();
    int bottom = h * m_roiRect.bottom();

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
    m_pressPositon = -1;
    m_lastPosition = QPoint(0, 0);

    //QWidget::mouseReleaseEvent(event);
}

void CSROIWidget::mousePressEvent(QMouseEvent* event)
{
    //QWidget::mousePressEvent(event);
    //QPoint pos = QCursor().pos();
    m_pressPositon = -1;

    int w = width() - m_roiOffset.left() - m_roiOffset.right();
    int h = height() - m_roiOffset.top() - m_roiOffset.bottom();

    int left = w * m_roiRect.left() + m_roiOffset.left();
    int top = h * m_roiRect.top() + m_roiOffset.top();
    int right = w * m_roiRect.right() + m_roiOffset.left();
    int bottom = h * m_roiRect.bottom() + m_roiOffset.top();

    static int width = 30;
    static int halfWidth = width / 2;

    QPoint pt = this->mapFromGlobal(event->globalPos());
    m_lastPosition = pt;

    if (left == right && top == bottom)
    {
        m_pressPositon = RIGHT_BOTTOM;
        return;
    }

    //left top
    int x = left - halfWidth; x = x < 0 ? 0 : x;
    int y =  top - halfWidth; y = y < 0 ? 0 : y;
    QRect rect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon = LEFT_TOP;
        return;
    }

    // center top
    x = (left + right) / 2 - halfWidth; x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon =CENTER_TOP;
        return;
    }

    // right top
    x = right - halfWidth; x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon = RIGHT_TOP;
        return;
    }

    // right center
    y = (top + bottom) / 2 - halfWidth; y = y < 0 ? 0 : y;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon =RIGHT_CENTER;
        return;
    }

    // left center
    x = left - halfWidth;x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon =LEFT_CENTER;
        return;
    }

    // left bottom
    y = bottom - halfWidth; y = y < 0 ? 0 : y;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon = LEFT_BOTTOM;
        return;
    }

    // center bottom
    x = (left + right) / 2 - halfWidth; x = x < 0 ? 0 : x;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon = CENTER_BOTTOM;
        return;
    }

    // right bottom
    x = right - halfWidth;
    rect = QRect(x, y, width, width);
    if(rect.contains(pt))
    {
        m_pressPositon = RIGHT_BOTTOM;
        return;
    }
}

void CSROIWidget::mouseMoveEvent(QMouseEvent* event)
{
     auto pos = event->pos();
     QPoint curPos = event->pos();
     QPoint dPos = curPos - m_lastPosition;

     //qDebug() << "pos x = " << pos.x() << ", y = " << pos.y();
     auto dx = (dPos.x()) * 1.0 / (width() - m_roiOffset.left() - m_roiOffset.right());
     auto dy = (dPos.y()) * 1.0 / (height() - m_roiOffset.top() - m_roiOffset.bottom());
     
     //roiRect
     m_lastPosition = curPos;
     if(m_pressPositon < 0)
     {
         translateRoi(dx, dy);
         return;
     }

     auto left = m_roiRect.left();
     auto top = m_roiRect.top();
     auto right = m_roiRect.right();
     auto bottom = m_roiRect.bottom();

     auto x = left;
     auto y = top;

     if(m_pressPositon == LEFT_TOP)
     {
        x += dx;
        y += dy;

        x = x > right ? right : x;
        x = x < 0 ? 0 : x;

        y = y < 0 ? 0 : y;
        y = y > bottom ? bottom : y;

        m_roiRect.setLeft(x);
        m_roiRect.setTop(y);
     }
     else if(m_pressPositon == LEFT_CENTER)
     {
         x += dx;

         x = x > right ? right : x;
         x = x < 0 ? 0 : x;

         m_roiRect.setLeft(x);
     }
     else if(m_pressPositon == LEFT_BOTTOM)
     {
         y = bottom;
         x += dx;
         y += dy;

         x = x > right ? right : x;
         x = x < 0 ? 0 : x;

         y = y > 1 ? 1 : y;
         y = y < top ? top : y;

         m_roiRect.setLeft(x);
         m_roiRect.setBottom(y);
     }
     else if(m_pressPositon == CENTER_TOP)
     {
         y += dy;
         y = y < 0 ? 0 : y;
         y = y > bottom ? bottom : y;
         m_roiRect.setTop(y);
     }
     else if(m_pressPositon == CENTER_BOTTOM)
     {
         y = bottom;
         y += dy;
         y = y > 1 ? 1: y;
         y = y < top ? top : y;
         m_roiRect.setBottom(y);
     }
     else if(m_pressPositon == RIGHT_TOP)
     {
         x = right;
         x += dx;
         y += dy;

         x = x > 1 ? 1 : x;
         x = x < left ? left : x;

         y = y < 0 ? 0 : y;
         y = y > bottom ? bottom : y;

         m_roiRect.setRight(x);
         m_roiRect.setTop(y);
     }
     else if(m_pressPositon == RIGHT_CENTER)
     {
         x = right;
         x += dx;

         x = x > 1 ? 1 : x;
         x = x < left? left : x;

         m_roiRect.setRight(x);
     }
     else if(m_pressPositon == RIGHT_BOTTOM)
     {
         x = right;
         y = bottom;
         x += dx;
         y += dy;

         x = x > 1 ? 1 : x;
         x = x < left ? left : x;

         y = y >1 ? 1 : y;
         y = y <top ? top : y;

         m_roiRect.setRight(x);
         m_roiRect.setBottom(y);
     }

     update();
}

void CSROIWidget::translateRoi(float x, float y)
{
    //qDebug() << "dx : " << x << ", dy : "<< y;
    auto left = m_roiRect.left();
    auto top = m_roiRect.top();
    auto right = m_roiRect.right();
    auto bottom = m_roiRect.bottom();

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

    m_roiRect.setTopLeft(QPointF(left, top));
    m_roiRect.setBottomRight(QPointF(right, bottom));

    update();
}
