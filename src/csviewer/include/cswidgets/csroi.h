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

#ifndef _CS_CSROI_H
#define _CS_CSROI_H

#include <QWidget>
#include <QVariant>
#include <QPushButton>
#include <QRectF>
#include <QPoint>

class CSROIWidget : public QWidget
{
    Q_OBJECT
public:
    CSROIWidget(QWidget * parent = nullptr);
    ~CSROIWidget();

    void setOffset(QMargins offset);
    int getButtonAreaHeight() const { return m_buttonAreaHeight; }
    void updateRoiRectF(QRectF rect);
    void paintEvent(QPaintEvent* event) override;

    void mouseReleaseEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
private:
    void drawRoi();
    void updateButtonsPos();
    void translateRoi(float x, float y);
signals:
    void roiValueUpdated(QRectF rect);
    void roiVisialeChanged(bool visible);
private:
    QRectF m_roiRect;
    QRectF m_roiRectLast;

    QMargins m_roiOffset;
    int m_buttonAreaHeight = 40;

    QPushButton* m_cancelButton;
    QPushButton* m_okButton;
    QWidget* m_buttonArea;

    int m_pressPositon = -1;
    QPoint m_lastPosition;
};

#endif //_CS_CSROI_H
