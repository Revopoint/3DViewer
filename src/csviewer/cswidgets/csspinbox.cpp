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
    , m_titleLabel(new QLabel(this))
    , m_spinBox(new CustomSpinBox(this))
{
    setObjectName("CSSpinBox");
    m_titleLabel->setObjectName("TitleLabel");

    QFormLayout* layout = new QFormLayout(this);
    m_titleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    layout->setWidget(0, QFormLayout::LabelRole, m_titleLabel);
    layout->setWidget(0, QFormLayout::FieldRole, m_spinBox);

    layout->setSpacing(0);
    layout->setContentsMargins(15, 0, 15, 0);

    bool suc = true;
    suc &= (bool)connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)));

    Q_ASSERT(suc);
}

CSSpinBox::~CSSpinBox()
{

}

void CSSpinBox::setParaRange(const QVariant& min, const QVariant& max, const QVariant& step)
{
    const int intMin = qRound(min.toFloat());
    const int intMax = qRound(max.toFloat());

    m_spinBox->setMinimum(intMin);
    m_spinBox->setMaximum(intMax);
}

void CSSpinBox::setValue(const QVariant& value)
{
    const int intValue = value.toInt();
    const int curValue = m_spinBox->value();

    if (intValue != curValue)
    {
        m_spinBox->blockSignals(true);
        m_spinBox->setValue(intValue);
        m_spinBox->blockSignals(false);
    }
}

void CSSpinBox::onSpinBoxValueChanged(int value)
{
    emit valueChanged(getParaId(), value);
}

void CSSpinBox::retranslate(const char* context)
{
    if (strlen(m_paraName) > 0)
    {
        m_titleLabel->setText(QApplication::translate(context, m_paraName));
    }
}

void CSSpinBox::clearValues()
{
    m_spinBox->clear();
}