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

#include "cswidgets/csrangeedit.h"
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIntValidator>
#include <QPair>
#include <QDebug>

#include "cswidgets/cslineedit.h"
#include "cswidgets/csline.h"

CSRangeEdit::CSRangeEdit(int paraId, const char* title, QWidget* parent)
    : CSParaWidget(paraId, title, parent)
    , rangeMinEdit(new CSLineEdit(this))
    , rangeMaxEdit(new CSLineEdit(this))
    , titleLabel(new QLabel(this))
    , intValidator(new QIntValidator(-10000000, 10000000, this))
    , rangeBottom(0)
    , rangeTop(0)
{
    setObjectName("CSRangeEdit");
    titleLabel->setObjectName("TitleLabel");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    titleLabel->setText(title);
    titleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    layout->addWidget(titleLabel);
    CSHLine* line = new CSHLine(this);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    layout->addItem(hLayout);

    line->setFixedWidth(10);
    line->setProperty("isDark", true);

    hLayout->addWidget(rangeMinEdit);
    hLayout->addWidget(line);
    hLayout->addWidget(rangeMaxEdit);
    hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->setSpacing(10);
    hLayout->setContentsMargins(0, 0, 0, 0);

    rangeMinEdit->setValidator(intValidator);
    rangeMaxEdit->setValidator(intValidator);

    rangeMinEdit->setFixedWidth(86);
    rangeMaxEdit->setFixedWidth(86);

    bool suc = true;

    suc &= (bool)connect(rangeMinEdit, &QLineEdit::editingFinished, this, &CSRangeEdit::onMinEditTextChanged);
    suc &= (bool)connect(rangeMaxEdit, &QLineEdit::editingFinished, this, &CSRangeEdit::onMaxEditTextChanged);

    suc &= (bool)connect(rangeMinEdit, &CSLineEdit::focusOutSignal, this, &CSRangeEdit::onMinFocusOut);
    suc &= (bool)connect(rangeMaxEdit, &CSLineEdit::focusOutSignal, this, &CSRangeEdit::onMaxFocusOut);
    Q_ASSERT(suc);
}

CSRangeEdit::~CSRangeEdit()
{
}

void CSRangeEdit::setParaRange(const QVariant& min, const QVariant& max, const QVariant& step)
{
    rangeBottom = qRound(min.toFloat());
    rangeTop = qRound(max.toFloat());
}

void CSRangeEdit::setValue(const QVariant& value)
{
    QPair<float, float> range = value.value< QPair<float, float>>();
    const int min = rangeMinEdit->text().toInt();
    const int max = rangeMaxEdit->text().toInt();

    const int rangeMin = qRound(range.first);
    const int rangeMax = qRound(range.second);

    if (min != rangeMin)
    {
       rangeMinEdit->setText(QString::number(rangeMin));
    }   
    
    if (max != rangeMax)
    {
        rangeMaxEdit->setText(QString::number(rangeMax));
    }
}

void CSRangeEdit::onMaxEditTextChanged()
{   
    const int min = rangeMinEdit->text().toInt();
    int max = rangeMaxEdit->text().toInt();

    if (max > rangeTop)
    {
        max = rangeTop;
        rangeMaxEdit->setText(QString::number(max));
    }

    if (max < min) 
    {
        max = rangeTop;
        rangeMaxEdit->setText(QString::number(max));
    }

    emit valueChanged(getParaId(), QVariant::fromValue(QPair<float, float>(min, max)));
}

void CSRangeEdit::onMinEditTextChanged()
{
    int min = rangeMinEdit->text().toInt();
    const int max = rangeMaxEdit->text().toInt();

    if (min < rangeBottom)
    {
        min = rangeBottom;
        rangeMinEdit->setText(QString::number(min));
    }

    if (min > max)
    {
        min = rangeBottom;
        rangeMinEdit->setText(QString::number(min));
    }

    emit valueChanged(getParaId(), QVariant::fromValue(QPair<float, float>(min, max)));
}

void CSRangeEdit::retranslate(const char* context)
{
    if (strlen(paraName) > 0)
    {
        titleLabel->setText(QApplication::translate(context, paraName));
    }
}

void CSRangeEdit::onMinFocusOut()
{
    auto tex = rangeMinEdit->text();
    int num = 0;
    if (intValidator->validate(tex, num) != QIntValidator::Acceptable)
    {
        rangeMinEdit->setText(QString::number(rangeBottom));
        emit rangeMinEdit->editingFinished();
    }
}

void CSRangeEdit::onMaxFocusOut()
{
    auto tex = rangeMaxEdit->text();
    int num = 0;
    if (intValidator->validate(tex, num) != QIntValidator::Acceptable)
    {
        rangeMaxEdit->setText(QString::number(rangeTop));
        emit rangeMaxEdit->editingFinished();
    }
}
