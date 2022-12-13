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

#include "ipsettingdialog.h"
#include "ui_ipsetting.h"
#include "csapplication.h"

#include <icscamera.h>
#include <QMessageBox>
#include <QIntValidator>

IpSettingDialog::IpSettingDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::IpSettingWidget)
{
    ui->setupUi(this);

    ui->applyButton->setProperty("isCSStyle", true);
    
    QIntValidator* intValidator = new QIntValidator(0, 255, this);
    QIntValidator* intValidator2 = new QIntValidator(1, 255, this);
    QIntValidator* intValidator3 = new QIntValidator(0, 254, this);

    ui->ipEdit1->setValidator(intValidator2);
    ui->ipEdit2->setValidator(intValidator);
    ui->ipEdit3->setValidator(intValidator);
    ui->ipEdit4->setValidator(intValidator3);

    bool suc = true;
    suc &= (bool)connect(ui->applyButton, &QPushButton::clicked, this, &IpSettingDialog::onApply);
    suc &= (bool)connect(ui->autoIPButton, &QRadioButton::toggled, this, [=](bool checked) 
        {
            ui->ipArea->setEnabled(!checked);
        });
    
    QVector<CSLineEdit*> lineEdits = { ui->ipEdit1,  ui->ipEdit2 ,  ui->ipEdit3 ,  ui->ipEdit4 };
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

    ui->autoIPButton->setChecked(autoEnable);
    ui->manualIPButton->setChecked(!autoEnable);

    ui->ipEdit1->setText(QString::number(ip1));
    ui->ipEdit2->setText(QString::number(ip2));
    ui->ipEdit3->setText(QString::number(ip3));
    ui->ipEdit4->setText(QString::number(ip4));

    ui->ipArea->setEnabled(!autoEnable);
}

void IpSettingDialog::onApply()
{
    CameraIpSetting cameraIp;
    cameraIp.autoEnable = ui->autoIPButton->isChecked() ? 1 : 0;

    cameraIp.ipBytes[0] = (ui->ipEdit1->text().toInt()) & 0xFF;
    cameraIp.ipBytes[1] = (ui->ipEdit2->text().toInt()) & 0xFF;
    cameraIp.ipBytes[2] = (ui->ipEdit3->text().toInt()) & 0xFF;
    cameraIp.ipBytes[3] = (ui->ipEdit4->text().toInt()) & 0xFF;

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
    ui->retranslateUi(this);
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