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

#include "cslistitem.h"

#include <QHBoxLayout>
#include <QApplication> 

CSListItem::CSListItem(const QIcon& icon, const QString& text, const char* onText, const char* offText, const char* context,QListWidget* view)
    : QWidget(view)
    , listWidgetItem(new QListWidgetItem)
    , actionOnText(onText)
    , actionOffText(offText)
    , context(context)
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

void CSListItem::enterEvent(QEvent* event)
{
    actionLabel->setVisible(true);
    if (isSelected)
    {
        actionLabel->setText(getTranslate(actionOnText));
    }
    else
    {
        actionLabel->setText(getTranslate(actionOffText));
    }

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
        actionLabel->setText(getTranslate(actionOnText));
    }
    else
    {
        actionLabel->setText(getTranslate(actionOffText));
    }
}

bool CSListItem::isSelect()
{
    return isSelected;
}

QString CSListItem::getText() const
{
    return textLabel->text();
}

QString CSListItem::getTranslate(const char* text)
{
    return QApplication::translate(context, text);
}