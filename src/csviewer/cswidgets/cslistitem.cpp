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

#include "cslistitem.h"

#include <QHBoxLayout>

CSListItem::CSListItem(const QIcon& icon, const QString& text, QListWidget* view)
    : QWidget(view)
    , listWidgetItem(new QListWidgetItem)
{
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("CSListItem");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 0, 20, 0);
    iconLabel = new QLabel(this);

    auto size = icon.availableSizes().first();
    iconLabel->setFixedSize(size);
    iconLabel->setPixmap(icon.pixmap(size));

    layout->addWidget(iconLabel);
    
    textLabel = new QLabel(text, this);
    layout->addWidget(textLabel);
    layout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    actionLabel = new QLabel(this);
    layout->addWidget(actionLabel);

    actionLabel->setVisible(false);
    iconLabel->setVisible(isSelected);

    QSizePolicy sizePilicy;
    sizePilicy.setRetainSizeWhenHidden(true);
    iconLabel->setSizePolicy(sizePilicy);
}

CSListItem::~CSListItem()
{

}

QListWidgetItem* CSListItem::getListWidgetItem() const
{
    return listWidgetItem;
}

void CSListItem::setActionText(const QString& text, bool on)
{
    if (on)
    {
        actionOnText = text;
    }
    else 
    {
        actionOffText = text;
    }

    setSelected(isSelected);
}

void CSListItem::enterEvent(QEvent* event)
{
    actionLabel->setVisible(true);

    textLabel->setStyleSheet("color:rgb(0, 164, 255)");
    actionLabel->setStyleSheet("color:rgb(0, 164, 255)");
    QWidget::enterEvent(event);
}

void CSListItem::leaveEvent(QEvent* event)
{
    actionLabel->setVisible(false);
    actionLabel->setStyleSheet("color:rgb(77, 77, 77)");
    textLabel->setStyleSheet("color:rgb(77, 77, 77)");

    QWidget::leaveEvent(event);
}

void CSListItem::mouseReleaseEvent(QMouseEvent* event)
{
    //QWidget::mouseReleaseEvent(event);

    emit toggled(isSelected, getText(), listWidgetItem);
}

void CSListItem::mousePressEvent(QMouseEvent* event)
{

}

void CSListItem::setSelected(bool select)
{
    isSelected = select;
    iconLabel->setVisible(isSelected);
    if (isSelected)
    {
        actionLabel->setText(actionOnText);
    }
    else
    {
        actionLabel->setText(actionOffText);
    }
}

QString CSListItem::getText() const
{
    return textLabel->text();
}