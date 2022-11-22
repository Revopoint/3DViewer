#ifndef __BMP_UTIL__
#define __BMP_UTIL__

#include <stdio.h>
#include <string.h>
#include <vector>
#include "hpp/Camera.hpp"
#include "hpp/Processing.hpp"

namespace cs {


#pragma  pack(1)
typedef struct tagBITMAPFILEHEADER {
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER {
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagRGBQUAD {
	char rgbBlue;
	char rgbGreen;
	char rgbRed;
	char rgbReserved;
} RGBQUAD;
#pragma  pack()

static bool saveBmp(const char *data, int width, int height, int channel, const char *filename)
{
	BITMAPINFOHEADER pHeader; //定义信息头        
	pHeader.biSize = sizeof(BITMAPINFOHEADER);
	pHeader.biWidth = width;
	pHeader.biHeight = -height;
	pHeader.biPlanes = 1;
	pHeader.biBitCount = 8 * channel;
	pHeader.biCompression = 0;
	pHeader.biSizeImage = width * height * channel;
	pHeader.biXPelsPerMeter = 0;
	pHeader.biYPelsPerMeter = 0;
	pHeader.biClrUsed = 0;
	pHeader.biClrImportant = 0;
	
	FILE* fp = fopen(filename, "wb");
	if (fp)
	{
		BITMAPFILEHEADER fheader = { 0 };
		fheader.bfType = 'M' << 8 | 'B';
		int offset = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
		std::vector<RGBQUAD> quads;
		if (channel == 1)
		{
			quads.resize(256);
			for (int i = 0; i < 256; i++)                        //遍历源图像的颜色表并转换，然后写入目标文件
			{
				quads[i].rgbBlue = quads[i].rgbGreen = quads[i].rgbRed = i;
				quads[i].rgbReserved = 0;
			}
			offset += sizeof(RGBQUAD) * 256;
		}
		fheader.bfSize = offset + pHeader.biSizeImage;
		fheader.bfOffBits = offset;
		fwrite(&fheader, 1, sizeof(fheader), fp);
		fwrite(&pHeader, 1, sizeof(BITMAPINFOHEADER), fp);
		if (channel == 1)
		{
			fwrite(quads.data(), 1, sizeof(RGBQUAD) * 256, fp);
			fwrite(data, 1, pHeader.biSizeImage, fp);
		}
		else
		{
			//swap rgb to bgr
			std::vector<char> bgrData;
			bgrData.resize(pHeader.biSizeImage);
			char *dst_b= bgrData.data();
			char *dst_g = dst_b + 1;
			char *dst_r = dst_b + 2;
			const char *src_r = data;
			const char *src_g = data + 1;
			const char *src_b = data + 2;
			for (int i = 0; i < pHeader.biSizeImage; i += 3)
			{
				dst_r[i] = src_r[i];
				dst_g[i] = src_g[i];
				dst_b[i] = src_b[i];
			}
			fwrite(bgrData.data(), 1, pHeader.biSizeImage, fp);
		}
		fclose(fp);
		return true;
	}
	return false;
}

static bool readBmp(char *bmpName, std::vector<char> &bgrData, int &width, int &height)
{
    FILE *fp;
    if ((fp = fopen(bmpName, "rb")) == NULL)  //以二进制的方式打开文件
    {
        return false;
    }
    if (fseek(fp, sizeof(BITMAPFILEHEADER), 0))  //跳过BITMAPFILEHEADE
    {
        return false;
    }
    BITMAPINFOHEADER header;
    fread(&header, sizeof(BITMAPINFOHEADER), 1, fp);   //从fp中读取BITMAPINFOHEADER信息到header中,同时fp的指针移动
    width = header.biWidth;
    height = header.biHeight;
    int channel = header.biBitCount / 8;
    int linebyte = width * channel;
    if (channel == 3)
    {
        bgrData.resize(channel * width * height);
        for (int i = height - 1; i >= 0; i--)
        {
            fread(bgrData.data() + linebyte * i , sizeof(char), linebyte, fp);
        }
    }
    fclose(fp);   
    return true;
}

static bool convertBgrToGray(std::vector<char> &bgrData, int width, int height, std::vector<char> &grayData)
{
    if (bgrData.size() == width * height * 3)
    {
        int size = width * height;
        grayData.resize(size);
        for (size_t i = 0; i < size; i++)
        {
            grayData[i] = (bgrData[3 * i] + bgrData[3 * i + 1] + bgrData[3 * i + 2]) / 3;
        }
        return true;
    }
    return false;
}

static bool saveFrameData(const char* saveDir, cs::IFramePtr frame, int index)
{
	char acPath[260];
	if (frame->getFormat() == STREAM_FORMAT_RGB8)
	{
		snprintf(acPath, sizeof(acPath), "%srgb%dx%d-%d-%d.bmp", saveDir, frame->getWidth(), frame->getHeight()
			, index, frame->getSize());

		if (cs::saveBmp(frame->getData(), frame->getWidth(), frame->getHeight(), 3, acPath))
		{
			printf("save OK:%s\n", acPath);
		}
		else
		{
			printf("save bmp failed:%s\n", acPath);
		}
	}
	else if (frame->getFormat() == STREAM_FORMAT_MJPG)
	{
		snprintf(acPath, sizeof(acPath), "%srgb%dx%d-%d.jpg", saveDir, frame->getWidth(), frame->getHeight()
			, index);

		FILE* lpf = fopen(acPath, "wb");
		if (lpf == NULL)
		{
			printf("open file failed:%s\n", acPath);
		}

		fwrite((unsigned char*)frame->getData(), 1, frame->getSize(), lpf);
		fclose(lpf);

		printf("save OK:%s\n", acPath);
	}
	else if (frame->getFormat() == STREAM_FORMAT_Z16)
	{
		snprintf(acPath, sizeof(acPath), "%sz16-%dx%d-%d-%d.bmp", saveDir, frame->getWidth(), frame->getHeight()
			, index, frame->getSize());
		cs::colorizer color;
		int iLen = frame->getWidth()*frame->getHeight() * 2;
		if (frame->getSize() != iLen)
		{
			printf("the size is not match,format:%d,size:%d/%d\n", frame->getFormat(), frame->getSize(), iLen);
			return false;
		}

		std::vector<unsigned char> rgb;
		int length = frame->getHeight() * frame->getWidth();
		rgb.resize(length * 3);
		float scale = 0.1f;

		color.process((unsigned short *)frame->getData(FRAME_DATA_FORMAT_Z16), 0.1, rgb.data(), length);
		if (cs::saveBmp((const char *)rgb.data(), frame->getWidth(), frame->getHeight(), 3, acPath))
		{
			printf("save OK:%s\n", acPath);
		}
		else
		{
			printf("save bmp failed:%s\n", acPath);
		}
	}
	else if (frame->getFormat() == STREAM_FORMAT_Z16Y8Y8)
	{
		//Z16Y8Y8包含一个z16数据，一个左红外数和一个右红外数据
		snprintf(acPath, sizeof(acPath), "%sz16y8y8-%dx%d-%d-%d.bmp", saveDir, frame->getWidth(), frame->getHeight()
			, index, frame->getSize());
		cs::colorizer color;
		//整体的数据长度为 宽*高*4，z16数据每个像素2字节，红外数据每个像素1字节，2+1+1。
		int iLen = frame->getWidth()*frame->getHeight() * 4;
		if (frame->getSize() != iLen)
		{
			printf("the size is not match,format:%d,size:%d/%d\n",  frame->getFormat(), frame->getSize(), iLen);
			return false;
		}

		std::vector<unsigned char> rgb;
		int length = frame->getHeight() * frame->getWidth();
		rgb.resize(length * 3);
		float scale = 0.1f;

		//getData(FRAME_DATA_FORMAT_Z16)获取Z16数据
		color.process((unsigned short *)frame->getData(FRAME_DATA_FORMAT_Z16), scale, rgb.data(), length);
		if (cs::saveBmp((const char *)rgb.data(), frame->getWidth(), frame->getHeight(), 3, acPath))
		{
			printf("save OK:%s\n", acPath);
		}
		else
		{
			printf("save bmp failed:%s\n", acPath);
		}

		snprintf(acPath, sizeof(acPath), "%sdepth-%d-%dx%d-%d-%d_y8-left.bmp", saveDir, frame->getFormat()
			, frame->getWidth(), frame->getHeight()
			, index, frame->getSize()/2);

		//getData(FRAME_DATA_FORMAT_IR_LEFT)获取左红外数据
		cs::saveBmp((const char *)frame->getData(FRAME_DATA_FORMAT_IR_LEFT)
			, frame->getWidth(), frame->getHeight(), 1, acPath);

		snprintf(acPath, sizeof(acPath), "%sdepth-%d-%dx%d-%d-%d_y8-right.bmp", saveDir, frame->getFormat()
			, frame->getWidth(), frame->getHeight()
			, index, frame->getSize()/2);

		//getData(FRAME_DATA_FORMAT_IR_RIGHT)获取右红外数据
		cs::saveBmp((const char *)frame->getData(FRAME_DATA_FORMAT_IR_RIGHT)
			, frame->getWidth(), frame->getHeight(), 1, acPath);
	}
	else if (frame->getFormat() == STREAM_FORMAT_GRAY)
	{
		int iLen = frame->getWidth()*frame->getHeight() * 2;
	}
	else if (frame->getFormat() == STREAM_FORMAT_PAIR)
	{
		int iLen = frame->getWidth()*frame->getHeight();
		if (frame->getSize() != iLen*2)
		{
			printf("the size is not match,format:%d,size:%d/%d\n",  frame->getFormat(), frame->getSize(), iLen);
			return false;
		}

		snprintf(acPath, sizeof(acPath), "%spair-%dx%d-%d-%d-left.bmp", saveDir, frame->getWidth(), frame->getHeight()
			, index, frame->getSize()/2);
		
		cs::saveBmp((const char *)frame->getData(FRAME_DATA_FORMAT_IR_LEFT)
			, frame->getWidth(), frame->getHeight(), 1, acPath);

		snprintf(acPath, sizeof(acPath), "%spair-%dx%d-%d-%d-right.bmp", saveDir, frame->getWidth(), frame->getHeight()
			, index, frame->getSize()/2);

		cs::saveBmp((const char *)frame->getData(FRAME_DATA_FORMAT_IR_RIGHT)
			, frame->getWidth(), frame->getHeight(), 1, acPath);
		printf("save Ok:%s\n", acPath);
	}
	else
	{
		printf("unknow format %d\n", frame->getFormat());
	}

	return true;
}


};
#endif