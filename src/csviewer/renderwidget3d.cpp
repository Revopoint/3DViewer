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

#include "renderwidget3d.h"

#include <QApplication>
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

#include <osg/ShapeDrawable>

#define AXIS_LEN  40
#define AXIS_RADIUS 3

typedef enum Axis
{
    X,
    Y,
    Z,
    COUNT
} Axis;

class CSAxis : public osg::Camera
{
public:
    CSAxis();
    CSAxis(CSAxis const& copy, osg::CopyOp copyOp = osg::CopyOp::SHALLOW_COPY);
    virtual ~CSAxis();
    META_Node(osg, CSAxis);
    inline void setMainCamera(Camera* camera) { mainCamera = camera; }
    virtual void traverse(osg::NodeVisitor& nv);
protected:
    osg::observer_ptr<Camera> mainCamera;
};

CSAxis::CSAxis()
{
}

CSAxis::CSAxis(CSAxis const& copy, osg::CopyOp copyOp)
    : Camera(copy, copyOp)
    , mainCamera(copy.mainCamera)
{
}

CSAxis::~CSAxis()
{
}

void CSAxis::traverse(osg::NodeVisitor& nv)
{
   osg::Viewport* viewPort =  mainCamera->getViewport();
   const int halfWidth = viewPort->width() / 2;
   const int halfHeight = viewPort->width() / 2;

   const int space = 20;

   this->setProjectionMatrixAsOrtho(-halfWidth, halfWidth, -halfHeight, halfHeight, -200.0, 200.0);
   osg::Vec3 trans(halfWidth- AXIS_LEN - space, -halfHeight + AXIS_LEN + space, -AXIS_LEN);

    if (mainCamera.valid() && nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        osg::Matrix matrix = mainCamera->getViewMatrix();
        matrix.setTrans(trans);

        this->setViewMatrix(matrix);
    }

    osg::Camera::traverse(nv);
}

RenderWidget3D::RenderWidget3D(QWidget* parent)
    : QWidget(parent)
    , osgQOpenGLWidgetPtr(new osgQOpenGLWidget(this))
    , homeButton(new QPushButton(this))
    , textureButton(new QPushButton(this))
    , exportButton(new QPushButton(this))
    , buttonArea(new QWidget(this))
{
    QHBoxLayout* pLayout = new QHBoxLayout(this);
    pLayout->setMargin(0);
    pLayout->addWidget(osgQOpenGLWidgetPtr);
    osgQOpenGLWidgetPtr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
}

void RenderWidget3D::initWindow()
{
    osgViewer::Viewer* pViewer = osgQOpenGLWidgetPtr->getOsgViewer();

    osg::ref_ptr<osg::Camera> camera = pViewer->getCamera();
    camera->setClearColor(osg::Vec4(229.0 / 255, 229.0 / 255, 229.0 / 255, 1.0));
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setComputeNearFarMode(osg::Camera::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
    //camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(width()) / static_cast<double>(height()), 1.0f, 10000.0f);

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
    manipulator->setTrackballSize(0.5);
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

    connect(homeButton, &QPushButton::clicked, this, [=]() {
            pViewer->home();
        });

    osg::ref_ptr<osg::MatrixTransform> axis = makeCoordinate();

    osg::ref_ptr<CSAxis> csAxes = new CSAxis;
    csAxes->addChild(axis);
    csAxes->setMainCamera(pViewer->getCamera());
    csAxes->setRenderOrder(osg::Camera::POST_RENDER);
    csAxes->setClearMask(GL_DEPTH_BUFFER_BIT);
    csAxes->setAllowEventFocus(false);
    csAxes->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    
    //coordAxis->setMatrix(matrix);

    rootNode->addChild(csAxes);
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
    unsigned char* tex_frame = (uchar*)image.bits();
    const int width = image.width();
    const int height = image.height();

    bool bTexture = (!image.isNull() && tex_frame != nullptr && width > 0 && height > 0);
    if (!bTexture)
    {
        osg::StateSet* ss = geom->getOrCreateStateSet();
        geom->setColorBinding(osg::Geometry::BIND_OFF);
        ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

        geom->getOrCreateStateSet()->setAttributeAndModes(material);
        return;
    }
    
    geom->getOrCreateStateSet()->removeAttribute(material);

    auto color_arr = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
    int num_ver = qMin(pointCloud.getNormals().size(), pointCloud.getVertices().size());
    num_ver = qMin(pointCloud.getTexcoords().size(), size_t(num_ver));
    color_arr->resize(num_ver);
    
    unsigned char* color;
    cs::float2* pcTexcoord = pointCloud.getTexcoords().data();
    osg::Vec4* osgColor = (osg::Vec4*)color_arr->getDataPointer();
   
    for (int i = 0; i < num_ver; ++i)
    {
        int x = qRound(pcTexcoord[i].u * width);
        x = (x >= width) ? (width - 1) : x;

        int y = qRound(pcTexcoord[i].v * height);
        y = (y >= height) ? (height - 1) : y;

        color = tex_frame + (y * width + x) * 3;
        osgColor[i] = std::move(osg::Vec4(color[0] / 255.f, color[1] / 255.f, color[2] / 255.f, 1.f));
    }

    osg::StateSet* ss = geom->getOrCreateStateSet();
    geom->setColorArray(color_arr);
    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    ss->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
}

void RenderWidget3D::onRenderDataUpdated(cs::Pointcloud& pointCloud, const QImage& image)
{
    osgQOpenGLWidgetPtr->mutex()->writeLock();

    // update vertexs
    updateNodeVertexs(pointCloud);

    //update texture
    updateNodeTexture(pointCloud, image);

    //draw
    refresh();

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
    const int offset = 12;
    const int x = width() - offset - buttonArea->width();
    const int y = offset;

    buttonArea->setGeometry(x, y, buttonArea->width(), buttonArea->height());
}

void RenderWidget3D::initButtons()
{
    buttonArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    buttonArea->resize(120, 40);

    homeButton->setObjectName("homeButton");
    homeButton->setIconSize(QSize(36, 36));
    homeButton->setIcon(QIcon(":/resources/home.png"));
    homeButton->setToolTip(tr("Home"));

    textureButton->setCheckable(true);
    textureButton->setObjectName("textureButton");
    textureButton->setIconSize(QSize(32, 32));
    textureButton->setToolTip(tr("Texture on"));

    QIcon icon1;
    icon1.addFile(QStringLiteral(":/resources/pointCloud_blue.png"), QSize(), QIcon::Normal, QIcon::Off);
    icon1.addFile(QStringLiteral(":/resources/pointCloud_color.png"), QSize(), QIcon::Active, QIcon::On);
    icon1.addFile(QStringLiteral(":/resources/pointCloud_color.png"), QSize(), QIcon::Selected, QIcon::On);
    textureButton->setIcon(icon1);

    exportButton->setObjectName("export3dButton");
    exportButton->setIconSize(QSize(32, 32));
    exportButton->setToolTip(tr("Save"));
    exportButton->setIcon(QIcon(":/resources/save_mid.png"));

    connect(exportButton, &QPushButton::clicked, this, [=]()
        {
            emit exportPointCloud();
        });
    
    QHBoxLayout* layout = new QHBoxLayout(buttonArea);
    layout->addWidget(textureButton);
    layout->addWidget(homeButton);
    layout->addWidget(exportButton);
    layout->setMargin(0);

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

            emit show3DTextureChanged(toggled);
        });
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

    homeButton->setToolTip(tr("Home"));
    exportButton->setToolTip(tr("Save"));
}

void RenderWidget3D::setTextureEnable(bool enable)
{
    textureButton->setEnabled(enable);
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
        osg::ref_ptr<osg::Cone> cone = new osg::Cone(osg::Vec3(0.0, 0.0, len), radius * 1.5, radius * 3);
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