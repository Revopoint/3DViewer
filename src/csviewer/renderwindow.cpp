#include "renderwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <icscamera.h>
#include "renderwidget.h"
#include "csapplication.h"

RenderWindow::RenderWindow(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("RenderWindow");
    setAttribute(Qt::WA_StyledBackground, true);
    rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
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
    suc &= (bool)connect(this, &RenderWindow::fullScreenUpdated,   this, &RenderWindow::onFullScreenUpdated, Qt::QueuedConnection);

    void onFullScreenUpdated(int renderID, bool value);

    Q_ASSERT(suc);
}

void RenderWindow::onWindowLayoutUpdated()
{
    for (auto key : renderWidgets.keys())
    {
        if (renderWidgets[key])
        {
            delete renderWidgets[key];
            renderWidgets[key] = nullptr;
        }
    }

    renderWidgets.clear();

    if (renderMainWidget)
    {
        rootLayout->removeWidget(renderMainWidget);
        delete renderMainWidget;
        renderMainWidget = nullptr;
    }

    initRenderWidgets();

    if (layoutMode == LAYOUT_TILE)
    {
        renderMainWidget = new QWidget(this);
        initRenderWidgetsTitle();
    }
    else
    {
        // tabs
        QTabWidget* tabWidget = new QTabWidget(this);
        //tabWidget->setTabBarAutoHide(true);
        tabWidget->setAttribute(Qt::WA_StyledBackground, true);
        renderMainWidget = tabWidget;
        initRenderWidgetsTab();
    }

    auto camera = cs::CSApplication::getInstance()->getCamera();
    // depth resolution
    QVariant depthRes;
    camera->getCameraPara(cs::parameter::PARA_DEPTH_RESOLUTION, depthRes);
    auto depthRatio = depthRes.toSize().width() * 1.0f / depthRes.toSize().height();

    // set (width/height) ratio
    if (renderWidgets[CAMERA_DATA_L])
    {
        RenderWidget2D* render = qobject_cast<RenderWidget2D*>(renderWidgets[CAMERA_DATA_L]);
        render->setWHRatio(depthRatio);
        render->setWHRatio(depthRatio);
    }

    if (renderWidgets[CAMERA_DATA_DEPTH])
    {
        RenderWidget2D* render = qobject_cast<RenderWidget2D*>(renderWidgets[CAMERA_DATA_DEPTH]);
        render->setWHRatio(depthRatio);
    }

    if (renderWidgets[CAMERA_DATA_RGB])
    {
        // set (width/height) ratio
        QVariant rgbRes;
        camera->getCameraPara(cs::parameter::PARA_RGB_RESOLUTION, rgbRes);
        auto res = rgbRes.toSize();

        RenderWidget2D* render = qobject_cast<RenderWidget2D*>(renderWidgets[CAMERA_DATA_RGB]);
        render->setWHRatio(res.width() * 1.0 / res.height());
    }

    if (layoutMode == LAYOUT_TILE)
    {
        for (auto widget : renderWidgets.values())
        {

            if (widget)
            {
                bool suc = true;
                suc &= (bool)connect(widget, &RenderWidget::renderExit, this, &RenderWindow::renderExit);
                suc &= (bool)connect(widget, &RenderWidget::fullScreenUpdated, this, &RenderWindow::fullScreenUpdated);
                Q_ASSERT(suc);

                widget->setShowFullScreen(true);
                RenderWidget2D* renderWidget = qobject_cast<RenderWidget2D*>(widget);
                if (renderWidget)
                {
                    renderWidget->setEnableScale(false);
                }
            }
        }
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
    
    initGridLayout();
}

void RenderWindow::initGridLayout()
{
    auto layout = qobject_cast<QGridLayout*>(renderMainWidget->layout());
    if (!layout)
    {
        qWarning() << "grid layout is null";
        return;
    }

    for (int i = 0; i < layout->rowCount(); ++i)
    {
        layout->setRowStretch(i, 0);
    }
    for (int i = 0; i < layout->columnCount(); ++i)
    {
        layout->setColumnStretch(i, 0);
    }

    int idx = 0;
    int row = 0;
    for (auto widget : renderWidgets.values())
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
    for (auto widget : renderWidgets.values())
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
                break;
            }
            case CAMERA_DATA_RGB:
            {
                tabWidget->addTab(widget, "RGB");
                break;
            } 
            case CAMERA_DTA_POINT_CLOUD:
            {
                tabWidget->addTab(widget, "Point Cloud");
                break;
            }
            default:
                break;
            }
        }
    }
}

void RenderWindow::onOutput2DUpdated(OutputData2D outputData)
{
    RenderWidget2D* widget = qobject_cast<RenderWidget2D*>(renderWidgets[outputData.info.cameraDataType]);
    if (widget)
    {
        widget->onRenderDataUpdated(outputData);
    }
}

void RenderWindow::onOutput3DUpdated(cs::Pointcloud pointCloud, const QImage& image)
{
    RenderWidget3D* widget = qobject_cast<RenderWidget3D*>(renderWidgets[CAMERA_DTA_POINT_CLOUD]);
    if (widget)
    {
        widget->onRenderDataUpdated(pointCloud, image);
    }
}

void RenderWindow::onRoiEditStateChanged(bool edit,  QRectF rect)
{
    DepthRenderWidget2D* widget = qobject_cast<DepthRenderWidget2D*>(renderWidgets[CAMERA_DATA_DEPTH]);;
    if (widget)
    {
        widget->onRoiEditStateChanged(edit, rect);
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
        case CAMERA_DATA_R:
        case CAMERA_DATA_RGB:
        {
            auto renderWidget = new RenderWidget2D((int)dataType);
            renderWidgets[dataType] = renderWidget;
            break;
        }
        case CAMERA_DATA_DEPTH:
        {
            auto renderWidget = new DepthRenderWidget2D((int)dataType);
            renderWidgets[dataType] = renderWidget;

            bool suc = true;
            suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(renderWidget), &DepthRenderWidget2D::roiRectFUpdated, this, &RenderWindow::roiRectFUpdated);
            suc &= (bool)connect(qobject_cast<DepthRenderWidget2D*>(renderWidget), &DepthRenderWidget2D::showCoordChanged, cs::CSApplication::getInstance(), &cs::CSApplication::onShowCoordChanged);
            Q_ASSERT(suc);

            break;
        }
        case CAMERA_DTA_POINT_CLOUD:
        {
            auto renderWidget = new RenderWidget3D((int)dataType, this);
            renderWidgets[dataType] = renderWidget;

            connect(qobject_cast<RenderWidget3D*>(renderWidget), &RenderWidget3D::show3DTextureChanged, cs::CSApplication::getInstance(), &cs::CSApplication::onShow3DTextureChanged);
            break;
        }
        default:
            break;
        }
    }
}

void RenderWindow::onFullScreenUpdated(int renderId, bool value)
{
    if (!renderMainWidget)
    {
        qWarning() << "the render root widget is null.";
        return;
    }

    auto layout = qobject_cast<QGridLayout*>(renderMainWidget->layout());
    if (layout)
    {
        RenderWidget* renderWidget = nullptr;
        for (auto key : renderWidgets.keys())
        {
            if (renderWidgets[key])
            {
                layout->removeWidget(renderWidgets[key]);

                if (renderId == key)
                {
                    renderWidget = renderWidgets[key];
                }
                else 
                {
                    renderWidgets[key]->setVisible(false);
                }
            }
        }

        if (value)
        {
            for (int i = 0; i < layout->rowCount(); ++i)
            {
                layout->setRowStretch(i, 0);
            }
            for (int i = 0; i < layout->columnCount(); ++i)
            {
                layout->setColumnStretch(i, 0);
            }

            if (renderWidget)
            {
                layout->addWidget(renderWidget, 0, 0);
            }
        }
        else 
        {
            for (auto key : renderWidgets.keys())
            {
                if (renderWidgets[key])
                {
                    layout->removeWidget(renderWidgets[key]);
                    renderWidgets[key]->setVisible(true);
                }
            }

            initGridLayout();
        }
    }
}
