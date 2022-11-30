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

#include "imageutil.h"
#include <png.h>
#include <QDebug>

static FILE* openFile(QString& path)
{
    FILE* fp = nullptr;

    QByteArray data = path.toLocal8Bit();
    fp = fopen(data.data(), "wb");

    return fp;
}

bool ImageUtil::saveGrayScale16ByLibpng(int width, int height, QByteArray data, QString path)
{
    png_structp pngPtr = nullptr;
    png_infop infoPtr = nullptr;
    FILE* fp = nullptr;

    int bitDepth = 16;
    uchar* pngData = (uchar*)data.data();

    bool result = true;
    do
    {
        fp = openFile(path);
        if (fp == nullptr) {
            qWarning() << "Open file failed, file : " << path;
            result = false;
            break;
        }

        pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (pngPtr == nullptr) {
            qWarning() << ("Could not allocate write struct");
            result = false;
            break;
        }

        // Initialize info structure
        infoPtr = png_create_info_struct(pngPtr);
        if (infoPtr == nullptr) {
            qWarning() << "Could not allocate info struct";
            result = false;
            break;
        }

        if (setjmp(png_jmpbuf(pngPtr))) {
            qWarning() << "Error during png creation";
            result = false;
            break;
        }

        png_init_io(pngPtr, fp);

        png_set_IHDR(pngPtr, infoPtr, width, height, bitDepth, PNG_FORMAT_GRAY, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(pngPtr, infoPtr);

        png_set_swap(pngPtr);

        int rowSize = width * (bitDepth / 8);

        for (int i = 0; i < height; i++) {
            int offset = i * rowSize;
            png_write_row(pngPtr, pngData + offset);
        }

        png_write_end(pngPtr, nullptr);

    } while (false);


    if (fp != nullptr)
        fclose(fp);

    if (pngPtr != nullptr || infoPtr != nullptr)
        png_destroy_write_struct(&pngPtr, &infoPtr);

    return result;
}

#define PNG_BYTES_TO_CHECK	8
int checkIsPng(FILE* fp)
{
    unsigned char checkheader[PNG_BYTES_TO_CHECK]; 
    if (fread(checkheader, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
        return -1;

    return png_sig_cmp(checkheader, 0, PNG_BYTES_TO_CHECK); 
}

class PngReadTool
{
public:
    PngReadTool(QByteArray pngData)
        : pngData(pngData)
    {
    
    }

    int readData(png_bytep outBytes, png_size_t byteCount)
    {
        int copyCount = byteCount;
        if (dataOffset + copyCount >= pngData.size())
        {
            copyCount = pngData.size() - dataOffset;
        }

        memcpy(outBytes, pngData.data() + dataOffset, copyCount);
        dataOffset += copyCount;

        //qInfo() << "read png data, offset:" << dataOffset;

        return copyCount;
    }

    void resetOffset()
    {
        dataOffset = 0;
    }
private:
    QByteArray pngData;
    int dataOffset = 0;
};

static void readPngDataCallBack(png_structp pngPtr, png_bytep outBytes, png_size_t byteCount)
{
    png_voidp ioPtr = png_get_io_ptr(pngPtr);
    if (ioPtr == NULL) {
        qWarning() << "ioPtr is null";
        return;
    }

    PngReadTool* readTool = (PngReadTool*)ioPtr;

    const size_t readCount = readTool->readData(outBytes, byteCount);

    if ((png_size_t)readCount != byteCount) {
        qWarning() << "read byte count error";
    }
}

bool ImageUtil::genPixDataFromPngData(QByteArray pngData, int& width, int& height, int& bitDepth, QByteArray& pixData)
{
    bool result = true;
    PngReadTool* readTool = new PngReadTool(pngData);

    png_structp pngPtr = nullptr;
    png_infop infoPtr = nullptr;

    do
    {
        pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (pngPtr == nullptr) {
            qWarning() << ("Could not allocate write struct");
            result = false;
            break;
        }
        // initialize info structure
        infoPtr = png_create_info_struct(pngPtr);
        if (infoPtr == nullptr) {
            qWarning() << "Could not allocate info struct";
            result = false;
            break;
        }

        if (setjmp(png_jmpbuf(pngPtr))) {
            qWarning() << "Error during png creation";
            result = false;
            break;
        }

        // check header
        unsigned char checkheader[PNG_BYTES_TO_CHECK]; 
        readTool->readData(checkheader, PNG_BYTES_TO_CHECK);
        if (!png_check_sig(checkheader, PNG_BYTES_TO_CHECK)) {
            qWarning() << "check header, not a png file";
            result = false;
            break;
        }

        png_set_read_fn(pngPtr, readTool, readPngDataCallBack);
        png_set_sig_bytes(pngPtr, PNG_BYTES_TO_CHECK);

        png_read_png(pngPtr, infoPtr, PNG_TRANSFORM_SWAP_ENDIAN, 0);

        width = png_get_image_width(pngPtr, infoPtr);
        height = png_get_image_height(pngPtr, infoPtr);
        bitDepth = png_get_bit_depth(pngPtr, infoPtr);

        int dataSize = width * height * (bitDepth / 8);
        pixData.resize(dataSize);

        // read png data
        int row_size = png_get_rowbytes(pngPtr, infoPtr);
        png_bytep* row_pointers = png_get_rows(pngPtr, infoPtr);

        for (int i = 0; i < height; i++) {
            memcpy(pixData.data() + i * row_size, row_pointers[i], row_size);
        }

    } while (false);

    if (pngPtr != nullptr || infoPtr != nullptr)
        png_destroy_read_struct(&pngPtr, &infoPtr, 0);

    delete readTool;
    return result;
}



