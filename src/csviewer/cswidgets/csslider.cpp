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