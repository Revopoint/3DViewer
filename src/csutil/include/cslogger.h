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

#ifndef _CS_LOGGER_H
#define _CS_LOGGER_H
#include <QThread>
#include <QMutex>
#include <QFile>

#include "csutilsapi.h"

class CS_UTILS_EXPORT CSLogger : public QThread
{
    Q_OBJECT
public:
    CSLogger();
    ~CSLogger();
    void setLogRootDir(QString dir);
    void setLogPrefix(QString prefix);
    void initialize();
public slots:
    void onLog(int type, QString msg, QString file, QString line, QString function);
    void onAboutToQuit();
signals:
    void log(int type, QString msg, QString file, QString line, QString function);
private:
    void redirect(const QString& msg);
    void delHistory();
private:
    QMutex m_mutex;
    QString m_logRootDir;
    QString m_logPrefix;
    QFile m_logFile;
};

#endif // _CS_LOGGER_H
