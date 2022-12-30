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
    tableWidget->clearContents();
}
