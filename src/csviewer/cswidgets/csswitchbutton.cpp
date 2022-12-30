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

#include "cswidgets/csswitchbutton.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

CSSwitchButton::CSSwitchButton(int paraId, const char* title , QWidget* parent)
    : CSParaWidget(paraId, title, parent)
    , titleLabel(new QLabel(this))
    , button(new QPushButton(this))
{
    setObjectName("CWSwitchButton");
    titleLabel->setObjectName("TitleLabel");

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0,0,20, 0);
    hLayout->addWidget(titleLabel);
    hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addWidget(button);

    button->setCheckable(true);

    bool suc = true;
    suc &= (bool)connect(button, &QPushButton::clicked, this, &CSSwitchButton::onButtonToggled);

    Q_ASSERT(suc);
}

CSSwitchButton::~CSSwitchButton()
{
}

void CSSwitchButton::setValue(const QVariant& value)
{
    button->setChecked(value.toBool());
}

void CSSwitchButton::onButtonToggled(bool checked)
{
    emit valueChanged(getParaId(), checked);
}

void CSSwitchButton::retranslate(const char* context)
{
    if (strlen(paraName) > 0)
    {
        titleLabel->setText(QApplication::translate(context, paraName));
    }
}

void CSSwitchButton::clearValues()
{
    button->setChecked(false);
}
