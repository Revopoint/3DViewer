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

#ifndef _CS_COMMON_HELPER_H
#define _CS_COMMON_HELPER_H

#include <QFile>
#include <QApplication>

class CommonHelper
{
public:
    static void setStyle(const QString& style) 
    {
        QFile qss(style);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }

    static void setFont(const QString& font, int fontSize = 12, bool bold = false, int weight = 1)
    {
        const int fontId = QFontDatabase::addApplicationFont(font);
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);

        if (fontFamilies.size() > 0) {
            QFont font(fontFamilies.at(0));
            font.setBold(bold);
            font.setPixelSize(fontSize);
            font.setWeight(weight);
            qApp->setFont(font);
        }
    }
};
#endif
