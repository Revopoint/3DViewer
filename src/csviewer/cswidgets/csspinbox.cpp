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

#include "cswidgets/csspinbox.h"
#include <QFormLayout>
#include <QLabel>
#include <QDebug>
#include <qglobal.h>

CSSpinBox::CSSpinBox(int paraId, const char* title, QWidget* parent)
    : CSParaWidget(paraId, title, parent)
    , titleLabel(new QLabel(this))
    , spinBox(new CustomSpinBox(this))
{
    setObjectName("CSSpinBox");

    QFormLayout* layout = new QFormLayout(this);
    titleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    layout->setWidget(0, QFormLayout::LabelRole, titleLabel);
    layout->setWidget(0, QFormLayout::FieldRole, spinBox);

    layout->setSpacing(0);
    layout->setContentsMargins(15, 0, 15, 0);

    bool suc = true;
    suc &= (bool)connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)));

    Q_ASSERT(suc);
}

CSSpinBox::~CSSpinBox()
{

}

void CSSpinBox::setParaRange(const QVariant& min, const QVariant& max, const QVariant& step)
{
    const int intMin = qRound(min.toFloat());
    const int intMax = qRound(max.toFloat());

    spinBox->setMinimum(intMin);
    spinBox->setMaximum(intMax);
}

void CSSpinBox::setValue(const QVariant& value)
{
    const int intValue = value.toInt();
    const int curValue = spinBox->value();

    if (intValue != curValue)
    {
        spinBox->blockSignals(true);
        spinBox->setValue(intValue);
        spinBox->blockSignals(false);
    }
}

void CSSpinBox::onSpinBoxValueChanged(int value)
{
    emit valueChanged(getParaId(), value);
}

void CSSpinBox::retranslate(const char* context)
{
    if (strlen(paraName) > 0)
    {
        titleLabel->setText(QApplication::translate(context, paraName));
    }
}