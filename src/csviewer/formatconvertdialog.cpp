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

#include "formatconvertdialog.h"
#include "ui_formatconvert.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include <formatconverter.h>

#include "appconfig.h"
#include "csapplication.h"

FormatConvertDialog::FormatConvertDialog()
    : ui(new Ui::FormatConvertDialog)
    , formatConverter(new cs::FormatConverter)
{
    ui->setupUi(this);

    ui->convertButton->setProperty("isCSStyle", true);
    ui->selectSrcButton->setProperty("isCSStyle", true);
    ui->selectOutputButton->setProperty("isCSStyle", true);

    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint);

    bool suc = true;
    suc &= (bool)connect(ui->convertButton,       &QPushButton::clicked, formatConverter, &cs::FormatConverter::onConvert);
    suc &= (bool)connect(formatConverter,         &cs::FormatConverter::convertStateChanged, this, &FormatConvertDialog::onConvertStateChanged);
    suc &= (bool)connect(ui->selectSrcButton,     &QPushButton::clicked, this, &FormatConvertDialog::onClickedBrowseSource);
    suc &= (bool)connect(ui->selectOutputButton,  &QPushButton::clicked, this, &FormatConvertDialog::onClickedBrowseOutputDirectory);
    suc &= (bool)connect(ui->withTextureCheckBox, &QCheckBox::toggled,   this, &FormatConvertDialog::onShowTextureChanged);

    suc &= (bool)connect(ui->lineEditSrc,    &QLineEdit::editingFinished, this, &FormatConvertDialog::onSourceFilePathChanged);
    suc &= (bool)connect(ui->lineEditOutput, &QLineEdit::editingFinished, this, &FormatConvertDialog::onOutputPathChanged);
}

FormatConvertDialog::~FormatConvertDialog()
{
    formatConverter->quit();
    formatConverter->wait();

    delete formatConverter;
    delete ui;
}

#define DEFAULT_DIR_NAME "output"
void FormatConvertDialog::onClickedBrowseSource()
{
    qInfo() << "click select source file";
    QString openDir = QString("file:///%1").arg(cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath());

    QString filters = "Zip file(*.zip)";
    QUrl url = QFileDialog::getOpenFileUrl(this, tr("Select source file"), openDir, filters);

    if (url.isValid())
    {
        QString filePath = url.toLocalFile();

        if (!filePath.endsWith(".zip"))
        {
            filePath += ".zip";
        }

        QFileInfo fileInfo(filePath);
        QString outputDir = fileInfo.absolutePath() + "/" + DEFAULT_DIR_NAME;
        
        ui->lineEditSrc->setText(filePath);
        ui->lineEditOutput->setText(outputDir);

        formatConverter->setSourceFile(filePath);
        formatConverter->setOutputDirectory(outputDir);
    }
    else
    {
        qInfo() << "Cancel select file";
    }
}

void FormatConvertDialog::onClickedBrowseOutputDirectory()
{
    qInfo() << "click select  output directory";
    QString openDir = QString("file:///%1").arg(cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath());

    QUrl url = QFileDialog::getExistingDirectoryUrl(this, tr("Select source file"), openDir);

    if (url.isValid())
    {
        QString filePath = url.toLocalFile();
        ui->lineEditOutput->setText(filePath);

        formatConverter->setOutputDirectory(filePath);
    }
    else
    {
        qInfo() << "Cancel select file";
    }
}

void FormatConvertDialog::onConvertStateChanged(int state, int progress, QString message)
{
    emit showMessage(message, 5000);
    auto convertState = (cs::FormatConverter::CONVERT_STATE)state;
    switch (convertState)
    {
    case cs::FormatConverter::CONVERTING:
        emit showMessage(message, 0);
        ui->convertProgress->setValue(progress);
        ui->convertButton->setEnabled(false);
        break;
    case cs::FormatConverter::CONVERT_SUCCESS:
    case cs::FormatConverter::CONVERT_FAILED:
        showMessageBox(message);
        ui->convertButton->setEnabled(true);
        break;
    case cs::FormatConverter::CONVERT_ERROR:
        break;
    default:
        break;
    }
}

void FormatConvertDialog::showMessageBox(QString message)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(tr("Tips"));
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.button(QMessageBox::Yes)->setText(tr("Yes"));

    msgBox.exec();
}

void FormatConvertDialog::onShowTextureChanged(bool show)
{
    formatConverter->setWithTexture(show);
}

void FormatConvertDialog::onTranslate()
{
    ui->retranslateUi(this);
}

void FormatConvertDialog::reject()
{
    if (formatConverter->getIsConverting())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Tips"));
        msgBox.setText(tr("Converting, are you sure to stop now ?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.button(QMessageBox::Yes)->setText(tr("Yes"));
        msgBox.button(QMessageBox::No)->setText(tr("No"));
        
        int ret = msgBox.exec();

        if (ret == QMessageBox::No)
        {

        }
        else if (ret == QMessageBox::Yes)
        {
            formatConverter->setInterruptConvert(true);
            QDialog::reject();
        }
    }
    else
    {
        QDialog::reject();
    }
}

void FormatConvertDialog::onSourceFilePathChanged()
{
    formatConverter->setSourceFile(ui->lineEditSrc->text());
}

void FormatConvertDialog::onOutputPathChanged()
{
    formatConverter->setOutputDirectory(ui->lineEditOutput->text());
}

void FormatConvertDialog::showEvent(QShowEvent* event)
{
    ui->lineEditSrc->setText("");
    ui->lineEditOutput->setText("");
    ui->convertProgress->setValue(0);
}