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

#include "aboutdialog.h"
#include "ui_about.h"
#include "./app_version.h"

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutWidget)
{
    ui->setupUi(this);

    ui->closeButton->setProperty("isCSStyle", true);

    QString appName = QString("<strong>%1 %2</strong>").arg(APP_NAME).arg(APP_VERSION);
    ui->appNameLabel->setText(appName);

    QString appBuiltTime = QString(tr("Built on %1")).arg(APP_BUILD_TIME);
    ui->builtInfo->setText(appBuiltTime);

    connect(ui->closeButton, &QPushButton::clicked, this, [=]()
        {
            close();
        });
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onTranslate()
{
    ui->retranslateUi(this);
    QString appName = QString("<strong>%1 %2</strong>").arg(APP_NAME).arg(APP_VERSION);
    ui->appNameLabel->setText(appName);

    QString appBuiltTime = QString(tr("Built on %1")).arg(APP_BUILD_TIME);
    ui->builtInfo->setText(appBuiltTime);
}