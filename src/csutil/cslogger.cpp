/*******************************************************************************
* This file is part of the 3DViewer
*
* Copyright 2022-2026 (C) Revopoint3D AS
* All rights reserved.
*
* Revopoint3D Software License, v1.0
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistribution of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistribution in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of Revopoint3D AS nor the names of its contributors may be used
* to endorse or promote products derived from this software without specific
* prior written permission.
*
* 4. This software, with or without modification, must not be used with any
* other 3D camera than from Revopoint3D AS.
*
* 5. Any software provided in binary form under this license must not be
* reverse engineered, decompiled, modified and/or disassembled.
*
* THIS SOFTWARE IS PROVIDED BY REVOPOINT3D AS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL REVOPOINT3D AS OR CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Info:  https://www.revopoint3d.com
******************************************************************************/

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
    if (logFile.isOpen())
    {
        logFile.flush();
        logFile.close();
    }

    quit();
    wait();
}

void CSLogger::setLogRootDir(QString dir)
{
    logRootDir = dir;
}

void CSLogger::setLogPrefix(QString prefix)
{
    logPrefix = prefix;
}

void CSLogger::initialize()
{
    if (logRootDir.isEmpty())
    {
        qWarning() << "Please set log dir first.";
        return;
    }

    QDir dir(logRootDir);
    if (!dir.exists())
    {
        dir.mkpath(logRootDir);
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
    if (!logFile.isOpen())
    {
        QDateTime now = QDateTime::currentDateTime();
        QString day = now.toString("yyyyMMdd");
        QString logPath = logRootDir + "/" + logPrefix + "." + day + ".log";

        logFile.setFileName(logPath);
        if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            return;
        }
    }

    if (logFile.isOpen())
    {
        QTextStream ts(&logFile);
        ts << msg << "\n";
    }
}

// delete logs from a week ago
void CSLogger::delHistory()
{
    QDir dir(logRootDir);

    QString endDay = QDateTime::currentDateTime().addDays(-7).toString("yyyyMMdd");

    QStringList filters;
    filters << (logPrefix + ".*.log");
    dir.setNameFilters(filters);

    auto fileList = dir.entryList();
    foreach(QString file, fileList) {
        QStringList s = file.split(".");
        if (s.size() == 3 && s.at(1).length() == 8 && s.at(1) < endDay) {
            dir.remove(file);
        }
    }
}
