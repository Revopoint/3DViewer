#include "renderwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>

#include <icscamera.h>

#include "renderwidget2d.h"
#include "renderwidget3d.h"
#include "csapplication.h"

RenderWindow::RenderWindow(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    rootLayout = new QVBoxLayout(this);

    initConnections();
}

RenderWindow::~RenderWindow()
{

}

void RenderWindow::initConnections()
{
    auto app = cs::CSApplication::getInstance();

    bool suc = true;
    suc &= (bool)connect(app, &cs::CSApplication::output3DUpdated, this, &RenderWindow::onOutput3DUpdated);
    suc &= (bool)connect(app, &cs::CSApplication::output2DUpdated, this, &RenderWindow::onOutput2DUpdated);

    Q_ASSERT(suc);
}

void RenderWindow::onWindowLayoutUpdated()
{
    if (renderWidget3D)
    {
        delete renderWidget3D;
        renderWidget3D = nullptr;
    }

    for (auto key : render2dWidgets.keys())
    {
        if (render2dWidgets[key])
        {
            delete render2dWidgets[key];
            render2dWidgets[key] = nullptr;
        }
    }

    render2dWidgets.clear();

    if (renderMainWidget)
    {
        rootLayout->removeWidget(renderMainWidget);
        delete renderMainWidget;
        renderMainWidget = nullptr;
    }

    initRenderWidgets();

    if (layoutMode == LAYOUT_TITLE)
    {
        renderMainWidget = new QWidget(this);
        initRenderWidgetsTitle();
    }
    else
    {
        // tabs
        renderMainWidget = new QTabWidget(this);
        initRenderWidgetsTab();
    }

    auto camera = cs::CSApplication::getInstance()->getCamera();
    // depth resolution
    QVariant depthRes;
    camera->getCameraPara(cs::parameter::PARA_DEPTH_RESOLUTION, depthRes);
    auto depthRatio = depthRes.toSize().width() * 1.0f / depthRes.toSize().height();

    // set (width/height) ratio
    if (render2dWidgets[CAMERA_DATA_L])
    {
        render2dWidgets[CAMERA_DATA_L]->setWHRatio(depthRatio);
        render2dWidgets[CAMERA_DATA_R]->setWHRatio(depthRatio);
    }

    if (render2dWidgets[CAMERA_DATA_DEPTH])
    {
        render2dWidgets[CAMERA_DATA_DEPTH]->setWHRatio(depthRatio);
    }

    if (render2dWidgets[CAMERA_DATA_RGB])
    {
        // set (width/height) ratio
        QVariant rgbRes;
        camera->getCameraPara(cs::parameter::PARA_RGB_RESOLUTION, rgbRes);
        auto res = rgbRes.toSize();
        render2dWidgets[CAMERA_DATA_RGB]->setWHRatio(res.width() * 1.0 / res.height());
    }

    rootLayout->addWidget(renderMainWidget);
}

void RenderWindow::onRenderWindowsUpdated(QVector<int> windows)
{
    qInfo() << "display windows changed";
    displayWindows = windows;

    onWindowLayoutUpdated();
}

void RenderWindow::onWindowLayoutModeUpdated(int mode)
{
    if(layoutMode != mode)
    {
        qInfo() << "render layout changed";
        layoutMode = (WINDOWLAYOUT_MODE)mode;
        onWindowLayoutUpdated();
    }
}

void RenderWindow::initRenderWidgetsTitle()
{
    if (!renderMainWidget)
    {
        qWarning() << "root render widget is nullptr";
        return;
    }

    QGridLayout* layout = new QGridLayout(renderMainWidget);

    int idx = 0;
    int row = 0;
    for(auto widget : render2dWidgets.values())
    {
        if (widget)
        {
            int col = (idx & 1) > 0 ? 1 : 0;
            layout->addWidget(widget, row, col);
            idx++;

            if (idx % 2 == 0)
            {
                row++;
            }
        }
    }

    if (renderWidget3D)
    {
        int col = (idx & 1) > 0 ? 1 : 0;
        layout->addWidget(renderWidget3D, row, col);
        idx++;
    }

    // set stretch
    const int rowCount = idx / 2 + ((idx & 1) > 0 ? 1 : 0);
    const int colCount = (idx > 1) ? 2 : 1;

    for (int i = 0; i < rowCount; i++)
    {
        layout->setRowStretch(i, 1);
    }

    for (int i = 0; i < colCount; i++)
    {
        layout->setColumnStretch(i, 1);
    }
}

void RenderWindow::initRenderWidgetsTab()
{
    auto tabWidget = qobject_cast<QTabWidget*>(renderMainWidget);
    if (!tabWidget)
    {
        qWarning() << "Error, tabWidget is nullptr";
        return;
    }

    tabWidget->clear();
    for (auto widget : render2dWidgets.values())
    {
        if (widget) 
        {
            CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)widget->getRenderId();

            switch (dataType)
            {
            case CAMERA_DATA_L:
            {
                tabWidget->addTab(widget, "IR(L)");
                break;
            }
            case CAMERA_DATA_R:
            {
                tabWidget->addTab(widget, "IR(R)");
                break;
            }
            case CAMERA_DATA_DEPTH:
            {
                tabWidget->addTab(widget, "Depth");
                bool suc = true;
                suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(widget), &DepthRenderWidget2D::roiRectFUpdated, this, &RenderWindow::roiRectFUpdated);
                suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(widget), &DepthRenderWidget2D::showCoordChanged, cs::CSApplication::getInstance(), &cs::CSApplication::onShowCoordChanged);
                Q_ASSERT(suc);

                break;
            }
            case CAMERA_DATA_RGB:
            {
                tabWidget->addTab(widget, "RGB");
                break;
            }
            default:
                break;
            }
        }
    }

    if (renderWidget3D)
    {
        tabWidget->addTab(renderWidget3D, "Point Cloud");
        connect(renderWidget3D, &RenderWidget3D::show3DTextureChanged, cs::CSApplication::getInstance(), &cs::CSApplication::onShow3DTextureChanged);
    }
}

void RenderWindow::onOutput2DUpdated(OutputData2D outputData)
{
    RenderWidget2D* widget = render2dWidgets[outputData.info.cameraDataType];
    if (widget)
    {
        widget->onRenderDataUpdated(outputData);
    }
}

void RenderWindow::onOutput3DUpdated(cs::Pointcloud pointCloud, const QImage& image)
{
    if (renderWidget3D)
    {
        renderWidget3D->onRenderDataUpdated(pointCloud, image);
    }
}

void RenderWindow::onRoiEditStateChanged(bool edit)
{
    DepthRenderWidget2D* widget = qobject_cast<DepthRenderWidget2D*>(render2dWidgets[CAMERA_DATA_DEPTH]);;
    if (widget)
    {
        widget->onRoiEditStateChanged(edit);
    }
}

void RenderWindow::initRenderWidgets()
{
    for (auto type : displayWindows)
    {
        CS_CAMERA_DATA_TYPE dataType = (CS_CAMERA_DATA_TYPE)type;

        switch (dataType)
        {
        case CAMERA_DATA_L:
        {
            auto renderWidget = new RenderWidget2D((int)dataType);
            render2dWidgets[dataType] = renderWidget;
            break;
        }
        case CAMERA_DATA_R:
        {
            auto renderWidget = new RenderWidget2D((int)dataType);
            render2dWidgets[dataType] = renderWidget;
            break;
        }
        case CAMERA_DATA_DEPTH:
        {
            auto renderWidget = new DepthRenderWidget2D((int)dataType);
            render2dWidgets[dataType] = renderWidget;

            bool suc = true;
            suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(renderWidget), &DepthRenderWidget2D::roiRectFUpdated, this, &RenderWindow::roiRectFUpdated);
            suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(renderWidget), &DepthRenderWidget2D::showCoordChanged, cs::CSApplication::getInstance(), &cs::CSApplication::onShowCoordChanged);
            Q_ASSERT(suc);

            break;
        }
        case CAMERA_DATA_RGB:
        {
            auto renderWidget = new RenderWidget2D((int)dataType);
            render2dWidgets[dataType] = renderWidget;
            break;
        }
        case CAMERA_DTA_POINT_CLOUD:
        {
            auto renderWidget = new RenderWidget3D(this);
            renderWidget3D = renderWidget;
            break;
        }
        default:
            break;
        }
    }
}