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

#include "renderwidget.h"

#include <QtCore/QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QTime>

RenderWidget2D::RenderWidget2D(int renderId, QWidget* parent)
    : QFrame(parent)
    , centerWidget(new QWidget(this))
    , topControlArea(new QWidget(centerWidget))
    , imageLabel(new QLabel(centerWidget))
    , fpsLabel(new  QLabel(centerWidget))
    , renderId(renderId)
    , frameCount(0)
    , fps(0)
{
    initFrame();
}

RenderWidget2D::~RenderWidget2D()
{

}

void RenderWidget2D::initFrame()
{
    setProperty("isRender", true);
    imageLabel->setProperty("isImage", true);
    setMinimumSize(QSize(300, 180));

    QVBoxLayout* layout = new QVBoxLayout(centerWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(topControlArea);
    layout->addWidget(imageLabel);

    centerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    topControlArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    topControlArea->setFixedHeight(32);

    //top
    QHBoxLayout* hLayout = new QHBoxLayout(topControlArea);
    hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->setContentsMargins(0, 0, 5, 0);
    hLayout->setSpacing(10);
    hLayout->addWidget(fpsLabel);
    fpsLabel->setObjectName("fpsLabel");
    
    updateFps();
}

void RenderWidget2D::onFrameIncrease()
{
    if (frameCount == 0)
    {
        frameTime.restart();
        frameCount++;
        return;
    }

    int msec = frameTime.elapsed();
    if (msec > 1000)
    {
        fps = frameCount * 1.0f / (msec / 1000.0f);

        frameTime.restart();
        frameCount = 0;
    }

    frameCount++;
}

void RenderWidget2D::onRenderDataUpdated(OutputData2D outputData)
{
    QPixmap pixmap = QPixmap::fromImage(outputData.image);
    pixmap = pixmap.scaled(imageLabel->size());
    painter.begin(&pixmap);
    //painter
    onPainterInfos(outputData);
    updateFps();

    painter.end();
    imageLabel->setPixmap(pixmap);
    onFrameIncrease();
}

void RenderWidget2D::onPainterInfos(OutputData2D outputData)
{
    /*
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::yellow, 1));

    QRect rect = imageLabel->rect();
    rect.adjust(0, 10, -10, 0);
    QString text = QString("fps : %1").arg(QString::number(fps, 'f', 2));
    painter.drawText(rect, Qt::AlignRight | Qt::AlignTop, text);
    */
}

void RenderWidget2D::updateFps()
{
    QString text = QString("FPS : %1").arg(QString::number(fps, 'f', 2));
    fpsLabel->setText(tr(text.toStdString().c_str()));
}

void RenderWidget2D::setWHRatio(float ratio)
{
    ratioWH = ratio;
    updateImageSize();
}

void RenderWidget2D::resizeEvent(QResizeEvent* event)
{
    updateImageSize();
}

void RenderWidget2D::updateImageSize()
{
    const int w1 = width();
    const int topH = topControlArea->height();
    const int h1 = height() - topH;
    
    int imgW, imgH;
    if (w1 / ratioWH < h1)
    {
        imgW = w1;
        imgH = w1 / ratioWH;
    }
    else
    {
        imgH = h1;
        imgW = h1 * ratioWH;
    }

    int x = (w1 - imgW) / 2;
    int y = (height() - imgH - topH) / 2;

    imageLabel->setGeometry(0, topH, imgW, imgH);
    centerWidget->setGeometry(x, y, imgW, topH + imgH);
}

void RenderWidget2D::onTranslate()
{

}

int RenderWidget2D::getRenderId()
{
    return renderId;
}

DepthRenderWidget2D::DepthRenderWidget2D(int renderId, QWidget* parent)
    : RenderWidget2D(renderId, parent)
    , mousePressPoint(0,0)
    , mouseReleasePoint(0, 0)
    , isRoiEdit(false)
    , isShowCoord(false)
{

}

DepthRenderWidget2D::~DepthRenderWidget2D()
{

}

void DepthRenderWidget2D::mouseMoveEvent(QMouseEvent* event)
{
    if (isRoiEdit)
    {
        QPoint pt = imageLabel->mapFromGlobal(event->globalPos());
        mouseReleasePoint.rx() = (pt.x() * 1.0f) / imageLabel->width();
        mouseReleasePoint.ry() = (pt.y() * 1.0f) / imageLabel->height();
    }
}

void DepthRenderWidget2D::mousePressEvent(QMouseEvent* event)
{
    QPoint pt = imageLabel->mapFromGlobal(event->globalPos());
    const auto rect = imageLabel->rect();

    mousePressPoint.rx() = (pt.x() * 1.0f) / imageLabel->width();
    mousePressPoint.ry() = (pt.y() * 1.0f) / imageLabel->height();

    setShowCoord(!isRoiEdit && rect.contains(pt));
}

template<class T>
static void valueCorrect(T& v, T min, T max)
{
    v = (v < min) ? min : v;
    v = (v > max) ? max : v;
}

void DepthRenderWidget2D::mouseReleaseEvent(QMouseEvent* event)
{
    if (isRoiEdit && (mousePressPoint != mouseReleasePoint))
    {
        auto x1 = mousePressPoint.x();
        auto y1 = mousePressPoint.y();

        auto x2 = mouseReleasePoint.x();
        auto y2 = mouseReleasePoint.y();
        
        valueCorrect<qreal>(x1, 0, 1);
        valueCorrect<qreal>(x2, 0, 1);
        valueCorrect<qreal>(y1, 0, 1);
        valueCorrect<qreal>(y2, 0, 1);
        
        qreal minx = (x1 < x2) ? x1 : x2;
        qreal miny = (y1 < y2) ? y1 : y2;
        qreal w = qAbs((x1 - x2));
        qreal h = qAbs((y1 - y2));

        QRectF rect(minx, miny, w, h);
        
        emit roiRectFUpdated(rect);
    }
}

void DepthRenderWidget2D::onPainterInfos(OutputData2D outputData)
{
    RenderWidget2D::onPainterInfos(outputData);

    // draw ROI rect
    if (isRoiEdit && mousePressPoint != mouseReleasePoint)
    {
        painter.setPen(QPen(Qt::red, 2));

        QPoint p1 = QPoint(mousePressPoint.x() * imageLabel->width(), mousePressPoint.y() * imageLabel->height());
        QPoint p2 = QPoint(mouseReleasePoint.x() * imageLabel->width(), mouseReleasePoint.y() * imageLabel->height());;
        painter.drawRect(QRect(p1, p2));
    }

    // draw point information
    if (isShowCoord)
    {
        painter.setPen(QPen(Qt::yellow, 2));
        QPoint p = QPoint(mousePressPoint.x() * imageLabel->width(), mousePressPoint.y() * imageLabel->height());
        painter.drawLine(p.x(), p.y() - 2, p.x(), p.y() - 5);
        painter.drawLine(p.x(), p.y() + 2, p.x(), p.y() + 5);
        painter.drawLine(p.x() - 2, p.y(), p.x() - 5, p.y());
        painter.drawLine(p.x() + 2, p.y(), p.x() + 5, p.y());

        painter.setPen(QPen(Qt::yellow, 1));
        //QFont font;
        //font.setPointSizeF(12);
        //painter.setFont(font);

        auto v = outputData.info.vertex;
        auto depthScale = outputData.info.depthScale;

        QString text = QString("Depth Scale : %4\nXYZ(MM) : [%1, %2, %3]")
            .arg(QString::number(v.x(), 'f', 2))
            .arg(QString::number(v.y(), 'f', 2))
            .arg(QString::number(v.z(), 'f', 2))
            .arg(QString::number(depthScale, 'f', 3));

        QRect rect = imageLabel->rect();
        rect.adjust(10, 0, 0, -10);

        painter.drawText(rect, Qt::AlignLeft | Qt::AlignBottom, text);
    }
}
void DepthRenderWidget2D::onRoiEditStateChanged(bool edit)
{
    isRoiEdit = edit;
    if (isRoiEdit)
    {
        setShowCoord(false);
    }

    mousePressPoint = mouseReleasePoint = QPoint(0, 0);
}

void DepthRenderWidget2D::onShowCoordChanged(bool show)
{
    isShowCoord = show;
}

void DepthRenderWidget2D::setShowCoord(bool show)
{
    isShowCoord = show;
    if (isShowCoord)
    {
        emit showCoordChanged(isShowCoord, mousePressPoint);
    }
    else 
    {
        emit showCoordChanged(isShowCoord);
    }  
}
