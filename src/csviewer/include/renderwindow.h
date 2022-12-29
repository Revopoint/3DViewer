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

#ifndef _CS_RENDER_WINDOWS_H
#define _CS_RENDER_WINDOWS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QImage>
#include <cstypes.h>
#include <hpp/Processing.hpp>

class RenderWidget;
class RenderWindow : public QWidget
{
    Q_OBJECT
public:
    RenderWindow(QWidget* parent = nullptr);
    ~RenderWindow();

    void onWindowLayoutUpdated();
    void hideRenderFps();
    void setShowTextureEnable(bool enable);
    void onTranslate();
signals:
    void roiRectFUpdated(QRectF rect);
    void renderExit(int renderId);
    void fullScreenUpdated(int renderID, bool value);

public slots:
    void onRenderWindowsUpdated(QVector<int> windows);
    void onWindowLayoutModeUpdated(int mode);
    void onOutput2DUpdated(OutputData2D outputData);
    void onOutput3DUpdated(cs::Pointcloud pointCloud, const QImage& image);
    void onRoiEditStateChanged(bool edit,  QRectF rect);

private slots:
    void onFullScreenUpdated(int renderId, bool value);
    void onShow3DTextureChanged(bool texture);
private:
    void initRenderWidgetsTitle();
    void initRenderWidgetsTab();
    void initConnections();
    void initRenderWidgets();
    void initGridLayout();

    void setRender3DTextureVisible(bool visible);
private:
    WINDOWLAYOUT_MODE layoutMode = LAYOUT_TILE;// LAYOUT_TITLE;
    QVector<int> displayWindows;

    QMap<int, RenderWidget*> renderWidgets;
    // the parent widget of render
    QWidget* renderMainWidget = nullptr;
    QLayout* rootLayout;
    bool show3DTexture = false;
    bool showTextureEnable = true;
};

#endif //_CS_RENDER_WINDOWS_H
