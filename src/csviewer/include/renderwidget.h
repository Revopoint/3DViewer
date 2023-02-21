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
        , m_renderId(renderId)
    {
        setAttribute(Qt::WA_StyledBackground);
    }
    int getRenderId() const 
    { 
        return m_renderId; 
    }
    virtual void setShowFullScreen(bool value) {}
    virtual void onTranslate() {}
signals:
    void renderExit(int renderId);
    void fullScreenUpdated(int renderID, bool value);
protected:
    int m_renderId;
    bool m_showFullScreen = false;
};

class RenderWidget2D : public RenderWidget
{
    Q_OBJECT
public:
    RenderWidget2D(int renderId, QWidget* parent = nullptr);
    ~RenderWidget2D();

    void setEnableScale(bool enable);

    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void hideRenderFps();
    void setShowFullScreen(bool value) override;
    void onTranslate() override;
public slots:
    void onRenderDataUpdated(OutputData2D outputData);
protected:
    void initWidget();
    void onFrameIncrease();
    virtual void updateImageSize();
    virtual void onPainterInfos(OutputData2D outputData);
    void updateFps();
    void initButtons();
    void setWHRatio(float ratio);
private:
    void onClickExitButton();
    void onClickFullScreen(bool checked);
protected:
    QScrollArea* m_scrollArea;
    QWidget* m_centerWidget;
    QWidget* m_topControlArea;
    QWidget* m_bottomControlArea = nullptr;

    QWidget* m_imageArea;
    QLabel* m_imageLabel;
    QLabel* m_fpsLabel;

    QPushButton* m_exitButton = nullptr;
    QPushButton* m_fullScreenBtn = nullptr;
    QLabel* m_titleLabel = nullptr;

    float m_ratioWH = 1.6f;
    QPainter m_painter;

    // fps
    QTime m_frameTime;
    int m_frameCount;
    float m_fps;

    bool enableImageScale = true;
    float imageAreaScale = 1.0f;
    float imageAreaScaleMin = 1;
    float imageAreaScaleMax = 10;
    float imageScaleStep = 0.1;

    bool holdCtrl = false;

    QImage cachedImage;

    int bottomOffset = 0;
    bool isFirstFrame = true;
};

class CSROIWidget;
class DepthRenderWidget2D : public RenderWidget2D
{
    Q_OBJECT
public:
     DepthRenderWidget2D(int renderId, QWidget* parent = nullptr);
    ~DepthRenderWidget2D();

    void setShowCoord(bool show);
    void mousePressEvent(QMouseEvent* event) override;

    void updateImageSize() override;
public slots:
    void onRoiEditStateChanged(bool edit, QRectF rect);
    void onShowCoordChanged(bool show);
signals:
    void roiRectFUpdated(QRectF rect);
    void showCoordChanged(bool show, QPointF position = QPointF(-1.0f, -1.0f));
private:
    void onPainterInfos(OutputData2D outputData) override;
private:
    QPointF mousePressPoint;
    bool isRoiEdit;
    bool isShowCoord;

    CSROIWidget* roiWidget;
};

typedef enum Axis
{
    X,
    Y,
    Z,
    COUNT
} Axis;

class CSCustomCamera : public osg::Camera
{
public:
    CSCustomCamera();
    CSCustomCamera(CSCustomCamera const& copy, osg::CopyOp copyOp = osg::CopyOp::SHALLOW_COPY);
    virtual ~CSCustomCamera();
    META_Node(osg, CSCustomCamera);
    inline void setMainCamera(Camera* camera) { mainCamera = camera; }
    virtual void traverse(osg::NodeVisitor& nv);

    void setOrthoProjection(bool isOrtho);
    void setTran(osg::Vec3 tran);
private:
    void initCamera();
protected:
    osg::observer_ptr<Camera> mainCamera;
    bool isOrthoProjection = false;
    osg::Vec3 translate;
};

class RenderWidget3D : public RenderWidget
{
    Q_OBJECT
public:
    RenderWidget3D(int renderId, QWidget* parent = nullptr);
    ~RenderWidget3D();

    void onTranslate() override;
    void setTextureEnable(bool enable);
    void setShowFullScreen(bool value) override;
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
    osg::ref_ptr<osg::MatrixTransform> makeTrackball();
    osg::ref_ptr<osg::MatrixTransform> makeClock(int axis);
signals:
    void show3DTextureChanged(bool texture);
private:
    osgQOpenGLWidget* osgQOpenGLWidgetPtr;
    osg::ref_ptr<osg::Group> rootNode;
    osg::ref_ptr<osg::MatrixTransform> sceneNode;
    osg::ref_ptr<osg::Material> material;
    osg::ref_ptr<osg::Geometry> geom;
    osg::ref_ptr<CSCustomCamera> trackballCamera;

    QPushButton* homeButton;
    QPushButton* textureButton;
    QPushButton* exitButton;
    QPushButton* fullScreenBtn;
    QPushButton* trackballButton;

    QLabel* titlLabel;
    QWidget* topItem;
    QWidget* bottomItem;

    bool isReady = false;
    bool isFirstFrame = true;

    cs::Pointcloud lastPointCloud;
    QImage lastTextureImage;
};
#endif // _CS_RENDERWIDGET2D_H
