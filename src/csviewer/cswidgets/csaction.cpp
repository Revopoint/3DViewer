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

#include "csaction.h"

CSAction::CSAction(int type, QObject* parent)
    : QAction(parent)
    , m_type(type)
{
}

CSAction::CSAction(int type, const QString& text, QObject* parent)
    : QAction(text, parent)
    , m_type(type)
{
}

CSAction::CSAction(int type, const QIcon& icon, const QString& text, QObject* parent)
    : QAction(icon, text, parent)
    , m_type(type)
{

}

CSAction::~CSAction()
{

}

void CSAction::setType(int t)
{
    m_type = t;
}

int CSAction::getType() const
{
    return m_type;
}