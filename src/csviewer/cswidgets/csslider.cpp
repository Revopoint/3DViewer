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
    , lineEdit(new CSLineEdit(this))
    , slider(new CustomSlider(Qt::Horizontal,this))
    , minLabel(new QLabel("0", this))
    , maxLabel(new QLabel("0", this))
    , intValidator(new QIntValidator(-10000000, 10000000, this))
    , step(1)
{
    setObjectName("CSSlider");
    lineEdit->setValidator(intValidator);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(0);

    titleLabel = new QLabel(this);
    hLayout->addWidget(titleLabel);

    titleLabel->setObjectName("TitleLabel");

    QVBoxLayout* vLayout2 = new QVBoxLayout();
    hLayout->addWidget(lineEdit);
    hLayout->addItem(vLayout2);
    hLayout->setSpacing(0);
    vLayout2->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    hLayout2->setContentsMargins(0, 0, 0, 0);

    hLayout2->addWidget(minLabel);
    hLayout2->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout2->addWidget(maxLabel);

    minLabel->setProperty("fontSize10", true);
    maxLabel->setProperty("fontSize10", true);
    
    vLayout2->addWidget(slider);
    vLayout2->addItem(hLayout2);
    vLayout2->setSpacing(0);
    vLayout2->setContentsMargins(20, 0, 20, 0);

    minLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    maxLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    bool suc = true;
    suc &= (bool)connect(lineEdit, &QLineEdit::editingFinished,  this, &CSSlider::onLineEditTextChanged);
    //suc &= (bool)connect(slider, &QSlider::sliderReleased, this, &CSSlider::onSliderValueChanged);
    suc &= (bool)connect(slider,   &CustomSlider::sliderClicked, this, &CSSlider::onSliderClicked);
    suc &= (bool)connect(lineEdit, &CSLineEdit::focusOutSignal,  this, &CSSlider::onLinEditFocusOut);

    Q_ASSERT(suc);

    if(QString(title).isEmpty())
    {
        titleLabel->setVisible(false);
    }
}

CSSlider::~CSSlider()
{

}

void CSSlider::setParaRange(const QVariant& min, const QVariant& max, const QVariant& step)
{
    minLabel->setText(QString::number(min.toInt()));
    maxLabel->setText(QString::number(max.toInt()));
    this->step = step.toInt() > 0 ? step.toInt() : 1;

    int sliderMin = min.toInt() / this->step;
    int sliderMax = max.toInt() / this->step;

    slider->setRange(sliderMin, sliderMax);
}

void CSSlider::setValue(const QVariant& value)
{
    int intValue = value.toInt();

    const int min = slider->minimum() * this->step;
    const int max = slider->maximum() * this->step;

    intValue = intValue > max ? max : intValue;
    intValue = intValue < min ? min : intValue;

    int sliderValue = intValue / this->step;
    sliderValue += (intValue % this->step) > 0 ? 1 : 0;

    lineEdit->setText(QString::number(intValue));
    slider->setValue(sliderValue);
}

void CSSlider::onLineEditTextChanged()
{
    int value = lineEdit->text().toInt();
    int sliderValue = value / this->step;

    lineEdit->setText(QString::number(sliderValue * this->step));
    slider->setValue(sliderValue);

    emit valueChanged(getParaId(), sliderValue * this->step);
}

void CSSlider::onSliderClicked(int value)
{
    slider->setValue(value);

    int realValue = value * this->step;
    lineEdit->setText(QString::number(realValue));

    emit valueChanged(getParaId(), realValue);
}

void CSSlider::onSliderValueChanged()
{
    int value = slider->value();
    int realValue = value * this->step;

    lineEdit->setText(QString::number(realValue));
    emit valueChanged(getParaId(), realValue);
}

void CSSlider::retranslate(const char* context)
{
    if (strlen(paraName) > 0)
    {
        titleLabel->setText(QApplication::translate(context, paraName));
    }
}

void CSSlider::onLinEditFocusOut()
{
    auto tex = lineEdit->text();
    int num = 0;
    if (intValidator->validate(tex, num) != QIntValidator::Acceptable)
    {
        const int min = slider->minimum();
        lineEdit->setText(QString::number(min));
        emit lineEdit->editingFinished();
    }
}

void CSSlider::clearValues()
{
    lineEdit->setText("");
    slider->setValue(0);
}