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

#include "renderwidget.h"

#include <QtCore/QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include <QTime>

#include "cswidgets/csroi.h"

static QMap<int, QString> renderTitleMap =
{
    {(int)CAMERA_DATA_L, "IR(L)"},
    {(int)CAMERA_DATA_R, "IR(R)"},
    {(int)CAMERA_DATA_DEPTH, "Depth"},
    {(int)CAMERA_DATA_RGB, "RGB"},
    {(int)CAMERA_DATA_POINT_CLOUD, "Point Cloud"},
};

RenderWidget2D::RenderWidget2D(int renderId, QWidget* parent)
    : RenderWidget(renderId, parent)
    , m_scrollArea(new QScrollArea(this))
    , m_centerWidget(new QWidget(this))
    , m_topControlArea(new QWidget(m_centerWidget))
    , m_imageArea(new QWidget(m_centerWidget))
    , m_imageLabel(new QLabel(m_imageArea))
    , m_fpsLabel(new  QLabel(m_imageArea))
    , m_frameCount(0)
    , m_fps(0)
{
    initWidget();
}

RenderWidget2D::~RenderWidget2D()
{

}

void RenderWidget2D::initWidget()
{
    setProperty("isRender", true);
    m_imageLabel->setProperty("isImage", true);
    m_centerWidget->setObjectName("RenderCenterWidget");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->addWidget(m_topControlArea);
    rootLayout->addWidget(m_scrollArea);
    rootLayout->setContentsMargins(0, 0, 0, 10);
    rootLayout->setSpacing(0);

    m_scrollArea->setWidget(m_centerWidget);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_centerWidget->setGeometry(0, 0, m_scrollArea->width()* m_imageAreaScale, m_scrollArea->height() * m_imageAreaScale);
    setMinimumSize(QSize(300, 180));

    // image area
    QVBoxLayout* vLayout = new QVBoxLayout(m_imageArea);
    vLayout->setContentsMargins(5, 5, 5, 5 + m_bottomOffset);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_imageLabel);

    ////top
    m_topControlArea->setProperty("TopControlArea", true);
    m_topControlArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout* hLayout = new QHBoxLayout(m_topControlArea);

    m_titleLabel = new QLabel(renderTitleMap[m_renderId], m_topControlArea);
    hLayout->addWidget(m_titleLabel);
    hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->setContentsMargins(15, 10, 15, 10);
    hLayout->setSpacing(10);

    hLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(m_fpsLabel);

    m_fpsLabel->setObjectName("fpsLabel");
    m_titleLabel->setVisible(false);

    updateFps();
}

void RenderWidget2D::onFrameIncrease()
{
    if (m_frameCount == 0)
    {
        m_frameTime.restart();
        m_frameCount++;
        return;
    }

    int msec = m_frameTime.elapsed();
    if (msec > 1000)
    {
        m_fps = m_frameCount * 1.0f / (msec / 1000.0f);

        m_frameTime.restart();
        m_frameCount = 0;
    }

    m_frameCount++;
}

void RenderWidget2D::onRenderDataUpdated(OutputData2D outputData)
{
    if (outputData.image.isNull())
    {
        qWarning() << "render image is null";
        return;
    }

    if (m_isFirstFrame)
    {
        m_isFirstFrame = false;
        int width = outputData.image.width();
        int height = outputData.image.height();
        float ratio = width * 1.0 / height;
        setWHRatio(ratio);
    }

    m_cachedImage = outputData.image;

    QPixmap pixmap = QPixmap::fromImage(outputData.image);
    pixmap = pixmap.scaled(m_imageLabel->size());
    m_painter.begin(&pixmap);
    //painter
    onPainterInfos(outputData);
    updateFps();

    m_painter.end();
    m_imageLabel->setPixmap(pixmap);
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
    QString text = QString("FPS : %1").arg(QString::number(m_fps, 'f', 2));
    m_fpsLabel->setText(tr(text.toStdString().c_str()));
}

void RenderWidget2D::setWHRatio(float ratio)
{
    m_ratioWH = ratio;
    updateImageSize();
}

void RenderWidget2D::setEnableScale(bool enable)
{
    m_enableImageScale = enable;
}

void RenderWidget2D::setShowFullScreen(bool value)
{
    m_showFullScreen = value;
    if (m_showFullScreen)
    {
        initButtons();
    }

    m_titleLabel->setVisible(value);
}

void RenderWidget2D::hideRenderFps()
{
    m_fpsLabel->setVisible(false);
}

void RenderWidget2D::initButtons()
{
    if (!m_exitButton)
    {
        m_exitButton = new QPushButton(this);
        m_exitButton->setObjectName("ExitButton");
        m_exitButton->setIconSize(QSize(20, 20));
        m_exitButton->setIcon(QIcon(":/resources/fork_large.png"));
        m_exitButton->setFocusPolicy(Qt::NoFocus);

        connect(m_exitButton, &QPushButton::clicked, this, &RenderWidget2D::onClickExitButton);
    }

    if (!m_fullScreenBtn)
    {
        m_fullScreenBtn = new QPushButton(this);
        m_fullScreenBtn->setObjectName("FullScreenButton");
        m_fullScreenBtn->setCheckable(true);
        m_fullScreenBtn->setFocusPolicy(Qt::NoFocus);

        connect(m_fullScreenBtn, &QPushButton::toggled, this, &RenderWidget2D::onClickFullScreen);

        QIcon icon1;
        icon1.addFile(QStringLiteral(":/resources/full_screen_exit.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon1.addFile(QStringLiteral(":/resources/full_screen.png"), QSize(), QIcon::Active, QIcon::On);
        icon1.addFile(QStringLiteral(":/resources/full_screen.png"), QSize(), QIcon::Selected, QIcon::On);

        m_fullScreenBtn->setIconSize(QSize(22, 22));
        m_fullScreenBtn->setIcon(icon1);
    }

    auto layout = m_topControlArea->layout();
    if (layout)
    {
        layout->addWidget(m_exitButton);
    }

    if (!m_bottomControlArea)
    {
        m_bottomControlArea = new QWidget(this);

        QHBoxLayout* hLayout = new QHBoxLayout(m_bottomControlArea);
        hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
        hLayout->setContentsMargins(0, 0, 15, 10);
        hLayout->setSpacing(10);

        hLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        hLayout->addWidget(m_fullScreenBtn);

        auto rootLayout = this->layout();
        if (rootLayout)
        {
            rootLayout->addWidget(m_bottomControlArea);
        }
    }
}

void RenderWidget2D::onClickExitButton()
{
    emit renderExit(getRenderId());
}

void RenderWidget2D::onClickFullScreen(bool checked)
{
    m_enableImageScale = checked;
    if(!m_enableImageScale)
    {
        this->m_imageAreaScale = 1;
        updateImageSize();
    }
    emit fullScreenUpdated(getRenderId(), checked);
    // notify to update
    updateGeometry();
}

void RenderWidget2D::resizeEvent(QResizeEvent* event)
{
    updateImageSize();
}

void RenderWidget2D::wheelEvent(QWheelEvent* event)
{
    if (m_enableImageScale && m_holdCtrl)
    {
        QPoint numDegrees = event->angleDelta() / 8;

        //qDebug() << "angleDelta, x= " << numDegrees.x() << ", y = " << numDegrees.y();
        if (!numDegrees.isNull())
        {
            QPoint numSteps = numDegrees / 15;
            float v = m_imageScaleStep * numSteps.y();

            float lastScale = m_imageAreaScale;
            m_imageAreaScale += v;
            m_imageAreaScale = (m_imageAreaScale < m_imageAreaScaleMin) ? m_imageAreaScaleMin : m_imageAreaScale;
            m_imageAreaScale = (m_imageAreaScale > m_imageAreaScaleMax) ? m_imageAreaScaleMax : m_imageAreaScale;

            if (qAbs(m_imageAreaScale - lastScale) > 0.0000001)
            {
                updateImageSize();
            }
        }
    }
    RenderWidget::wheelEvent(event);
}

void RenderWidget2D::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        m_holdCtrl = true;
    }

    RenderWidget::keyPressEvent(event);
}

void RenderWidget2D::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        m_holdCtrl = false;
    }

    RenderWidget::keyPressEvent(event);
}

void RenderWidget2D::updateImageSize()
{
    int width = m_scrollArea->width();
    int height = m_scrollArea->height();

    if (m_imageAreaScale < 1)
    {
        int scaleW = width * m_imageAreaScale;
        int scaleH = width * m_imageAreaScale;
        int x = (width - scaleW) / 2;
        int y = (height - scaleH) / 2;

        m_centerWidget->setGeometry(x, y, scaleW, scaleH);
    }
    else 
    {
        m_centerWidget->setGeometry(0, 0, width * m_imageAreaScale, height * m_imageAreaScale);
    }
    const int w1 = m_centerWidget->width();
    const int h1 = m_centerWidget->height() - m_bottomOffset;
    
    int imgW, imgH;
    if (w1 / m_ratioWH < h1)
    {
        imgW = w1;
        imgH = w1 / m_ratioWH;
    }
    else
    {
        imgH = h1;
        imgW = h1 * m_ratioWH;
    }

    int x = (w1 - imgW) / 2;
    int y = (h1 - imgH) / 2;

    //imageLabel->setGeometry(0, 0, imgW, imgH);
    m_imageArea->setGeometry(x, y, imgW, imgH + m_bottomOffset);
    auto layout = m_imageArea->layout();
    layout->setContentsMargins(5, 5, 5, 5 + m_bottomOffset);

    if (!m_cachedImage.isNull())
    {
        QPixmap pixmap = QPixmap::fromImage(m_cachedImage);
        pixmap = pixmap.scaled(m_imageLabel->size());
        m_imageLabel->setPixmap(pixmap);
    }
}

void RenderWidget2D::onTranslate()
{

}

DepthRenderWidget2D::DepthRenderWidget2D(int renderId, QWidget* parent)
    : RenderWidget2D(renderId, parent)
    , m_mousePressPoint(0,0)
    , m_isRoiEdit(false)
    , m_isShowCoord(false)
    , m_roiWidget(new CSROIWidget(m_centerWidget))
{
    m_roiWidget->setObjectName("ROIWidget");

    QMargins margins = m_imageArea->layout()->contentsMargins();
    margins.setBottom(margins.bottom() + m_roiWidget->getButtonAreaHeight());

    m_roiWidget->setOffset(margins);
    m_roiWidget->setVisible(false);
    connect(m_roiWidget, &CSROIWidget::roiValueUpdated, this, &DepthRenderWidget2D::roiRectFUpdated); 
    connect(m_roiWidget, &CSROIWidget::roiVisialeChanged, this, [=](bool visible)
        {
            m_bottomOffset = visible ? m_roiWidget->getButtonAreaHeight() : 0;
            updateImageSize();
        });
}

DepthRenderWidget2D::~DepthRenderWidget2D()
{

}

void DepthRenderWidget2D::mousePressEvent(QMouseEvent* event)
{
    QPoint pt = m_imageLabel->mapFromGlobal(event->globalPos());
    const auto rect = m_imageLabel->rect();

    m_mousePressPoint.rx() = (pt.x() * 1.0f) / m_imageLabel->width();
    m_mousePressPoint.ry() = (pt.y() * 1.0f) / m_imageLabel->height();

    setShowCoord(rect.contains(pt));
}

template<class T>
static void valueCorrect(T& v, T min, T max)
{
    v = (v < min) ? min : v;
    v = (v > max) ? max : v;
}

void DepthRenderWidget2D::onPainterInfos(OutputData2D outputData)
{
    RenderWidget2D::onPainterInfos(outputData);
    if(m_isRoiEdit)
    {
        m_roiWidget->update();
    }

    // draw point information
    if (m_isShowCoord)
    {
        m_painter.setPen(QPen(Qt::yellow, 2));
        QPoint p = QPoint(m_mousePressPoint.x() * m_imageLabel->width(), m_mousePressPoint.y() * m_imageLabel->height());
        m_painter.drawLine(p.x(), p.y() - 2, p.x(), p.y() - 5);
        m_painter.drawLine(p.x(), p.y() + 2, p.x(), p.y() + 5);
        m_painter.drawLine(p.x() - 2, p.y(), p.x() - 5, p.y());
        m_painter.drawLine(p.x() + 2, p.y(), p.x() + 5, p.y());

        m_painter.setPen(QPen(Qt::yellow, 1));
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

        QRect rect = m_imageLabel->rect();
        rect.adjust(10, 0, 0, -10);

        m_painter.drawText(rect, Qt::AlignLeft | Qt::AlignBottom, text);
    }
}
void DepthRenderWidget2D::onRoiEditStateChanged(bool edit, QRectF rect)
{
    m_isRoiEdit = edit;

    m_roiWidget->updateRoiRectF(rect);
    m_roiWidget->setVisible(m_isRoiEdit);

    m_bottomOffset = m_isRoiEdit ? m_roiWidget->getButtonAreaHeight() : 0;
    updateImageSize();
}

void DepthRenderWidget2D::onShowCoordChanged(bool show)
{
    m_isShowCoord = show;
}

void DepthRenderWidget2D::setShowCoord(bool show)
{
    m_isShowCoord = show;
    if (m_isShowCoord)
    {
        emit showCoordChanged(m_isShowCoord, m_mousePressPoint);
    }
    else 
    {
        emit showCoordChanged(m_isShowCoord);
    }  
}

void DepthRenderWidget2D::updateImageSize()
{
    RenderWidget2D::updateImageSize();

    QRect imageAreaRect = m_imageArea->geometry();
    m_roiWidget->setGeometry(imageAreaRect);
}
