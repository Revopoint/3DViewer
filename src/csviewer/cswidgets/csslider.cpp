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

#include "cswidgets/csslider.h"
#include <QLineEdit>
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QIntValidator>
#include <QDebug>
#include <QStyle>
#include <QDateTime>

#include "cswidgets/cslineedit.h"

void CustomSlider::wheelEvent(QWheelEvent*)
{

}

void CustomSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QSlider::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        const int value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().x(), width());
        emit sliderClicked(value);
    }
}

CSSlider::CSSlider(QWidget* parent)
    : CSSlider(-1, "", parent)
{


}

CSSlider::CSSlider(int paraId, const char* title, QWidget* parent)
    : CSParaWidget(paraId, title, parent)
    , m_lineEdit(new CSLineEdit(this))
    , m_slider(new CustomSlider(Qt::Horizontal,this))
    , m_minLabel(new QLabel("0", this))
    , m_maxLabel(new QLabel("0", this))
    , m_intValidator(new QIntValidator(-10000000, 10000000, this))
    , m_step(1)
{
    setObjectName("CSSlider");
    m_lineEdit->setValidator(m_intValidator);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    m_titleLabel = new QLabel(this);
    hLayout->addWidget(m_titleLabel);

    m_titleLabel->setObjectName("TitleLabel");

    QVBoxLayout* vLayout2 = new QVBoxLayout();
    hLayout->addWidget(m_lineEdit);
    hLayout->addItem(vLayout2);
    hLayout->setSpacing(0);
    vLayout2->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    hLayout2->setContentsMargins(0, 0, 0, 0);

    hLayout2->addWidget(m_minLabel);
    hLayout2->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout2->addWidget(m_maxLabel);

    m_minLabel->setProperty("fontSize10", true);
    m_maxLabel->setProperty("fontSize10", true);
    
    vLayout2->addWidget(m_slider);
    vLayout2->addItem(hLayout2);
    vLayout2->setSpacing(0);
    vLayout2->setContentsMargins(20, 0, 20, 0);

    m_minLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_maxLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    bool suc = true;
    suc &= (bool)connect(m_lineEdit, &QLineEdit::editingFinished,  this, &CSSlider::onLineEditTextChanged);
    //suc &= (bool)connect(slider, &QSlider::sliderReleased, this, &CSSlider::onSliderValueChanged);
    suc &= (bool)connect(m_slider,   &CustomSlider::sliderClicked, this, &CSSlider::onSliderClicked);
    suc &= (bool)connect(m_lineEdit, &CSLineEdit::focusOutSignal,  this, &CSSlider::onLinEditFocusOut);

    Q_ASSERT(suc);

    if(QString(title).isEmpty())
    {
        m_titleLabel->setVisible(false);
    }
}

CSSlider::~CSSlider()
{

}

void CSSlider::setParaRange(const QVariant& min, const QVariant& max, const QVariant& step)
{
    m_minLabel->setText(QString::number(min.toInt()));
    m_maxLabel->setText(QString::number(max.toInt()));
    this->m_step = step.toInt() > 0 ? step.toInt() : 1;

    int sliderMin = min.toInt() / this->m_step;
    int sliderMax = max.toInt() / this->m_step;

    m_slider->setRange(sliderMin, sliderMax);
}

void CSSlider::setValue(const QVariant& value)
{
    int intValue = value.toInt();

    const int min = m_slider->minimum() * this->m_step;
    const int max = m_slider->maximum() * this->m_step;

    intValue = intValue > max ? max : intValue;
    intValue = intValue < min ? min : intValue;

    int sliderValue = intValue / this->m_step;
    sliderValue += (intValue % this->m_step) > 0 ? 1 : 0;

    m_lineEdit->setText(QString::number(intValue));
    m_slider->setValue(sliderValue);
}

void CSSlider::onLineEditTextChanged()
{
    int value = m_lineEdit->text().toInt();
    int sliderValue = value / this->m_step;

    m_lineEdit->setText(QString::number(sliderValue * this->m_step));
    m_slider->setValue(sliderValue);

    emit valueChanged(getParaId(), sliderValue * this->m_step);
}

void CSSlider::onSliderClicked(int value)
{
    m_slider->setValue(value);

    int realValue = value * this->m_step;
    m_lineEdit->setText(QString::number(realValue));

    emit valueChanged(getParaId(), realValue);
}

void CSSlider::onSliderValueChanged()
{
    int value = m_slider->value();
    int realValue = value * this->m_step;

    m_lineEdit->setText(QString::number(realValue));
    emit valueChanged(getParaId(), realValue);
}

void CSSlider::retranslate(const char* context)
{
    if (strlen(m_paraName) > 0)
    {
        m_titleLabel->setText(QApplication::translate(context, m_paraName));
    }
}

void CSSlider::onLinEditFocusOut()
{
    auto tex = m_lineEdit->text();
    int num = 0;
    if (m_intValidator->validate(tex, num) != QIntValidator::Acceptable)
    {
        const int min = m_slider->minimum();
        m_lineEdit->setText(QString::number(min));
        emit m_lineEdit->editingFinished();
    }
}

void CSSlider::clearValues()
{
    m_lineEdit->setText("");
    m_slider->setValue(0);
}