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

#include <QDir>

#include "appconfig.h"
#include "./app_version.h"

#define CONFIG_SAVE_PATH  (QDir::homePath() + QDir::separator() + APP_NAME + QDir::separator() + "config.ini")

AppConfig::AppConfig()
{
    m_settings = new QSettings(CONFIG_SAVE_PATH, QSettings::IniFormat, this);

    m_language = m_settings->value("language", "en").toString();
    m_defaultSavePath = m_settings->value("defaultSavePath", QDir::homePath()).toString();
    m_autoNameWhenCaptring = m_settings->value("autoNameWhenCapturing", false).toBool();
}

AppConfig::~AppConfig()
{
    save();
}

void AppConfig::setLanguage(QString lan)
{
    m_language = lan;
    save();
}

void AppConfig::setDefaultSavePath(QString path)
{
    m_defaultSavePath = path;
    save();
}

QString AppConfig::getLanguage() const
{
    return m_language;
}

QString AppConfig::getDefaultSavePath() const
{
    return m_defaultSavePath;
}

void AppConfig::save()
{
    m_settings->setValue("language", m_language);
    m_settings->setValue("defaultSavePath", m_defaultSavePath);
    m_settings->setValue("autoNameWhenCapturing", m_autoNameWhenCaptring);
    m_settings->sync();
}

void AppConfig::setAutoNameWhenCapturing(bool autoName)
{
    m_autoNameWhenCaptring = autoName;
    save();
}

bool AppConfig::getAutoNameWhenCapturing() const
{
    return m_autoNameWhenCaptring;
}