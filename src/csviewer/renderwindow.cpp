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

#include "renderwindow.h"
#include <QPushButton>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
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
    bool suc = true;
    suc &= (bool)connect(this, &RenderWindow::fullScreenUpdated,   this, &RenderWindow::onFullScreenUpdated, Qt::QueuedConnection);
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
    onShow3DTextureChanged(false);

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

    setRender3DTextureVisible(showTextureEnable);
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
            case CAMERA_DATA_POINT_CLOUD:
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
        if (!widget->isHidden())
        {
            widget->onRenderDataUpdated(outputData);
        }
    }
}

void RenderWindow::onOutput3DUpdated(cs::Pointcloud pointCloud, const QImage& image)
{
    RenderWidget3D* widget = qobject_cast<RenderWidget3D*>(renderWidgets[CAMERA_DATA_POINT_CLOUD]);
    if (widget)
    {
        if (!widget->isHidden())
        {
            widget->onRenderDataUpdated(pointCloud, image);
        }
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
        case CAMERA_DATA_POINT_CLOUD:
        {
            auto renderWidget = new RenderWidget3D((int)dataType, this);
            renderWidgets[dataType] = renderWidget;

            connect(renderWidget, &RenderWidget3D::show3DTextureChanged, this, &RenderWindow::onShow3DTextureChanged, Qt::QueuedConnection);
            emit renderWidget->show3DTextureChanged(false);
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

void RenderWindow::onShow3DTextureChanged(bool texture)
{
    show3DTexture = texture;
    cs::CSApplication::getInstance()->onShow3DTextureChanged(show3DTexture);
}

void RenderWindow::hideRenderFps()
{
    for (auto widget : renderWidgets.values())
    {
        auto render2d = qobject_cast<RenderWidget2D*>(widget);
        if (render2d)
        {
            render2d->hideRenderFps();
        }
    }
}

void RenderWindow::setRender3DTextureVisible(bool visible)
{
    RenderWidget3D* widget = qobject_cast<RenderWidget3D*>(renderWidgets[CAMERA_DATA_POINT_CLOUD]);
    if (widget)
    {
        widget->setTextureEnable(visible);
    }
}

void RenderWindow::setShowTextureEnable(bool enable)
{
    showTextureEnable = enable;
}

void RenderWindow::onTranslate()
{
    for (auto widget : renderWidgets.values())
    {
        if (widget)
        {
            widget->onTranslate();
        }
    }
}