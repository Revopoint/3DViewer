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

#include <QThread>
#include "cscameraapi.h"

namespace cs
{

class CapturedZipParser;
class CS_CAMERA_EXPORT FormatConverter : public QThread
{
    Q_OBJECT
public:

    enum CONVERT_STATE
    {
        CONVERTING,
        CONVERT_SUCCESS,
        CONVERT_FAILED,
        CONVERT_ERROR
    };
    FormatConverter();
    ~FormatConverter();

    void setSourceFile(QString sourceFile);
    void setOutputDirectory(QString output);
    void setWithTexture(bool withTexture);

    bool getIsConverting();
    void setIsConverting(bool value);
    void setInterruptConvert(bool value);
    bool getInterruptConvert();
public slots:
    void onConvert();
signals:
    void convertStateChanged(int state, int progress, QString message);
private:
    // zip file
    QString sourceFile;
    // output directory
    QString outputDirectory;

    bool withTexture = false;

    CapturedZipParser* capturedZipParser = nullptr;

    bool isConverting = false;
    bool interruptConvert = false;
};

}
