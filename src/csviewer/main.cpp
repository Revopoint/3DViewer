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

#include <QApplication>
#include <QFontDatabase>
#include <QDir>
#include <QDebug>

#include <commonhelper.h>
#include <cslogger.h>

#include "csapplication.h"
#include "viewerwindow.h"
#include "app_version.h"

int main(int argc, char *argv[])
{ 
    QApplication app(argc, argv);

    // initialize log
    CSLogger logger;
    logger.setLogRootDir(LOG_ROOT_DIR);
    logger.setLogPrefix(LOG_PREFIX);

    bool suc = QObject::connect(&app, &QApplication::aboutToQuit, &logger, &CSLogger::onAboutToQuit, Qt::DirectConnection);
    Q_ASSERT(suc);

    logger.initialize();

    // application information
    qInfo() << "";
    qInfo() << "****************************************";
    qInfo() << "Applicaion Name : "     << APP_NAME;
    qInfo() << "Applicaion Version : "  << APP_VERSION;
    qInfo() << "****************************************";

    cs::CSApplication::getInstance()->start();
    ViewerWindow w;
    w.show();

    // set style and font
    QString stylePath = QString("%1/themes/global.css").arg(APP_PATH);
    QString fontPath = QString("%1/fonts/SourceHanSansCN-Regular.ttf").arg(APP_PATH);

    qInfo() << "set style and font";
    CommonHelper::setStyle(stylePath);
    CommonHelper::setFont(fontPath);
    
    return app.exec();
}
