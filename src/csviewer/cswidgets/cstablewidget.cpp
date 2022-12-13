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

#include "cswidgets/cstablewidget.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QTableWidgetItem>
#include <hpp/Types.hpp>

Q_DECLARE_METATYPE(HdrExposureParam);
Q_DECLARE_METATYPE(HdrExposureSetting);

CSTableWidget::CSTableWidget(int paraId, int cols, QStringList titleLabels, QWidget* parent)
    : CSParaWidget(paraId, "", parent)
    , tableWidget(new QTableWidget(this))
    , headers(titleLabels)
{
    setObjectName("CSTableWidget");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 20, 20, 0);
    layout->addWidget(tableWidget);

    tableWidget->setColumnCount(cols);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableWidget->setHorizontalHeaderLabels(titleLabels); 
    tableWidget->setFocusPolicy(Qt::NoFocus);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tableWidget->verticalScrollBar()->setDisabled(true);
    tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget->setVerticalScrollMode(QTableWidget::ScrollPerPixel);

    tableWidget->setFixedHeight(30+20);
}

CSTableWidget::~CSTableWidget()
{

}

void CSTableWidget::setValue(const QVariant& settings)
{
    const HdrExposureSetting& hdrSetting = settings.value<HdrExposureSetting>();
    const int count = hdrSetting.count;

    tableWidget->clearContents();
    tableWidget->setRowCount(count);
    tableWidget->setFixedHeight((30+2 )* (count+1)-1);

    for (int i = 0; i < count; i++)
    {
        QTableWidgetItem* item = new QTableWidgetItem(QString::number(i + 1));
        item->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(i, 0, item);
        item->setFlags(item->flags() & (~Qt::ItemIsSelectable) & (~Qt::ItemIsEditable));

        //exposure
        item = new QTableWidgetItem(QString::number(hdrSetting.param[i].exposure));
        item->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(i, 1, item);

        //gain
        item = item->clone();
        item->setText(QString::number(hdrSetting.param[i].gain));
        tableWidget->setItem(i, 2, item);
    }
}

void CSTableWidget::getValue(QVariant& value)
{
    HdrExposureSetting hdrSettings;
    const int rows = tableWidget->rowCount();
    
    hdrSettings.count = rows;

    for (int i = 0; i < rows; i++)
    {
        auto item1 = tableWidget->item(i, 1);
        auto exposure = item1->text().toUInt();

        auto item2 = tableWidget->item(i, 2);
        auto gain = item2->text().toUInt();

        hdrSettings.param[i].exposure = exposure;
        hdrSettings.param[i].gain = gain;
    }

    value = QVariant::fromValue(hdrSettings);
}

void CSTableWidget::retranslate(const char* context)
{
    QStringList tranHeaders;
    for (auto s : headers)
    {
        tranHeaders << QApplication::translate(context, s.toStdString().c_str());
    }
    tableWidget->setHorizontalHeaderLabels(tranHeaders);
}

void CSTableWidget::clearValues()
{
    tableWidget->clear();
}
