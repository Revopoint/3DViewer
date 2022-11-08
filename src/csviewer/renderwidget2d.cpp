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
    , scrollArea(new QScrollArea(this))
    , centerWidget(new QWidget(this))
    , topControlArea(new QWidget(centerWidget))
    , imageArea(new QWidget(centerWidget))
    , imageLabel(new QLabel(imageArea))
    , fpsLabel(new  QLabel(imageArea))
    , frameCount(0)
    , fps(0)
{
    initWidget();
}

RenderWidget2D::~RenderWidget2D()
{

}

void RenderWidget2D::initWidget()
{
    setProperty("isRender", true);
    imageLabel->setProperty("isImage", true);
    centerWidget->setObjectName("RenderCenterWidget");

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->addWidget(topControlArea);
    rootLayout->addWidget(scrollArea);
    rootLayout->setContentsMargins(0, 0, 0, 10);
    rootLayout->setSpacing(0);

    scrollArea->setWidget(centerWidget);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    centerWidget->setGeometry(0, 0, scrollArea->width()* imageAreaScale, scrollArea->height() * imageAreaScale);
    setMinimumSize(QSize(300, 180));

    // image area
    QVBoxLayout* vLayout = new QVBoxLayout(imageArea);
    vLayout->setContentsMargins(5, 5, 5, 5 + bottomOffset);
    vLayout->setSpacing(0);
    vLayout->addWidget(imageLabel);

    ////top
    topControlArea->setProperty("TopControlArea", true);
    topControlArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout* hLayout = new QHBoxLayout(topControlArea);

    titleLabel = new QLabel(renderTitleMap[renderId], topControlArea);
    hLayout->addWidget(titleLabel);
    hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->setContentsMargins(15, 10, 15, 10);
    hLayout->setSpacing(10);

    hLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(fpsLabel);

    fpsLabel->setObjectName("fpsLabel");
    titleLabel->setVisible(false);

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
    if (outputData.image.isNull())
    {
        qWarning() << "render image is null";
        return;
    }

    if (isFirstFrame)
    {
        isFirstFrame = false;
        int width = outputData.image.width();
        int height = outputData.image.height();
        float ratio = width * 1.0 / height;
        setWHRatio(ratio);
    }

    cachedImage = outputData.image;

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

void RenderWidget2D::setEnableScale(bool enable)
{
    enableImageScale = enable;
}

void RenderWidget2D::setShowFullScreen(bool value)
{
    showFullScreen = value;
    if (showFullScreen)
    {
        initButtons();
    }

    titleLabel->setVisible(value);
}

void RenderWidget2D::hideRenderFps()
{
    fpsLabel->setVisible(false);
}

void RenderWidget2D::initButtons()
{
    if (!exitButton)
    {
        exitButton = new QPushButton(this);
        exitButton->setObjectName("ExitButton");
        exitButton->setIconSize(QSize(20, 20));
        exitButton->setIcon(QIcon(":/resources/fork_large.png"));

        connect(exitButton, &QPushButton::clicked, this, &RenderWidget2D::onClickExitButton);
    }

    if (!fullScreenBtn)
    {
        fullScreenBtn = new QPushButton(this);
        fullScreenBtn->setObjectName("FullScreenButton");
        fullScreenBtn->setCheckable(true);

        connect(fullScreenBtn, &QPushButton::toggled, this, &RenderWidget2D::onClickFullScreen);

        QIcon icon1;
        icon1.addFile(QStringLiteral(":/resources/full_screen_exit.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon1.addFile(QStringLiteral(":/resources/full_screen.png"), QSize(), QIcon::Active, QIcon::On);
        icon1.addFile(QStringLiteral(":/resources/full_screen.png"), QSize(), QIcon::Selected, QIcon::On);

        fullScreenBtn->setIconSize(QSize(22, 22));
        fullScreenBtn->setIcon(icon1);
    }

    auto layout = topControlArea->layout();
    if (layout)
    {
        layout->addWidget(exitButton);
    }

    if (!bottomControlArea)
    {
        bottomControlArea = new QWidget(this);

        QHBoxLayout* hLayout = new QHBoxLayout(bottomControlArea);
        hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
        hLayout->setContentsMargins(0, 0, 15, 10);
        hLayout->setSpacing(10);

        hLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        hLayout->addWidget(fullScreenBtn);

        auto rootLayout = this->layout();
        if (rootLayout)
        {
            rootLayout->addWidget(bottomControlArea);
        }
    }
}

void RenderWidget2D::onClickExitButton()
{
    emit renderExit(getRenderId());
}

void RenderWidget2D::onClickFullScreen(bool checked)
{
    enableImageScale = checked;
    if(!enableImageScale)
    {
        this->imageAreaScale = 1;
        updateImageSize();
    }
    emit fullScreenUpdated(getRenderId(), checked);
}

void RenderWidget2D::resizeEvent(QResizeEvent* event)
{
    updateImageSize();
}

void RenderWidget2D::wheelEvent(QWheelEvent* event)
{
    if (enableImageScale && holdCtrl)
    {
        QPoint numDegrees = event->angleDelta() / 8;

        //qDebug() << "angleDelta, x= " << numDegrees.x() << ", y = " << numDegrees.y();
        if (!numDegrees.isNull())
        {
            QPoint numSteps = numDegrees / 15;
            float v = imageScaleStep * numSteps.y();

            float lastScale = imageAreaScale;
            imageAreaScale += v;
            imageAreaScale = (imageAreaScale < imageAreaScaleMin) ? imageAreaScaleMin : imageAreaScale;
            imageAreaScale = (imageAreaScale > imageAreaScaleMax) ? imageAreaScaleMax : imageAreaScale;

            if (qAbs(imageAreaScale - lastScale) > 0.0000001)
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
        holdCtrl = true;
    }

    RenderWidget::keyPressEvent(event);
}

void RenderWidget2D::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Control)
    {
        holdCtrl = false;
    }

    RenderWidget::keyPressEvent(event);
}

void RenderWidget2D::updateImageSize()
{
    int width = scrollArea->width();
    int height = scrollArea->height();

    if (imageAreaScale < 1)
    {
        int scaleW = width * imageAreaScale;
        int scaleH = width * imageAreaScale;
        int x = (width - scaleW) / 2;
        int y = (height - scaleH) / 2;

        centerWidget->setGeometry(x, y, scaleW, scaleH);
    }
    else 
    {
        centerWidget->setGeometry(0, 0, width * imageAreaScale, height * imageAreaScale);
    }
    const int w1 = centerWidget->width();
    const int h1 = centerWidget->height() - bottomOffset;
    
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
    int y = (h1 - imgH) / 2;

    //imageLabel->setGeometry(0, 0, imgW, imgH);
    imageArea->setGeometry(x, y, imgW, imgH + bottomOffset);
    auto layout = imageArea->layout();
    layout->setContentsMargins(5, 5, 5, 5 + bottomOffset);

    if (!cachedImage.isNull())
    {
        QPixmap pixmap = QPixmap::fromImage(cachedImage);
        pixmap = pixmap.scaled(imageLabel->size());
        imageLabel->setPixmap(pixmap);
    }
}

void RenderWidget2D::onTranslate()
{

}

DepthRenderWidget2D::DepthRenderWidget2D(int renderId, QWidget* parent)
    : RenderWidget2D(renderId, parent)
    , mousePressPoint(0,0)
    , isRoiEdit(false)
    , isShowCoord(false)
    , roiWidget(new CSROIWidget(centerWidget))
{
    roiWidget->setObjectName("ROIWidget");

    QMargins margins = imageArea->layout()->contentsMargins();
    margins.setBottom(margins.bottom() + roiWidget->getButtonAreaHeight());

    roiWidget->setOffset(margins);
    roiWidget->setVisible(false);
    connect(roiWidget, &CSROIWidget::roiValueUpdated, this, &DepthRenderWidget2D::roiRectFUpdated); 
    connect(roiWidget, &CSROIWidget::roiVisialeChanged, this, [=](bool visible)
        {
            bottomOffset = visible ? roiWidget->getButtonAreaHeight() : 0;
            updateImageSize();
        });
}

DepthRenderWidget2D::~DepthRenderWidget2D()
{

}

void DepthRenderWidget2D::mousePressEvent(QMouseEvent* event)
{
    QPoint pt = imageLabel->mapFromGlobal(event->globalPos());
    const auto rect = imageLabel->rect();

    mousePressPoint.rx() = (pt.x() * 1.0f) / imageLabel->width();
    mousePressPoint.ry() = (pt.y() * 1.0f) / imageLabel->height();

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
    if(isRoiEdit)
    {
        roiWidget->update();
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
void DepthRenderWidget2D::onRoiEditStateChanged(bool edit, QRectF rect)
{
    isRoiEdit = edit;

    roiWidget->updateRoiRectF(rect);
    roiWidget->setVisible(isRoiEdit);

    bottomOffset = isRoiEdit ? roiWidget->getButtonAreaHeight() : 0;
    updateImageSize();
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

void DepthRenderWidget2D::updateImageSize()
{
    RenderWidget2D::updateImageSize();

    QRect imageAreaRect = imageArea->geometry();
    roiWidget->setGeometry(imageAreaRect);
}
