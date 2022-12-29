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
    settings = new QSettings(CONFIG_SAVE_PATH, QSettings::IniFormat, this);

    language = settings->value("language", "en").toString();
    defaultSavePath = settings->value("defaultSavePath", QDir::homePath()).toString();
    autoNameWhenCaptring = settings->value("autoNameWhenCapturing", false).toBool();
}

AppConfig::~AppConfig()
{
    save();
}

void AppConfig::setLanguage(QString lan)
{
    language = lan;
    save();
}

void AppConfig::setDefaultSavePath(QString path)
{
    defaultSavePath = path;
    save();
}

QString AppConfig::getLanguage() const
{
    return language;
}

QString AppConfig::getDefaultSavePath() const
{
    return defaultSavePath;
}

void AppConfig::save()
{
    settings->setValue("language", language);
    settings->setValue("defaultSavePath", defaultSavePath);
    settings->setValue("autoNameWhenCapturing", autoNameWhenCaptring);
    settings->sync();
}

void AppConfig::setAutoNameWhenCapturing(bool autoName)
{
    autoNameWhenCaptring = autoName;
    save();
}

bool AppConfig::getAutoNameWhenCapturing() const
{
    return autoNameWhenCaptring;
}