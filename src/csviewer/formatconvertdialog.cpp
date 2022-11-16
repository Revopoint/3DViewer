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
    QString openDir = cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath();

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
    QString openDir = cs::CSApplication::getInstance()->getAppConfig()->getDefaultSavePath();

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
    QMessageBox::information(this, tr("Tips"), message, QMessageBox::Yes);
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
        int button = QMessageBox::question(this, tr("Tips"),
            QString(tr("Converting, are you sure to stop now ?")),
            QMessageBox::Yes | QMessageBox::No);

        if (button == QMessageBox::No)
        {

        }
        else if (button == QMessageBox::Yes)
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
}