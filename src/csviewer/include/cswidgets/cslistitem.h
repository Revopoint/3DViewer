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

#ifndef _CS_LIST_ITEM_H
#define _CS_LIST_ITEM_H

#include <QListWidgetItem>
#include <QIcon>
#include <QLabel>
#include <QListWidget>

class CSListItem : public QWidget
{
    Q_OBJECT
public:
    explicit CSListItem(const QIcon& icon, const QString& text, const char* onText, const char* offText, const char* context, QListWidget* view = Q_NULLPTR);
    ~CSListItem();
    QListWidgetItem* getListWidgetItem() const;
    void setSelected(bool select);
    bool isSelect();

    QString getText() const;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
signals:
    void toggled(bool selected, QString text, QListWidgetItem* listItem);
private:
    QString getTranslate(const char* text);
private:
    QLabel* m_iconLabel;
    QLabel* m_actionLabel;
    QLabel* m_textLabel;
    
    int m_itemIndex = 0;
    bool m_isSelected = false;
    const char* m_actionOnText;
    const char* m_actionOffText;
    const char* m_context;
    QListWidgetItem* m_listWidgetItem;
};

#endif // _CS_LIST_ITEM_H