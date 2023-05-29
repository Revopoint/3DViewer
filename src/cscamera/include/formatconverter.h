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
        CONVERT_LOADING,
        CONVERT_READDY,
        CONVERT_LOADING_FAILED,
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
    
    bool getHasRGBData() const;
    void setHasRGBData(bool value);
public slots:
    void onConvert();
    void onLoadFile();
signals:
    void convertStateChanged(int state, int progress, QString message);
    void loadFileSignal();
private:
    // zip file
    QString m_sourceFile;
    // output directory
    QString m_outputDirectory;

    bool m_withTexture = false;

    CapturedZipParser* m_capturedZipParser = nullptr;

    bool m_isConverting = false;
    bool m_interruptConvert = false;
    
    bool m_isFileValid = false;
    bool m_hasRGBData = false;
};

}
