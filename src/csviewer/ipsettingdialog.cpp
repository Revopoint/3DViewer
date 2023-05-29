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

#include "ipsettingdialog.h"
#include "ui_ipsetting.h"
#include "csapplication.h"

#include <icscamera.h>
#include <QMessageBox>
#include <QIntValidator>

IpSettingDialog::IpSettingDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::IpSettingWidget)
{
    m_ui->setupUi(this);

    m_ui->applyButton->setProperty("isCSStyle", true);
    
    QIntValidator* intValidator = new QIntValidator(0, 255, this);
    QIntValidator* intValidator2 = new QIntValidator(1, 255, this);
    QIntValidator* intValidator3 = new QIntValidator(0, 254, this);

    m_ui->ipEdit1->setValidator(intValidator2);
    m_ui->ipEdit2->setValidator(intValidator);
    m_ui->ipEdit3->setValidator(intValidator);
    m_ui->ipEdit4->setValidator(intValidator3);

    bool suc = true;
    suc &= (bool)connect(m_ui->applyButton, &QPushButton::clicked, this, &IpSettingDialog::onApply);
    suc &= (bool)connect(m_ui->autoIPButton, &QRadioButton::toggled, this, [=](bool checked) 
        {
            m_ui->ipArea->setEnabled(!checked);
        });
    
    QVector<CSLineEdit*> lineEdits = { m_ui->ipEdit1,  m_ui->ipEdit2 ,  m_ui->ipEdit3 ,  m_ui->ipEdit4 };
    for (auto lineEdit : lineEdits)
    {
        suc &= (bool)connect(lineEdit, &CSLineEdit::focusOutSignal, this, &IpSettingDialog::onLineEditFocusOut);
    }

    Q_ASSERT(suc);
}

IpSettingDialog::~IpSettingDialog()
{

}

void IpSettingDialog::showEvent(QShowEvent* event)
{
    updateIpInfo();
    QDialog::showEvent(event);
}

void IpSettingDialog::updateIpInfo()
{
    auto camera = cs::CSApplication::getInstance()->getCamera();

    QVariant value;
    camera->getCameraPara(cs::parameter::PARA_CAMERA_IP, value);
    auto cameraIp = value.value<CameraIpSetting>();
    
    bool autoEnable = cameraIp.autoEnable;
    uchar ip1 = cameraIp.ipBytes[0];
    uchar ip2 = cameraIp.ipBytes[1];
    uchar ip3 = cameraIp.ipBytes[2];
    uchar ip4 = cameraIp.ipBytes[3];

    m_ui->autoIPButton->setChecked(autoEnable);
    m_ui->manualIPButton->setChecked(!autoEnable);

    m_ui->ipEdit1->setText(QString::number(ip1));
    m_ui->ipEdit2->setText(QString::number(ip2));
    m_ui->ipEdit3->setText(QString::number(ip3));
    m_ui->ipEdit4->setText(QString::number(ip4));

    m_ui->ipArea->setEnabled(!autoEnable);
}

void IpSettingDialog::onApply()
{
    CameraIpSetting cameraIp;
    cameraIp.autoEnable = m_ui->autoIPButton->isChecked() ? 1 : 0;

    cameraIp.ipBytes[0] = (m_ui->ipEdit1->text().toInt()) & 0xFF;
    cameraIp.ipBytes[1] = (m_ui->ipEdit2->text().toInt()) & 0xFF;
    cameraIp.ipBytes[2] = (m_ui->ipEdit3->text().toInt()) & 0xFF;
    cameraIp.ipBytes[3] = (m_ui->ipEdit4->text().toInt()) & 0xFF;

    // set IP
    auto camera = cs::CSApplication::getInstance()->getCamera();
    QVariant value;
    camera->getCameraPara(cs::parameter::PARA_CAMERA_IP, value);
    auto cameraIp2 = value.value<CameraIpSetting>();

    if ((cameraIp.autoEnable == cameraIp2.autoEnable) 
        && (cameraIp.ipBytes[0] == cameraIp2.ipBytes[0])
        && (cameraIp.ipBytes[1] == cameraIp2.ipBytes[1])
        && (cameraIp.ipBytes[2] == cameraIp2.ipBytes[2])
        && (cameraIp.ipBytes[3] == cameraIp2.ipBytes[3]))
    {
        emit showMessage(tr("The IP address has not been modified."), 5000);
    }
    else 
    {
        value = QVariant::fromValue(cameraIp);
        camera->setCameraPara(cs::parameter::PARA_CAMERA_IP, value);

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Tips"));
        msgBox.setText(tr("The IP address has been modified. Restarting the camera takes effect. Do you want to restart now ?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.button(QMessageBox::Yes)->setText(tr("Yes"));
        msgBox.button(QMessageBox::No)->setText(tr("No"));

        int button = msgBox.exec();

        if (button == QMessageBox::No)
        {
            close();
        }
        else if (button == QMessageBox::Yes)
        {
            emit cs::CSApplication::getInstance()->restartCamera();
            close();
        }
    }
}

void IpSettingDialog::onTranslate()
{
    m_ui->retranslateUi(this);
}

void IpSettingDialog::onLineEditFocusOut()
{
    QLineEdit* obj = qobject_cast<QLineEdit*>(sender());
    if (obj)
    {
        auto tex = obj->text();

        int num = 0;
        auto validator = qobject_cast<const QIntValidator*>(obj->validator());

        if (validator && validator->validate(tex, num) != QIntValidator::Acceptable)
        {
            obj->setText(QString::number(validator->bottom()));
        }
    }
}