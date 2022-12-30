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

#include "cswidgets/csroieditwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>

CSRoiEditWidget::CSRoiEditWidget(int paraId, const char* name, QWidget * parent)
    : CSParaWidget(paraId, name, parent)
    , fullScreenButton(new QPushButton(tr("Full Screen"), this))
    , roiEditButton(new QPushButton(tr("Edit ROI"), this))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 20, 0);
    layout->setSpacing(10);

    layout->addWidget(new QLabel(name, this));
    layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout->addWidget(fullScreenButton);
    layout->addWidget(roiEditButton);

    fullScreenButton->setProperty("isCSStyle", true);
    roiEditButton->setProperty("isCSStyle", true);

    bool suc = true;
    suc = (bool)connect(fullScreenButton, &QPushButton::clicked, this, &CSRoiEditWidget::clickedFullScreen);
    suc = (bool)connect(roiEditButton,    &QPushButton::clicked, this, &CSRoiEditWidget::clickedEditRoi);
    Q_ASSERT(suc);
}

CSRoiEditWidget::~CSRoiEditWidget()
{

}

void CSRoiEditWidget::retranslate(const char* context)
{
    fullScreenButton->setText(tr("Full Screen"));
    roiEditButton->setText(tr("Edit ROI"));
}