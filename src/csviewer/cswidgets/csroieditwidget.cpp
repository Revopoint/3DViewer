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