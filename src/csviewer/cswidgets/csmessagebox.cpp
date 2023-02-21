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

#include "csmessagebox.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVariant>

CSMessageBox::CSMessageBox(QWidget* parent)
    : QDialog(parent)
    , m_confirmButton(new QPushButton(tr("OK"), this))
    , m_msgLabel(new QLabel(this))
{
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    resize(250, 200);
    m_msgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_msgLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    auto spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto spacerItem2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);


    vLayout->addItem(spacerItem2);
    vLayout->addWidget(m_msgLabel);
    vLayout->addItem(spacerItem);
    vLayout->addWidget(m_confirmButton);

    setObjectName("CSMessageBox");
    m_confirmButton->setProperty("isCSStyle", true);

    connect(m_confirmButton, &QPushButton::clicked, [=]() 
        {
            close();
            emit accepted();
        });
}

CSMessageBox::~CSMessageBox()
{

}

void CSMessageBox::updateMessage(QString msg)
{
    m_msgLabel->setText(msg);
}

void CSMessageBox::retranslate()
{
    m_confirmButton->setText(tr("OK"));
    setWindowTitle(tr("Tips"));
}

