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

#ifndef _CS_ACTION_H
#define _CS_ACTION_H

#include <QAction>

class CSAction : public QAction
{
    Q_OBJECT
public:
    CSAction(int type, QObject* parent = nullptr);
    CSAction(int type, const QString& text, QObject* parent = nullptr);
    CSAction(int type, const QIcon& icon, const QString& text, QObject* parent = nullptr);

    ~CSAction();
    void setType(int t);
    int getType() const;
private:
    int m_type;
};

#endif