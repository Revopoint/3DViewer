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
    , m_listWidgetItem(new QListWidgetItem)
    , m_actionOnText(onText)
    , m_actionOffText(offText)
    , m_context(context)
{
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("CSListItem");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 0, 20, 0);
    m_iconLabel = new QLabel(this);

    auto size = icon.availableSizes().first();
    m_iconLabel->setFixedSize(size);
    m_iconLabel->setPixmap(icon.pixmap(size));

    layout->addWidget(m_iconLabel);
    
    m_textLabel = new QLabel(text, this);
    layout->addWidget(m_textLabel);
    layout->addItem(new QSpacerItem(20, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    m_actionLabel = new QLabel(this);
    layout->addWidget(m_actionLabel);

    m_actionLabel->setVisible(false);
    m_iconLabel->setVisible(m_isSelected);

    QSizePolicy sizePilicy;
    sizePilicy.setRetainSizeWhenHidden(true);
    m_iconLabel->setSizePolicy(sizePilicy);
}

CSListItem::~CSListItem()
{

}

QListWidgetItem* CSListItem::getListWidgetItem() const
{
    return m_listWidgetItem;
}

void CSListItem::enterEvent(QEvent* event)
{
    m_actionLabel->setVisible(true);
    if (m_isSelected)
    {
        m_actionLabel->setText(getTranslate(m_actionOnText));
    }
    else
    {
        m_actionLabel->setText(getTranslate(m_actionOffText));
    }

    m_textLabel->setStyleSheet("color:rgb(0, 164, 255)");
    m_actionLabel->setStyleSheet("color:rgb(0, 164, 255)");
    QWidget::enterEvent(event);
}

void CSListItem::leaveEvent(QEvent* event)
{
    m_actionLabel->setVisible(false);
    m_actionLabel->setStyleSheet("color:rgb(77, 77, 77)");
    m_textLabel->setStyleSheet("color:rgb(77, 77, 77)");

    QWidget::leaveEvent(event);
}

void CSListItem::mouseReleaseEvent(QMouseEvent* event)
{
    //QWidget::mouseReleaseEvent(event);

    emit toggled(m_isSelected, getText(), m_listWidgetItem);
}

void CSListItem::mousePressEvent(QMouseEvent* event)
{

}

void CSListItem::setSelected(bool select)
{
    m_isSelected = select;
    m_iconLabel->setVisible(m_isSelected);
    if (m_isSelected)
    {
        m_actionLabel->setText(getTranslate(m_actionOnText));
    }
    else
    {
        m_actionLabel->setText(getTranslate(m_actionOffText));
    }
}

bool CSListItem::isSelect()
{
    return m_isSelected;
}

QString CSListItem::getText() const
{
    return m_textLabel->text();
}

QString CSListItem::getTranslate(const char* text)
{
    return QApplication::translate(m_context, text);
}