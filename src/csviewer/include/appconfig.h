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

#ifndef _CS_APP_CONFIG_H
#define _CS_APP_CONFIG_H

#include <QSettings>

class AppConfig : public QObject
{
public:
    AppConfig();
    ~AppConfig();

    void setLanguage(QString lan);
    void setDefaultSavePath(QString path);
    void setAutoNameWhenCapturing(bool autoName);

    QString getLanguage() const;
    QString getDefaultSavePath() const;
    bool getAutoNameWhenCapturing() const;

private:
    void save();
private:
    QSettings* m_settings;
    
    //settings
    // frame save path
    QString m_defaultSavePath;
    // current language
    QString m_language;
    // Auto name when capturing
    bool m_autoNameWhenCaptring = false;
};

#endif //_CS_APP_CONFIG_H