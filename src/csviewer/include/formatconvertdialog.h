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

#ifndef _CS_FORMATCONVERTDIALOG_H
#define _CS_FORMATCONVERTDIALOG_H
#include <QDialog>

namespace Ui 
{
    class FormatConvertDialog;
}

namespace cs 
{
    class FormatConverter;
}

class FormatConvertDialog : public QDialog
{
    Q_OBJECT
public:
    FormatConvertDialog();
    ~FormatConvertDialog();
    void onTranslate();
    void reject() override;
    void showEvent(QShowEvent* event) override;
signals:
    void showMessage(QString msg, int time);
public slots:
    void onConvertStateChanged(int state, int convertedCount, QString message);
private slots:
    void onClickedBrowseSource();
    void onClickedBrowseOutputDirectory();
    void onShowTextureChanged(bool show);
    void onSourceFilePathChanged();
    void onOutputPathChanged();
private:
    void showMessageBox(QString message);
private:
    Ui::FormatConvertDialog* m_ui;
    cs::FormatConverter* m_formatConverter = nullptr;
};

#endif //  _CS_FORMATCONVERTDIALOG_H