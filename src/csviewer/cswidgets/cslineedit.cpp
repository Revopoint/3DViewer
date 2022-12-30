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

#include "cswidgets/cslineedit.h"
#include <QIntValidator>

CSLineEdit::CSLineEdit(QWidget* parent) 
    : QLineEdit(parent) 
{

}

void CSLineEdit::keyPressEvent(QKeyEvent* event)
{
    QLineEdit::keyPressEvent(event);

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (text().isEmpty())
        {
            emit editingFinished();
        }
    }
}

void CSLineEdit::focusOutEvent(QFocusEvent* e)
{
    QLineEdit::focusOutEvent(e);

    auto validator = qobject_cast<const QIntValidator*>(this->validator());
    auto tex = this->text();

    int num = 0;
    if (validator && validator->validate(tex, num) == QIntValidator::Acceptable)
    {
        this->setText(QString::number(tex.toInt()));
    }

    emit focusOutSignal();
}

void CSLineEdit::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);
    emit focusInSignal();
}