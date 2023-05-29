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

#include "cslogger.h"
#include <QDebug>
#include <QtGlobal>
#include <QMetaEnum>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>

#include <stdio.h>
#include <stdlib.h>

static CSLogger* logger = nullptr;

static void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QString file = context.file;
    QString line = QString::asprintf("%u", context.line);
    QString function = context.function;

    emit logger->log(type, msg, file, line, function);
}

CSLogger::CSLogger()
{
    moveToThread(this);
    qInstallMessageHandler(myMessageOutput);

    bool suc = (bool)connect(this, &CSLogger::log, this, &CSLogger::onLog);
    Q_ASSERT(suc);

    logger = this;

    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

CSLogger::~CSLogger()
{
    if (m_logFile.isOpen())
    {
        m_logFile.flush();
        m_logFile.close();
    }

    quit();
    wait();
}

void CSLogger::setLogRootDir(QString dir)
{
    m_logRootDir = dir;
}

void CSLogger::setLogPrefix(QString prefix)
{
    m_logPrefix = prefix;
}

void CSLogger::initialize()
{
    if (m_logRootDir.isEmpty())
    {
        qWarning() << "Please set log dir first.";
        return;
    }

    QDir dir(m_logRootDir);
    if (!dir.exists())
    {
        dir.mkpath(m_logRootDir);
    }

    // delete logs from a week ago
    delHistory();
    start();
}

void CSLogger::onAboutToQuit()
{
    disconnect(this, &CSLogger::log, this, &CSLogger::onLog);
    qInstallMessageHandler(0);
}

void CSLogger::onLog(int type, QString msg, QString file, QString line, QString function)
{
    QtMsgType msgType = (QtMsgType)type;

    QString sType;
    switch (type) {
    case QtDebugMsg:
        sType = QString().sprintf("%10s", "[Debug]"); ;
        break;
    case QtInfoMsg:
        sType = QString().sprintf("%10s", "[Info]"); ;
        break;
    case QtWarningMsg:
        sType = QString().sprintf("%10s", "[Warning]"); ;
        break;
    case QtCriticalMsg: 
        sType = QString().sprintf("%10s", "[Critical]"); ;
        break;
    case QtFatalMsg:
        sType = QString().sprintf("%10s", "[Fatal]"); ;
    }

    QDateTime now = QDateTime::currentDateTime();
    QString time = now.toString("yyyy-MM-dd HH:mm:ss:zzz");

    QStringList list = file.split(QRegExp("[/\\\\]"));
    //QString ouputMsg = QString("[%1] %2 :  %3  (%4 : %5 : %6)").arg(sType).arg(time).arg(msg).arg(file).arg(line).arg(function);
#ifdef  _DEBUG
    QString ouputMsg = QString("%2 %1 : %3  (%4 : %5)").arg(sType).arg(time).arg(msg).arg(list.last()).arg(line);
#else
    QString ouputMsg = QString("%2 %1 : %3").arg(sType).arg(time).arg(msg);
#endif
    printf("%s\n", ouputMsg.toStdString().c_str());
    // write to file
    redirect(ouputMsg);
}

void CSLogger::redirect(const QString& msg)
{
    if (!m_logFile.isOpen())
    {
        QDateTime now = QDateTime::currentDateTime();
        QString day = now.toString("yyyyMMdd");
        QString logPath = m_logRootDir + "/" + m_logPrefix + "." + day + ".log";

        m_logFile.setFileName(logPath);
        if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            return;
        }
    }

    if (m_logFile.isOpen())
    {
        QTextStream ts(&m_logFile);
        ts << msg << "\n";
    }
}

// delete logs from a week ago
void CSLogger::delHistory()
{
    QDir dir(m_logRootDir);

    QString endDay = QDateTime::currentDateTime().addDays(-7).toString("yyyyMMdd");

    QStringList filters;
    filters << (m_logPrefix + ".*.log");
    dir.setNameFilters(filters);

    auto fileList = dir.entryList();
    foreach(QString file, fileList) {
        QStringList s = file.split(".");
        if (s.size() == 3 && s.at(1).length() == 8 && s.at(1) < endDay) {
            dir.remove(file);
        }
    }
}
