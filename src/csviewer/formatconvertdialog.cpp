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
    : m_ui(new Ui::FormatConvertDialog)
    , m_formatConverter(new cs::FormatConverter)
{
    m_ui->setupUi(this);

    m_ui->convertButton->setProperty("isCSStyle", true);
    m_ui->selectSrcButton->setProperty("isCSStyle", true);
    m_ui->selectOutputButton->setProperty("isCSStyle", true);

    setWindowFlags(this->windowFlags() & Qt::WindowCloseButtonHint);

    bool suc = true;
    suc &= (bool)connect(m_ui->convertButton,       &QPushButton::clicked, m_formatConverter, &cs::FormatConverter::onConvert);
    suc &= (bool)connect(m_formatConverter,         &cs::FormatConverter::convertStateChanged, this, &FormatConvertDialog::onConvertStateChanged);
    suc &= (bool)connect(m_ui->selectSrcButton,     &QPushButton::clicked, this, &FormatConvertDialog::onClickedBrowseSource);
    suc &= (bool)connect(m_ui->selectOutputButton,  &QPushButton::clicked, this, &FormatConvertDialog::onClickedBrowseOutputDirectory);
    suc &= (bool)connect(m_ui->withTextureCheckBox, &QCheckBox::toggled,   this, &FormatConvertDialog::onShowTextureChanged);

    suc &= (bool)connect(m_ui->lineEditSrc,    &QLineEdit::editingFinished, this, &FormatConvertDialog::onSourceFilePathChanged);
    suc &= (bool)connect(m_ui->lineEditOutput, &QLineEdit::editingFinished, this, &FormatConvertDialog::onOutputPathChanged);
}

FormatConvertDialog::~FormatConvertDialog()
{
    m_formatConverter->quit();
    m_formatConverter->wait();

    delete m_formatConverter;
    delete m_ui;
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
        
        m_ui->lineEditSrc->setText(filePath);
        m_ui->lineEditOutput->setText(outputDir);

        m_formatConverter->setSourceFile(filePath);
        m_formatConverter->setOutputDirectory(outputDir);

        emit m_formatConverter->loadFileSignal();
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
        m_ui->lineEditOutput->setText(filePath);

        m_formatConverter->setOutputDirectory(filePath);
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
    case cs::FormatConverter::CONVERT_LOADING:
        m_ui->convertButton->setEnabled(false);
        break;
    case cs::FormatConverter::CONVERT_READDY:
        m_ui->withTextureCheckBox->setEnabled(m_formatConverter->getHasRGBData());
        m_ui->convertButton->setEnabled(true);
        break;
    case cs::FormatConverter::CONVERTING:
        emit showMessage(message, 0);
        m_ui->convertProgress->setValue(progress);
        m_ui->convertButton->setEnabled(false);
        m_ui->withTextureCheckBox->setEnabled(false);
        break;
    case cs::FormatConverter::CONVERT_SUCCESS:
    case cs::FormatConverter::CONVERT_FAILED:
        showMessageBox(message);
        m_ui->convertButton->setEnabled(true);
        m_ui->withTextureCheckBox->setEnabled(true);
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
    m_formatConverter->setWithTexture(show);
}

void FormatConvertDialog::onTranslate()
{
    m_ui->retranslateUi(this);
}

void FormatConvertDialog::reject()
{
    if (m_formatConverter->getIsConverting())
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
            m_formatConverter->setInterruptConvert(true);
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
    m_formatConverter->setSourceFile(m_ui->lineEditSrc->text());
}

void FormatConvertDialog::onOutputPathChanged()
{
    m_formatConverter->setOutputDirectory(m_ui->lineEditOutput->text());
}

void FormatConvertDialog::showEvent(QShowEvent* event)
{
    m_ui->lineEditSrc->setText("");
    m_ui->lineEditOutput->setText("");
    m_ui->convertProgress->setValue(0);
    m_ui->withTextureCheckBox->setEnabled(false);
}