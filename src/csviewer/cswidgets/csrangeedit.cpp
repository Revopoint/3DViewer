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

    QHBoxLayout* hLayout = new QHBoxLayout();
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

void CSRangeEdit::clearValues()
{
    rangeMinEdit->setText("");
    rangeMaxEdit->setText("");
}