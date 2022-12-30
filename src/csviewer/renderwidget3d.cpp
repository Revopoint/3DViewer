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

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <osgViewer/Viewer>
#include <osg/Node>
#include <osg/Multisample>
#include <osg/LightModel>
#include <osgGA/TrackballManipulator>
#include <osg/MatrixTransform>
#include <osg/Point>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osg/Camera>
#include <osg/Referenced>
#include <osg/LineWidth>

#include <osg/ShapeDrawable>

#define AXIS_LEN  40
#define AXIS_RADIUS 3

CSCustomCamera::CSCustomCamera()
{
    initCamera();
}

CSCustomCamera::CSCustomCamera(CSCustomCamera const& copy, osg::CopyOp copyOp)
    : Camera(copy, copyOp)
    , mainCamera(copy.mainCamera)
{
    initCamera();
}

CSCustomCamera::~CSCustomCamera()
{
}

void CSCustomCamera::setOrthoProjection(bool isOrtho)
{
    isOrthoProjection = isOrtho;
}

void CSCustomCamera::setTran(osg::Vec3 tran)
{
    translate = tran;
}

void CSCustomCamera::traverse(osg::NodeVisitor& nv)
{
   osg::Viewport* viewPort =  mainCamera->getViewport();
   const int width = viewPort->width();
   const int height = viewPort->height();

   if (isOrthoProjection)
   {
       setProjectionMatrixAsOrtho(-width / 2, width / 2, -height / 2, height / 2, -1000.0, 1000.0);
       const int space = 20;      
       translate = osg::Vec3(width/2 - AXIS_LEN - space, -height/2 + AXIS_LEN + space, -AXIS_LEN);
   }
   else 
   {
       setProjectionMatrixAsPerspective(30.0f, static_cast<double>(viewPort->width()) / static_cast<double>(viewPort->height()), 1.0f, 10000.0f);
   }

   setViewport(0, 0, width, height);

    if (mainCamera.valid() && nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osg::Matrix matrix = mainCamera->getViewMatrix();
        matrix.setTrans(translate);

        this->setViewMatrix(matrix);
    }

    osg::Camera::traverse(nv);
}

void CSCustomCamera::initCamera()
{
    setRenderOrder(osg::Camera::POST_RENDER);
    setClearMask(GL_DEPTH_BUFFER_BIT);
    setAllowEventFocus(false);
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
}

RenderWidget3D::RenderWidget3D(int renderId, QWidget* parent)
    : RenderWidget(renderId, parent)
    , osgQOpenGLWidgetPtr(new osgQOpenGLWidget(this))
    , homeButton(new QPushButton(this))
    , textureButton(new QPushButton(this))
    , topItem(new QWidget(this))
    , bottomItem(new QWidget(this))
    , exitButton(new QPushButton(this))
{
    QHBoxLayout* pLayout = new QHBoxLayout(this);
    pLayout->setMargin(2);
    pLayout->addWidget(osgQOpenGLWidgetPtr);
    osgQOpenGLWidgetPtr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(QSize(300, 180));

    initButtons();
    updateButtonArea();

    bool suc = connect(osgQOpenGLWidgetPtr, SIGNAL(initialized()), this, SLOT(initWindow()));
    Q_ASSERT(suc);
}

RenderWidget3D::~RenderWidget3D()
{

}

void RenderWidget3D::resizeEvent(QResizeEvent* event)
{
    updateButtonArea();

    osgViewer::Viewer* pViewer = osgQOpenGLWidgetPtr->getOsgViewer();
    if(pViewer)
    {
        osg::ref_ptr<osg::Camera> camera = pViewer->getCamera();
        camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(osgQOpenGLWidgetPtr->width()) / static_cast<double>(osgQOpenGLWidgetPtr->height()), 1.0f, 10000.0f);
    }
}

void RenderWidget3D::initWindow()
{
    osgViewer::Viewer* pViewer = osgQOpenGLWidgetPtr->getOsgViewer();

    osg::ref_ptr<osg::Camera> camera = pViewer->getCamera();
    camera->setClearColor(osg::Vec4(242.0 / 255.0, 242.0 / 255, 242.0 / 255, 1.0));

    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setComputeNearFarMode(osg::Camera::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
    camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(osgQOpenGLWidgetPtr->width()) / static_cast<double>(osgQOpenGLWidgetPtr->height()), 1.0f, 10000.0f);

    //camera->setViewport(0, 0, width(), height());

    pViewer->setKeyEventSetsDone(0);
    pViewer->setQuitEventSetsDone(false);
    pViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    pViewer->setRunFrameScheme(osgViewer::ViewerBase::ON_DEMAND);

    rootNode = new osg::Group();
    rootNode->setName("rootNode");

    sceneNode = new osg::MatrixTransform();

    pViewer->setSceneData(rootNode);

    osg::Multisample* multi = new osg::Multisample;
    multi->setHint(osg::Multisample::NICEST);
    rootNode->getOrCreateStateSet()->setAttributeAndModes(multi, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    rootNode->getOrCreateStateSet()->setMode(GL_MULTISAMPLE_ARB, osg::StateAttribute::ON);

    osg::LightModel* lightModel = new osg::LightModel();
    lightModel->setTwoSided(false);
    lightModel->setLocalViewer(true);
    lightModel->setAmbientIntensity(osg::Vec4(0.4, 0.4, 0.4, 1.0));
    rootNode->getOrCreateStateSet()->setAttributeAndModes(lightModel);

    pViewer->getLight()->setAmbient(osg::Vec4(0.1, 0.1, 0.1, 1.0));
    pViewer->getLight()->setDiffuse(osg::Vec4(0.9, 0.9, 0.9, 1.0));
    pViewer->getLight()->setSpecular(osg::Vec4(0.3, 0.3, 0.3, 1.0));

    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator();
    pViewer->setCameraManipulator(manipulator);

    manipulator->setNode(sceneNode);
    manipulator->setTrackballSize(1000);
    manipulator->setAllowThrow(false);

    //rotate show
    osg::Matrix matrix;
    matrix.setRotate(osg::Quat(-osg::PI_2, osg::X_AXIS));
    sceneNode->setMatrix(matrix);

    rootNode->addChild(sceneNode);

    material = new osg::Material();
    material->setDiffuse(osg::Material::BACK, osg::Vec4(0.486, 0.486, 0.811, 1.0));
    material->setDiffuse(osg::Material::FRONT, osg::Vec4(23.0f / 255.f, 96.0f / 255.f, 133.0f / 255.f, 1.0));
    material->setColorMode(osg::Material::DIFFUSE);

    initNode();

    connect(homeButton, &QPushButton::clicked, this, [=]() 
        {
            pViewer->home();
        });

    osg::ref_ptr<osg::MatrixTransform> axis = makeCoordinate();

    osg::ref_ptr<CSCustomCamera> csAxes = new CSCustomCamera;
    csAxes->addChild(axis);
    csAxes->setMainCamera(pViewer->getCamera());
    csAxes->setOrthoProjection(true);

    rootNode->addChild(csAxes);

    // make trackball
    // set line property
    osg::ref_ptr<osg::LineWidth> lineSize = new osg::LineWidth;
    lineSize->setWidth(2.0);
    osg::ref_ptr<osg::StateSet> stateSet = rootNode->getOrCreateStateSet();
    stateSet->setAttributeAndModes(lineSize, osg::StateAttribute::ON);

    osg::ref_ptr<osg::MatrixTransform> trackball = makeTrackball();
    trackballCamera = new CSCustomCamera;
    trackballCamera->addChild(trackball);
    trackballCamera->setMainCamera(pViewer->getCamera());
    trackballCamera->setOrthoProjection(false);
    trackballCamera->setTran(osg::Vec3(0, 0, -500));

    rootNode->addChild(trackballCamera);

    osgQOpenGLWidgetPtr->mutex()->writeLock();
    isReady = true;
    osgQOpenGLWidgetPtr->mutex()->writeUnlock();
}

void RenderWidget3D::updateNodeVertexs(cs::Pointcloud& pointCloud)
{
    auto vertexArr = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    auto normalArr = dynamic_cast<osg::Vec3Array*>(geom->getNormalArray());
   
    const int num_ver = qMin(pointCloud.getNormals().size(), pointCloud.getVertices().size());
    vertexArr->resize(num_ver);
    normalArr->resize(num_ver);

    memcpy((void*)vertexArr->getDataPointer(), pointCloud.getVertices().data(), sizeof(cs::float3) * num_ver);
    memcpy((void*)normalArr->getDataPointer(), pointCloud.getNormals().data(), sizeof(cs::float3) * num_ver);

    if (geom->getNumPrimitiveSets() > 0)
    {
        geom->removePrimitiveSet(0);
    }

    geom->insertPrimitiveSet(0, new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, num_ver));
}

void RenderWidget3D::updateNodeTexture(cs::Pointcloud& pointCloud, const QImage& image)
{
    unsigned char* texFrame = (uchar*)image.bits();
    const int width = image.width();
    const int height = image.height();

    bool bTexture = (!image.isNull() && texFrame != nullptr && width > 0 && height > 0);
    if (!bTexture || !textureButton->isChecked())
    {
        osg::StateSet* ss = geom->getOrCreateStateSet();
        geom->setColorBinding(osg::Geometry::BIND_OFF);
        ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

        geom->getOrCreateStateSet()->setAttributeAndModes(material);
        return;
    }
    
    geom->getOrCreateStateSet()->removeAttribute(material);

    auto colorArr = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
    int numVer = qMin(pointCloud.getNormals().size(), pointCloud.getVertices().size());
    numVer = qMin(pointCloud.getTexcoords().size(), size_t(numVer));
    colorArr->resize(numVer);
    
    unsigned char* color;
    cs::float2* pcTexcoord = pointCloud.getTexcoords().data();
    osg::Vec4* osgColor = (osg::Vec4*)colorArr->getDataPointer();
   
    for (int i = 0; i < numVer; ++i)
    {
        int x = qRound(pcTexcoord[i].u * width);
        x = (x >= width) ? (width - 1) : x;

        int y = qRound(pcTexcoord[i].v * height);
        y = (y >= height) ? (height - 1) : y;

        color = texFrame + (y * width + x) * 3;
        osgColor[i] = std::move(osg::Vec4(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f, 1.f));
    }

    osg::StateSet* ss = geom->getOrCreateStateSet();
    geom->setColorArray(colorArr);
    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
}

void RenderWidget3D::onRenderDataUpdated(cs::Pointcloud& pointCloud, const QImage& image)
{
    lastPointCloud = pointCloud;
    lastTextureImage = image;

    osgQOpenGLWidgetPtr->mutex()->writeLock();
    if (!isReady)
    {
        osgQOpenGLWidgetPtr->mutex()->writeUnlock();
        return;
    }

    // update vertexs
    updateNodeVertexs(pointCloud);

    //update texture
    updateNodeTexture(pointCloud, image);

    //draw
    refresh();

    if (isFirstFrame)
    {
        osgViewer::Viewer* pViewer = osgQOpenGLWidgetPtr->getOsgViewer();
        if (pViewer)
        {
            pViewer->home();
        }

        isFirstFrame = false;
    }
    
    osgQOpenGLWidgetPtr->mutex()->writeUnlock();
}

void RenderWidget3D::refresh()
{
    osgViewer::Viewer* pViewer = osgQOpenGLWidgetPtr->getOsgViewer();
    static bool firstFlag = true;
    if (firstFlag)
    {
        pViewer->home();
        firstFlag = false;
    }
   
    geom->setUseVertexBufferObjects(true);
    geom->setUseDisplayList(false);

    geom->getVertexArray()->dirty();
    geom->getNormalArray()->dirty();
    geom->getColorArray()->dirty();

    pViewer->requestRedraw();
}

void RenderWidget3D::initNode()
{
    // vertex
    geom = new osg::Geometry;
    geom->setUseDisplayList(false);
    geom->setVertexArray(new osg::Vec3Array());

    // normal
    geom->setNormalArray(new osg::Vec3Array());
    geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    // texture
    geom->setColorArray(new osg::Vec4Array());

    osg::StateSet* ss = geom->getOrCreateStateSet();
    ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

    geom->setUseVertexBufferObjects(true);
    //set point size
    geom->getOrCreateStateSet()->setAttribute(new osg::Point(2));

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(geom);
    osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
    mt->addChild(geode);

    sceneNode->addChild(mt.release());
}

void RenderWidget3D::updateButtonArea()
{
    topItem->setGeometry(0, 0, width(), topItem->height());
    bottomItem->setGeometry(0, height()- bottomItem->height(), width(), bottomItem->height());
}

void RenderWidget3D::initButtons()
{
    topItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bottomItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    topItem->setFixedHeight(48);
    bottomItem->setFixedHeight(48);

    homeButton->setObjectName("homeButton");
    homeButton->setIconSize(QSize(28, 28));
    homeButton->setIcon(QIcon(":/resources/home.png"));
    homeButton->setToolTip(tr("Home"));

    textureButton->setCheckable(true);
    textureButton->setObjectName("textureButton");
    textureButton->setIconSize(QSize(28, 28));
    textureButton->setToolTip(tr("Texture on"));

    QIcon icon1;
    icon1.addFile(QStringLiteral(":/resources/texture_off.png"), QSize(), QIcon::Normal, QIcon::Off);
    icon1.addFile(QStringLiteral(":/resources/texture_on.png"), QSize(), QIcon::Active, QIcon::On);
    icon1.addFile(QStringLiteral(":/resources/texture_on.png"), QSize(), QIcon::Selected, QIcon::On);
    textureButton->setIcon(icon1);

    exitButton->setIconSize(QSize(22, 22));
    exitButton->setIcon(QIcon(":/resources/fork_large.png"));

    connect(exitButton, &QPushButton::clicked, this, [=]() {
            emit renderExit(renderId);
        });

    // trackball 
    trackballButton = new QPushButton(this);
    trackballButton->setObjectName("TrackballButton");
    trackballButton->setCheckable(true);
    trackballButton->setChecked(true);
    trackballButton->setToolTip(tr("Hide track ball"));

    QIcon icon3;
    icon3.addFile(QStringLiteral(":/resources/trackball_off.png"), QSize(), QIcon::Normal, QIcon::Off);
    icon3.addFile(QStringLiteral(":/resources/trackball_on.png"), QSize(), QIcon::Active, QIcon::On);
    icon3.addFile(QStringLiteral(":/resources/trackball_on.png"), QSize(), QIcon::Selected, QIcon::On);

    trackballButton->setIconSize(QSize(28, 28));
    trackballButton->setIcon(icon3);
    connect(trackballButton, &QPushButton::toggled, this,
        [=](bool checked)
        {
            if (!checked)
            {
                if (trackballCamera.valid())
                {
                    rootNode->removeChild(trackballCamera);
                }
                trackballButton->setToolTip(tr("Show track ball"));
            }
            else
            {
                if (trackballCamera.valid())
                {
                    rootNode->addChild(trackballCamera);
                }
                trackballButton->setToolTip(tr("Hide track ball"));
            }

            onRenderDataUpdated(lastPointCloud, lastTextureImage);
        });
    

    QHBoxLayout* layout = new QHBoxLayout(topItem);

    titlLabel = new QLabel("Point Cloud", topItem);
    layout->addWidget(titlLabel);
    layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(textureButton);
    layout->addWidget(trackballButton);
    layout->addWidget(homeButton);
    layout->addWidget(exitButton);

    layout->setContentsMargins(15, 10, 15, 10);

    connect(textureButton, &QPushButton::toggled, [=](bool toggled)
        {
            if (textureButton->isChecked())
            {
                textureButton->setToolTip(tr("Texture off"));
            }
            else
            {
                textureButton->setToolTip(tr("Texture on"));
            }

            onRenderDataUpdated(lastPointCloud, lastTextureImage);
            emit show3DTextureChanged(toggled);
        });

    // bottom item
    layout = new QHBoxLayout(bottomItem);
    layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->setContentsMargins(0, 0, 15, 10);

    fullScreenBtn = new QPushButton(bottomItem);
    fullScreenBtn->setObjectName("FullScreenButton");
    fullScreenBtn->setCheckable(true);

    connect(fullScreenBtn, &QPushButton::toggled, this, [=](bool checked) {
            emit fullScreenUpdated(renderId, checked);
        });

    QIcon icon2;
    icon2.addFile(QStringLiteral(":/resources/full_screen_exit.png"), QSize(), QIcon::Normal, QIcon::Off);
    icon2.addFile(QStringLiteral(":/resources/full_screen.png"), QSize(), QIcon::Active, QIcon::On);
    icon2.addFile(QStringLiteral(":/resources/full_screen.png"), QSize(), QIcon::Selected, QIcon::On);

    fullScreenBtn->setIconSize(QSize(22, 22));
    fullScreenBtn->setIcon(icon2);

    layout->addWidget(fullScreenBtn);

    fullScreenBtn->setVisible(false);
    titlLabel->setVisible(false);
    exitButton->setVisible(false);
}

void RenderWidget3D::onTranslate()
{
    if (textureButton->isChecked())
    {
        textureButton->setToolTip(tr("Texture off"));
    }
    else
    {
        textureButton->setToolTip(tr("Texture on"));
    }

    if (trackballButton)
    {
        if (!trackballButton->isChecked())
        {
            trackballButton->setToolTip(tr("Show track ball"));
        }
        else 
        {
            trackballButton->setToolTip(tr("Hide track ball"));
        }
    }

    homeButton->setToolTip(tr("Home"));
}

void RenderWidget3D::setTextureEnable(bool enable)
{
    //textureButton->setEnabled(enable);
    textureButton->setVisible(enable);
}

void RenderWidget3D::setShowFullScreen(bool value)
{
    fullScreenBtn->setVisible(value);
    titlLabel->setVisible(value);
    exitButton->setVisible(value);
}

osg::ref_ptr<osg::MatrixTransform> RenderWidget3D::makeCoordinate()
{
    osg::ref_ptr<osg::MatrixTransform> group = new osg::MatrixTransform();

    osg::ref_ptr<osg::TessellationHints> hits = new osg::TessellationHints;
    hits->setDetailRatio(10.0f);

    float len = AXIS_LEN;
    float radius = AXIS_RADIUS;

    osg::Vec4 blue = osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f);
    osg::Vec4 red = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
    osg::Vec4 green = osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f);

    osg::ref_ptr<osg::Sphere> sphere = new osg::Sphere(osg::Vec3(0, 0, 0), radius * 1.5);
    osg::ref_ptr<osg::ShapeDrawable> sphereDrawable = new osg::ShapeDrawable(sphere.get());
    sphereDrawable->setColor(osg::Vec4f(1.0, 1.0, 1.0, 1.0));
    group->addChild(sphereDrawable);

    for (int i = 0; i < 3; ++i) {
        osg::ref_ptr<osg::Cylinder> cylinder = new osg::Cylinder(osg::Vec3(0.0, 0.0, len / 2), radius / 2, len);
        osg::ref_ptr<osg::Cone> cone = new osg::Cone(osg::Vec3(0.0, 0.0, len), radius * 2, radius * 4);
        osg::ref_ptr<osg::ShapeDrawable> cylinderDrawable = new osg::ShapeDrawable(cylinder.get());
        osg::ref_ptr<osg::ShapeDrawable> coneDrawable = new osg::ShapeDrawable(cone.get());
        osg::ref_ptr<osgText::Text> pTextXAuxis = new osgText::Text;

        osg::ref_ptr<osg::MatrixTransform> axisTransform = new osg::MatrixTransform();

        axisTransform->addChild(cylinderDrawable);
        axisTransform->addChild(coneDrawable);
        axisTransform->addChild(pTextXAuxis);

        cylinderDrawable->setTessellationHints(hits);
        coneDrawable->setTessellationHints(hits);

        Axis axis = static_cast<Axis>(i);
        switch (axis) {
        case X:
            axisTransform->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0), osg::Y_AXIS));
            cylinderDrawable->setColor(red);
            coneDrawable->setColor(red);
            pTextXAuxis->setText("X");
            break;
        case Y:
            axisTransform->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0), osg::X_AXIS));
            cylinderDrawable->setColor(green);
            coneDrawable->setColor(green);
            pTextXAuxis->setText("Y");
            break;
        case Z:
            axisTransform->setMatrix(osg::Matrix::rotate(osg::inDegrees(90.0), osg::Z_AXIS));
            cylinderDrawable->setColor(blue);
            coneDrawable->setColor(blue);
            pTextXAuxis->setText("Z");
            break;
        }

        pTextXAuxis->setPosition(osg::Vec3(0.0f, 0.0f, len));
        pTextXAuxis->setAxisAlignment(osgText::Text::SCREEN);
        pTextXAuxis->setCharacterSize(14);
        pTextXAuxis->setColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

        group->addChild(axisTransform.get());
    }

    return group;
}

osg::ref_ptr<osg::MatrixTransform> RenderWidget3D::makeClock(int axis)
{
    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform();
    osg::ref_ptr<osg::Geometry> clockGeometry = new osg::Geometry;
    root->addChild(clockGeometry);

    osg::ref_ptr<osg::Vec3Array> allPoints = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

    osg::Matrix matrix;
    osg::Vec4f color;

    switch (axis)
    {
    case X:
        matrix.setRotate(osg::Quat(-osg::PI_2, osg::Z_AXIS));
        color = osg::Vec4f(1.0, 0, 0, 1.0);
        break;
    case Y:
        color = osg::Vec4f(0.0, 1.0, 0, 1.0);
        break;
    case Z:
        color = osg::Vec4f(0.0, 0, 1.0, 1.0);
        matrix.setRotate(osg::Quat(-osg::PI_2, osg::X_AXIS));
        break;
    default:
        break;
    }

    for (double i = 0.0; i < 6.28; i += 0.02) {
        colors->push_back(color);
        allPoints->push_back(osg::Vec3(50 * sin(i), -0.0, 50 * cos(i)));
    }

    clockGeometry->setVertexArray(allPoints);
    clockGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, allPoints->size()));

    clockGeometry->setColorArray(colors);
    clockGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    
    root->setMatrix(matrix);

    return root;
}

osg::ref_ptr<osg::MatrixTransform> RenderWidget3D::makeTrackball()
{
    osg::ref_ptr<osg::MatrixTransform> group = new osg::MatrixTransform();

    osg::ref_ptr<osg::MatrixTransform>  geo1 = makeClock(X);
    osg::ref_ptr<osg::MatrixTransform>  geo2 = makeClock(Y);
    osg::ref_ptr<osg::MatrixTransform>  geo3 = makeClock(Z);

    group->addChild(geo1);
    group->addChild(geo2);
    group->addChild(geo3);

    return group;
}