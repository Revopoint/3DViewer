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
    , m_comboBox(new CustomComboBox(this))
    , m_titleLabel(new QLabel(this))
    , m_lastIndex(-1)
    , m_tips(tips)
{
    //setAttribute(Qt::WA_StyledBackground);
    setObjectName("CSComboBox");
    m_titleLabel->setObjectName("TitleLabel");

    QHBoxLayout* layout = new QHBoxLayout(this);

    m_titleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(m_titleLabel);
    layout->addWidget(m_comboBox);

    // add tips button
    if (strlen(tips) > 0)
    {
        m_tipsButton = new QPushButton(this);
        m_tipsButton->setObjectName("tipsButton");
        layout->addWidget(m_tipsButton);
    }

    layout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QListView* listView = new QListView(this);
    m_comboBox->setFocusPolicy(Qt::NoFocus);
    listView->setFocusPolicy(Qt::NoFocus);

    m_comboBox->setView(listView);

    bool suc = true;
    suc &= (bool)connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(onComboBoxIndexChanged(int)));

    Q_ASSERT(suc);
}

CSComboBox::~CSComboBox()
{

}

void CSComboBox::setItems(const QList<QPair<QString, QVariant>>& items)
{
    m_comboBox->clear();
    for (const auto& item : items)
    {
        m_comboBox->addItem(item.first, item.second);
    }
}

void CSComboBox::setValue(const QVariant& value)
{
    const int curIdx = m_comboBox->currentIndex();
    m_lastIndex = curIdx;

    for (int i = 0; i < m_comboBox->count(); i++)
    {
        auto itemData = m_comboBox->itemData(i);
        if (itemData == value)
        {
            if (i != curIdx)
            {
                m_comboBox->setCurrentIndex(i);
                m_lastIndex = m_comboBox->currentIndex();
            }
            break;
        }
    }
}

void CSComboBox::getValue(QVariant& value)
{
    value = m_comboBox->itemData(m_comboBox->currentIndex());
}

void CSComboBox::onComboBoxIndexChanged(int index)
{
    if (m_lastIndex != index)
    {
        m_lastIndex = index;
        emit valueChanged(getParaId(), m_comboBox->itemData(index));
    }
}

void CSComboBox::retranslate(const char* context)
{
    if (strlen(m_paraName) > 0)
    {       
        m_titleLabel->setText(QApplication::translate(context, m_paraName));
    }

    if(m_tipsButton)
    {
        m_tipsButton->setToolTip(QApplication::translate(context, m_tips));
    }
}

void CSComboBox::clearValues()
{
    m_comboBox->clear();
}
