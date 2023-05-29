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
    , m_rangeMinEdit(new CSLineEdit(this))
    , m_rangeMaxEdit(new CSLineEdit(this))
    , m_titleLabel(new QLabel(this))
    , m_intValidator(new QIntValidator(-10000000, 10000000, this))
    , m_rangeBottom(0)
    , m_rangeTop(0)
{
    setObjectName("CSRangeEdit");
    m_titleLabel->setObjectName("TitleLabel");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleLabel->setText(title);
    m_titleLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    layout->addWidget(m_titleLabel);
    CSHLine* line = new CSHLine(this);

    QHBoxLayout* hLayout = new QHBoxLayout();
    layout->addItem(hLayout);

    line->setFixedWidth(10);
    line->setProperty("isDark", true);

    hLayout->addWidget(m_rangeMinEdit);
    hLayout->addWidget(line);
    hLayout->addWidget(m_rangeMaxEdit);
    hLayout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->setSpacing(10);
    hLayout->setContentsMargins(0, 0, 0, 0);

    m_rangeMinEdit->setValidator(m_intValidator);
    m_rangeMaxEdit->setValidator(m_intValidator);

    m_rangeMinEdit->setFixedWidth(86);
    m_rangeMaxEdit->setFixedWidth(86);

    bool suc = true;

    suc &= (bool)connect(m_rangeMinEdit, &QLineEdit::editingFinished, this, &CSRangeEdit::onMinEditTextChanged);
    suc &= (bool)connect(m_rangeMaxEdit, &QLineEdit::editingFinished, this, &CSRangeEdit::onMaxEditTextChanged);

    suc &= (bool)connect(m_rangeMinEdit, &CSLineEdit::focusOutSignal, this, &CSRangeEdit::onMinFocusOut);
    suc &= (bool)connect(m_rangeMaxEdit, &CSLineEdit::focusOutSignal, this, &CSRangeEdit::onMaxFocusOut);
    Q_ASSERT(suc);
}

CSRangeEdit::~CSRangeEdit()
{
}

void CSRangeEdit::setParaRange(const QVariant& min, const QVariant& max, const QVariant& step)
{
    m_rangeBottom = qRound(min.toFloat());
    m_rangeTop = qRound(max.toFloat());
}

void CSRangeEdit::setValue(const QVariant& value)
{
    QPair<float, float> range = value.value< QPair<float, float>>();
    const int min = m_rangeMinEdit->text().toInt();
    const int max = m_rangeMaxEdit->text().toInt();

    const int rangeMin = qRound(range.first);
    const int rangeMax = qRound(range.second);

    if (min != rangeMin)
    {
       m_rangeMinEdit->setText(QString::number(rangeMin));
    }   
    
    if (max != rangeMax)
    {
        m_rangeMaxEdit->setText(QString::number(rangeMax));
    }
}

void CSRangeEdit::onMaxEditTextChanged()
{   
    const int min = m_rangeMinEdit->text().toInt();
    int max = m_rangeMaxEdit->text().toInt();

    if (max > m_rangeTop)
    {
        max = m_rangeTop;
        m_rangeMaxEdit->setText(QString::number(max));
    }

    if (max < min) 
    {
        max = m_rangeTop;
        m_rangeMaxEdit->setText(QString::number(max));
    }

    emit valueChanged(getParaId(), QVariant::fromValue(QPair<float, float>(min, max)));
}

void CSRangeEdit::onMinEditTextChanged()
{
    int min = m_rangeMinEdit->text().toInt();
    const int max = m_rangeMaxEdit->text().toInt();

    if (min < m_rangeBottom)
    {
        min = m_rangeBottom;
        m_rangeMinEdit->setText(QString::number(min));
    }

    if (min > max)
    {
        min = m_rangeBottom;
        m_rangeMinEdit->setText(QString::number(min));
    }

    emit valueChanged(getParaId(), QVariant::fromValue(QPair<float, float>(min, max)));
}

void CSRangeEdit::retranslate(const char* context)
{
    if (strlen(m_paraName) > 0)
    {
        m_titleLabel->setText(QApplication::translate(context, m_paraName));
    }
}

void CSRangeEdit::onMinFocusOut()
{
    auto tex = m_rangeMinEdit->text();
    int num = 0;
    if (m_intValidator->validate(tex, num) != QIntValidator::Acceptable)
    {
        m_rangeMinEdit->setText(QString::number(m_rangeBottom));
        emit m_rangeMinEdit->editingFinished();
    }
}

void CSRangeEdit::onMaxFocusOut()
{
    auto tex = m_rangeMaxEdit->text();
    int num = 0;
    if (m_intValidator->validate(tex, num) != QIntValidator::Acceptable)
    {
        m_rangeMaxEdit->setText(QString::number(m_rangeTop));
        emit m_rangeMaxEdit->editingFinished();
    }
}

void CSRangeEdit::clearValues()
{
    m_rangeMinEdit->setText("");
    m_rangeMaxEdit->setText("");
}