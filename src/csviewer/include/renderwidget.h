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

#ifndef _CS_RENDERWIDGET2D_H
#define _CS_RENDERWIDGET2D_H
#include <QFrame>
#include <QLabel>
#include <QImage>
#include <QPoint>
#include <QRectF>
#include <QTime>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>

#include <osgQOpenGL/osgQOpenGLWidget>
#include <osgViewer/Viewer>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <cstypes.h>
#include <hpp/Processing.hpp>

class RenderWidget : public QWidget
{
    Q_OBJECT
public:
    RenderWidget(int renderId, QWidget* parent = nullptr)
        : QWidget(parent)
        , renderId(renderId)
    {
        setAttribute(Qt::WA_StyledBackground);
    }
    int getRenderId() const 
    { 
        return renderId; 
    }
protected:
    int renderId;
};


class RenderWidget2D : public RenderWidget
{
    Q_OBJECT
public:
    RenderWidget2D(int renderId, QWidget* parent = nullptr);
    ~RenderWidget2D();
    void setWHRatio(float ratio);
   
    void setShowFullScreen(bool value);
    void setEnableScale(bool enable);

    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void onTranslate();
public slots:
    void onRenderDataUpdated(OutputData2D outputData);
protected:
    void initWidget();
    void onFrameIncrease();
    void updateImageSize();
    virtual void onPainterInfos(OutputData2D outputData);
    void updateFps();
    void initButtons();
private:
    void onClickExitButton();
    void onClickFullScreen(bool checked);
signals:
    void renderExit(int renderId);
    void fullScreenUpdated(int renderID, bool value);
protected:
    QScrollArea* scrollArea;
    QWidget* centerWidget;
    QWidget* topControlArea;
    QWidget* bottomControlArea = nullptr;

    QWidget* imageArea;
    QLabel* imageLabel;
    QLabel* fpsLabel;

    QPushButton* exitButton = nullptr;
    QPushButton* fullScreenBtn = nullptr;

    float ratioWH = 1.6f;
    QPainter painter;

    // fps
    QTime frameTime;
    int frameCount;
    float fps;

    bool enableImageScale = true;
    float imageAreaScale = 1.0f;
    float imageAreaScaleMin = 1;
    float imageAreaScaleMax = 10;
    float imageScaleStep = 0.1;

    bool showFullScreen = false;
    bool holdCtrl = false;

    QImage cachedImage;
};

class DepthRenderWidget2D : public RenderWidget2D
{
    Q_OBJECT
public:
     DepthRenderWidget2D(int renderId, QWidget* parent = nullptr);
    ~DepthRenderWidget2D();

    void setShowCoord(bool show);
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
public slots:
    void onRoiEditStateChanged(bool edit);
    void onShowCoordChanged(bool show);
signals:
    void roiRectFUpdated(QRectF rect);
    void showCoordChanged(bool show, QPointF position = QPointF(-1.0f, -1.0f));
private:
    void onPainterInfos(OutputData2D outputData) override;
private:
    QPointF mousePressPoint;
    QPointF mouseReleasePoint;
    bool isRoiEdit;
    bool isShowCoord;
};

class RenderWidget3D : public RenderWidget
{
    Q_OBJECT
public:
    RenderWidget3D(int renderId, QWidget* parent = nullptr);
    ~RenderWidget3D();

    void onTranslate();
    void setTextureEnable(bool enable);
public slots:
    void onRenderDataUpdated(cs::Pointcloud& pointCloud, const QImage& image);
protected slots:
    void initWindow();
    void resizeEvent(QResizeEvent* event) override;

private:
    void initNode();
    void updateNodeVertexs(cs::Pointcloud& pointCloud);
    void updateNodeTexture(cs::Pointcloud& pointCloud, const QImage& image);
    void refresh();
    void updateButtonArea();
    void initButtons();
    osg::ref_ptr<osg::MatrixTransform> makeCoordinate();
signals:
    void show3DTextureChanged(bool texture);
private:
    osgQOpenGLWidget* osgQOpenGLWidgetPtr;
    osg::ref_ptr<osg::Group> rootNode;
    osg::ref_ptr<osg::MatrixTransform> sceneNode;
    osg::ref_ptr<osg::Material> material;
    osg::ref_ptr<osg::Geometry> geom;

    QPushButton* homeButton;
    QPushButton* textureButton;
    QWidget* buttonArea;

    bool isReady = false;
};
#endif // _CS_RENDERWIDGET2D_H