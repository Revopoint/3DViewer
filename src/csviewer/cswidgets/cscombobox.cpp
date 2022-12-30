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

#include "cswidgets/cscombobox.h"

#include <QListView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

CSComboBox::CSComboBox(int paraId, const char* title, QWidget* parent, const char* tips)
    : CSParaWidget(paraId, title, parent)
    , comboBox(new CustomComboBox(this))
    , titleLabel(new QLabel(this))
    , lastIndex(-1)
    , tips(tips)
{
    //setAttribute(Qt::WA_StyledBackground);
    setObjectName("CSComboBox");
    titleLabel->setObjectName("TitleLabel");

    QHBoxLayout* layout = new QHBoxLayout(this);

    titleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(titleLabel);
    layout->addWidget(comboBox);

    // add tips button
    if (strlen(tips) > 0)
    {
        tipsButton = new QPushButton(this);
        tipsButton->setObjectName("tipsButton");
        layout->addWidget(tipsButton);
    }

    layout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QListView* listView = new QListView(this);
    comboBox->setFocusPolicy(Qt::NoFocus);
    listView->setFocusPolicy(Qt::NoFocus);

    comboBox->setView(listView);

    bool suc = true;
    suc &= (bool)connect(comboBox, SIGNAL(activated(int)), this, SLOT(onComboBoxIndexChanged(int)));

    Q_ASSERT(suc);
}

CSComboBox::~CSComboBox()
{

}

void CSComboBox::setItems(const QList<QPair<QString, QVariant>>& items)
{
    comboBox->clear();
    for (const auto& item : items)
    {
        comboBox->addItem(item.first, item.second);
    }
}

void CSComboBox::setValue(const QVariant& value)
{
    const int curIdx = comboBox->currentIndex();
    lastIndex = curIdx;

    for (int i = 0; i < comboBox->count(); i++)
    {
        auto itemData = comboBox->itemData(i);
        if (itemData == value)
        {
            if (i != curIdx)
            {
                comboBox->setCurrentIndex(i);
                lastIndex = comboBox->currentIndex();
            }
            break;
        }
    }
}

void CSComboBox::getValue(QVariant& value)
{
    value = comboBox->itemData(comboBox->currentIndex());
}

void CSComboBox::onComboBoxIndexChanged(int index)
{
    if (lastIndex != index)
    {
        lastIndex = index;
        emit valueChanged(getParaId(), comboBox->itemData(index));
    }
}

void CSComboBox::retranslate(const char* context)
{
    if (strlen(paraName) > 0)
    {       
        titleLabel->setText(QApplication::translate(context, paraName));
    }

    if(tipsButton)
    {
        tipsButton->setToolTip(QApplication::translate(context, tips));
    }
}

void CSComboBox::clearValues()
{
    comboBox->clear();
}
