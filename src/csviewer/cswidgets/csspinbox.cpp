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
    titleLabel->setObjectName("TitleLabel");

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

void CSSpinBox::clearValues()
{
    spinBox->clear();
}