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

#ifndef _CS_IMAGE_BUTTON_H
#define _CS_IMAGE_BUTTON_H

#include <QPushButton>
#include <QIcon>
#include <QLabel>

class CSTextImageButton : public QPushButton
{
public:
    CSTextImageButton(const QIcon& icon, const char* text, Qt::LayoutDirection layoutDirection = Qt::LeftToRight, QWidget* parent = Q_NULLPTR);
    void retranslate(const char* context);
private:
    void initButton();
private:
    Qt::LayoutDirection layoutDirection;
    
    QIcon m_icon;
    QLabel* m_buttonIcon;
    QLabel* m_buttonText;
    const char* m_buttonTitle;
};


#endif //_CS_IMAGE_BUTTON_H
