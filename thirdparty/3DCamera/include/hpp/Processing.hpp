#ifndef __PROCESSING_HPP__
#define __PROCESSING_HPP__

#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <memory.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include "Types.hpp"
#define VALIDATE(x) (x < 90000.f)

namespace cs
{
	typedef struct float3 { 
		float x, y, z;
		float3() { x = 0;  y = 0; z = 0; };
		float3(float a, float b, float c){x = a; y=b; z=c;}
	}float3;
	
	inline float3 operator + (const float3 & a, const float3 & b) { return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
	inline float3 operator - (const float3 & a, const float3 & b) { return float3(a.x - b.x, a.y - b.y, a.z - b.z); }
	inline float3 operator * (const float3 & a, float b) { return float3(a.x * b, a.y * b, a.z * b); }

	typedef struct float2{
		float u, v;
		float2(){};
		float2(float a, float b){u = a;v = b;}
	}float2;

    typedef struct int3 {
        int x, y, z;
        int3() {};
        int3(int a, int b, int c) { x = a; y = b; z = c; };
    }int3;
/**
* @~chinese
* \defgroup Process 深度图后处理
* @brief 深度图后处理接口
* @{
* @~english
* \defgroup Process Depth map's common operation
* @brief Camera Object Interface Class
* @{
*/

	/*!\class Pointcloud
	* @~chinese
	* @brief 点云类
	* @~english 
	* @brief Pointcloud class 
	**/
	class Pointcloud
	{
	public:
		Pointcloud(){};
		virtual ~Pointcloud(){};

		/**
		* @~chinese
		* @brief 将深度图计算为点云
		* @param[in] depthMap			深度图地址
		* @param[in] width				深度图宽度
		* @param[in] height				深度图高度
		* @param[in] depthScale			深度数据的缩放系数,通过ICamera::getPropertyExtension(PROPERTY_EXT_DEPTH_SCALE, ...)获得
		* @param[in] intrinsicsDepth	深度流的内参, 通过ICamera::getIntrinsic(STREAM_TYPE_DEPTH, ...)获得
		* @param[in] intrinsicsRgb		RGB流内参,通过ICamera::getIntrinsics(STREAM_TYPE_RGB, ...)获得，
		* @								如果不带纹理则传入nullptr
		* @param[in] extrinsics			深度流到RGB流的旋转平移矩阵, 通过ICamera::getExtrinsics获得
		*								如果不带纹理则传入nullptr
		* @param[in] removeInvalid		true: 移除无效点, false: 将无效点设置为(0,0,0)
		* @~english
		* @brief Calculate pointcloud from depth map
		* @param[in] depthMap			The address of the depth map
		* @param[in] width				The width of the depth map
		* @param[in] height				The height of the depth map
		* @param[in] depthScale			The depthScale of the depth, return by ICamera::getPropertyExtension(PROPERTY_EXT_DEPTH_SCALE, ...)
		* @param[in] intrinsicsDepth	The intrinsics of the depth stream, return by ICamera::getIntrinsics(STREAM_TYPE_DEPTH, ...)
		* @param[in] intrinsicsRgb		The intrinsics of the RGB stream, return by ICamera::getIntrinsics(STREAM_TYPE_RGB, ...)
		* @								it should be setted to nullptr if no texture
		* @param[in] extrinsics			The extrinsics of the RGB stream, return by ICamera::getExtrinsics
		* @								it should be setted to nullptr if no texture
		* @param[in] removeInvalid		true: remove invalid point, false: set invalid point to (0,0,0)
		*/
		template<class T>
		void generatePoints(T *depthMap, int width, int height, float depthScale,Intrinsics *intrinsicsDepth, Intrinsics *intrinsicsRgb, Extrinsics *extrinsics, bool removeInvalid = false)
		{
			_points.clear();
			_textures.clear();
			_normals.clear();
			float3 *points = (float3 *)malloc(width * height * sizeof(float3));
			float3 *normals = (float3 *)malloc(width * height * sizeof(float3));

			Intrinsics intrDepth;
			memcpy(&intrDepth, intrinsicsDepth, sizeof(Intrinsics));
			intrDepth.fx *= float(width) / intrinsicsDepth->width;
			intrDepth.cx *= float(width) / intrinsicsDepth->width;
			intrDepth.fy *= float(height) / intrinsicsDepth->height;
			intrDepth.cy *= float(height) / intrinsicsDepth->height;
			
#pragma omp parallel for
			for(int v = 0; v < height; v++)
			{
				for(int u = 0; u < width; u++)
				{
					int index = v*width + u;
					float z = depthMap[index] * depthScale;
					if(z > 0.f)
					{
						points[index].x = (u - intrDepth.cx) * z / intrDepth.fx;
						points[index].y = (v - intrDepth.cy) * z / intrDepth.fy;
						points[index].z = z;
					}
					else
					{
						points[index].x = points[index].y = points[index].z = 0.0;
					}

					normals[index].x = normals[index].y = normals[index].z = 0.0;
				}
			}

			calculateNormals(points, normals, width, height);

			insertPoints(points, normals, width, height, extrinsics, intrinsicsRgb, removeInvalid);
			
			free(points);
			free(normals);
		}

		/**
		* @~chinese
		* @brief 生成rgb图像（转换到深度图大小）位置到深度图位置的映射表
		* @		 必须先调用generatePoints，传入rgb相机的参数，且removeInvalid设置为false生成纹理坐标，否则返回失败
		* @param[in]  depthWidth		深度图宽度
		* @param[in]  depthHeight		深度图高度
		* @param[out] rgbToDepth		输出映射表
		* @return     成功:true, 失败:false
		* @~english
		* @brief Generate a map of RGB image (converted to depth map size) coords to depth map coords
		* @		 You must call generatePoints and pass in rgb parameters, and removeInvalid is set to false to generate texture coordinates, otherwise false is returned
		* @param[in]  depthWidth		The width of the depth map
		* @param[in]  depthHeight		The height of the depth map
		* @param[out] rgbToDepth		output the map rgb coord to depth coord
		* @return     成功:true, 失败:false
		*/
		bool generateTextureToDepthMap(int depthWidth, int depthHeight, std::map<int, int> &rgbToDepth)
		{
			int length = depthHeight*depthWidth;
			if (!_textures.size() || _textures.size() != length)
				return false;
			int rgbIndex;
			for (size_t i = 0; i < length; i++)
			{
				rgbIndex = _textures[i].u * depthWidth + _textures[i].v * depthHeight * depthWidth;
				if (rgbIndex > 0)
				{
					rgbToDepth[rgbIndex] = i;
				}
			}
            return true;
		}

		/**
		* @~chinese
		* @brief 计算rgb坐标对应的深度图坐标
		* @		 必须先调用generateTextureToDepthMap生成map表
		* @param[in]  rgbToDepth		映射表
		* @param[in]  rgbX				rgb图X坐标
		* @param[in]  rgbY				rgb图Y坐标
		* @param[in]  rgbWidth			rgb图宽度
		* @param[in]  rgbHeight			rgb图高度
		* @param[in]  depthWidth		深度图宽度
		* @param[in]  depthHeight		深度图高度
		* @param[out] depthX			输出深度图X坐标
		* @param[out] depthY			输出深度图Y坐标
		* @return     成功:true, 失败:false
		* @~english
		* @brief Calculate the depth map coordinates corresponding to the RGB coordinates
		* @		 You must call generateTextureToDepthMap to generate map first
		* @param[in]  rgbToDepth		the map rgb coordinates to depth map coordinates
		* @param[in]  rgbX				The x-coordinates of rgb image
		* @param[in]  rgbY				The y-coordinates of rgb image
		* @param[in]  rgbWidth			The width of the rgb image
		* @param[in]  rgbHeight			The height of the rgb image
		* @param[in]  depthWidth		The width of the depth map
		* @param[in]  depthHeight		The height of the depth map
		* @param[out] depthX			Output the x-coordinates of depth map
		* @param[out] depthY			Output the y-coordinates of depth map
		* @return    成功:true, 失败:false
		*/
		bool getDepthCoordFromMap(std::map<int, int> &rgbToDepth, int rgbX, int rgbY, int rgbWidth, int rgbHeight,
			int depthWidth, int depthHeight, int &depthX, int &depthY)
		{
			rgbX = float(rgbX) / rgbWidth * depthWidth;
			rgbY = float(rgbY) / rgbHeight * depthHeight;
			int targetIndex, x, y;
			int index = 0;
			int xoffsets[] = { 0, 1, -1, 2, -2 };
			int yoffsets[] = { 0, 1, -1 };
			bool found = false;
			for (auto yoff : yoffsets)
			{
				y = rgbY + yoff;
				if (y >= 0 && y < depthHeight)
				{
					for (auto xoff : xoffsets)
					{
						x = rgbX + xoff;
						if (x >= 0 && x < depthWidth)
						{
							targetIndex = y*depthWidth + x;
							if (rgbToDepth.find(targetIndex) != rgbToDepth.end())
							{
								index = rgbToDepth[targetIndex];
								found = true;
								break;
							}
						}
					}
					if (found)
						break;
				}
			}
			if (found)
			{
				depthX = index % depthWidth;
				depthY = index / depthWidth;
			}
			return found;
		}
	
		/**
		* @~chinese
		* @brief 将深度图+光刀中心图的数据转换为点云
		* @param[in] data				数据起始地址
		* @param[in] width				深度图宽度
		* @param[in] height				深度图高度
		* @param[in] intrinsicsDepth	深度流的内参, 通过ICamera::getIntrinsic(STREAM_TYPE_DEPTH, ...)获得
		* @param[in] intrinsicsRgb		RGB流内参,通过ICamera::getIntrinsics(STREAM_TYPE_RGB, ...)获得，
		* @								如果不带纹理则传入nullptr
		* @param[in] extrinsics			深度流到RGB流的旋转平移矩阵, 通过ICamera::getExtrinsics获得
		*								如果不带纹理则传入nullptr
		* @~english
		* @brief Calculate pointcloud from depth map and x-offset map of laser
		* @param[in] data				The address of data
		* @param[in] width				The width of the depth map
		* @param[in] height				The height of the depth map
		* @param[in] intrinsicsDepth	The intrinsics of the depth stream, return by ICamera::getIntrinsics(STREAM_TYPE_DEPTH, ...)
		* @param[in] intrinsicsRgb		The intrinsics of the RGB stream, return by ICamera::getIntrinsics(STREAM_TYPE_RGB, ...)
		* @								it should be setted to nullptr if no texture
		* @param[in] extrinsics			The extrinsics of the RGB stream, return by ICamera::getExtrinsics
		*								it should be setted to nullptr if no texture
		**/
		int generatePointsFromCenterZ(float *data, int width, int height, Intrinsics *intrinsicsDepth, Intrinsics *intrinsicsRgb = nullptr, Extrinsics *extrinsics = nullptr)
		{
			_points.clear();
			_textures.clear();
			_normals.clear();

			float *depthMap = data + width*height;
			float *yMap = data;

			float3 *points = (float3 *)malloc(width * height * sizeof(float3));
			float3 *normals = (float3 *)malloc(width * height * sizeof(float3));

			Intrinsics intrDepth;
			memcpy(&intrDepth, intrinsicsDepth, sizeof(Intrinsics));
			float scale = float(width) / intrinsicsDepth->width;
			intrDepth.fx *= scale;
			intrDepth.cx *= scale;
			intrDepth.fy *= scale;
			intrDepth.cy *= scale;

			int index = 0;
			for (int v = 0; v < height; v++)
			{
				for (int u = 0; u < width; u++, index++)
				{
					float ptZ = depthMap[index];
					float y = yMap[index];
					if (VALIDATE(ptZ) && VALIDATE(y))
					{
						float ptX = (u - intrDepth.cx) * ptZ / intrDepth.fx;
						float ptY = (y - intrDepth.cy) * ptZ / intrDepth.fy;

						points[index].x = ptX;
						points[index].y = ptY;
						points[index].z = ptZ;

					}
					else
					{
						points[index].x = 0.0;
						points[index].y = 0.0;
						points[index].z = 0.0;
					}

					normals[index].x = 0.0;
					normals[index].y = 0.0;
					normals[index].z = 0.0;
				}
			}

			calculateNormals(points, normals, width, height);

			insertPoints(points, normals, width, height, extrinsics, intrinsicsRgb, false);

			free(points);
			free(normals);
			return 0;
		}
		
		/**
		* @~chinese 将STREAM_FORMAT_XZ32格式输出的帧数据转换为点云
		* \param[in] data				数据指针
		* \param[in] width				原始图像宽度
		* @~english
		* @brief Convert frame data which format is STREAM_FORMAT_XZ32 to point cloud
		* \param[in] data				The address of data
		* \param[in] width				The width of image
		**/
        /**
        * @~chinese 将STREAM_FORMAT_XZ32格式输出的帧数据转换为点云
        * \param[in] data				数据指针
        * \param[in] width				原始图像宽度
        * @~english
        * @brief Convert frame data which format is STREAM_FORMAT_XZ32 to point cloud
        * \param[in] data				The address of data
        * \param[in] width				The width of image
        **/
        int generatePointsFromXZ(float *data, int width, bool removeInvalid = false)
        {
            _points.clear();
            _textures.clear();
            _normals.clear();
            _validSize = 0;
            float *depthMap = data + width;
            float *XMap = data;

            for (int u = 0; u < width; u++)
            {
                if (VALIDATE(XMap[u]) && VALIDATE(depthMap[u]) && depthMap[u] > 0.f)
                {
                    _points.push_back(float3(XMap[u], 0.f, depthMap[u]));
                    _normals.push_back(float3(0.f, 0.f, 0.f));
                    _validSize++;
                }
                else if (!removeInvalid)
                {
                    _points.push_back(float3(0.f, 0.f, 0.f));
                    _normals.push_back(float3(0.f, 0.f, 0.f));
                }
            }

            return 0;
        }

		/**
		* @~chinese
		* @brief 计算深度图上的一个点
		* @param[out] vertex			返回顶点坐标
		* @param[out] textureCoord		返回RGB图像的映射坐标(已归一化至0~1.f)，乘以RGB图像的分辨率即可得到绝对坐标值
		* @param[in] u					深度图上的列坐标
		* @param[in] v					深度图上的行坐标
		* @param[in] depth				深度图数据指针
		* @param[in] width				深度图宽度
		* @param[in] height				深度图高度
		* @param[in] depthScale			深度图一个单位表示的物理长度（单位毫米）
		* @param[in] intrinsicsDepth	深度流的内参, 通过ICamera::getIntrinsic(STREAM_TYPE_DEPTH, ...)获得
		* @param[in] intrinsicsRgb		RGB流内参,通过ICamera::getIntrinsics(STREAM_TYPE_RGB, ...)获得，
		* @								如果不带纹理则传入nullptr
		* @param[in] extrinsics			深度流到RGB流的旋转平移矩阵, 通过getExtrinsics获得
		*								如果不带纹理则传入nullptr
		* @~english
		* @brief Calculate one point
		* @param[out] vertex			return coordinate of point
		* @param[out] textureCoord		return texture coordinate of point(0~1.f), multiply the image width/height to get the coordinates in image
		* @param[in] u					The x-coordinate of the depth map
		* @param[in] v					The y-coordinate of the depth map
		* @param[in] depth				the pointer of depth map
		* @param[in] width				The width of the depth map
		* @param[in] height				The height of the depth map
		* @param[in] depthScale			the physical length represented by a unit
		* @param[in] intrinsicsDepth	The intrinsics of the depth stream, return by ICamera::getIntrinsics(STREAM_TYPE_DEPTH, ...)
		* @param[in] intrinsicsRgb		The intrinsics of the RGB stream, return by ICamera::getIntrinsics(STREAM_TYPE_RGB, ...)
		* @								it should be setted to nullptr if no texture
		* @param[in] extrinsics			The extrinsics of the RGB stream, return by ICamera::getExtrinsics
		*								it should be setted to nullptr if no texture
		**/
		template<class T>
		void generatePoint(float3& vertex, float2& textureCoord, int u, int v, T depth, int width, int height, float depthScale,Intrinsics *intrinsicsDepth, Intrinsics *intrinsicsRgb = nullptr, Extrinsics *extrinsics = nullptr)
		{
			Intrinsics intrDepth;
			memcpy(&intrDepth, intrinsicsDepth, sizeof(Intrinsics));
			float scaleX = float(width) / intrinsicsDepth->width;
			float scaleY = float(height) / intrinsicsDepth->height;
			intrDepth.fx *= scaleX;
			intrDepth.cx *= scaleX;
			intrDepth.fy *= scaleY;
			intrDepth.cy *= scaleY;

			vertex = float3(0.f, 0.f, 0.f);
			textureCoord = float2(0.f, 0.f);

			float z = depth * depthScale;
			if (z > 0.f)
			{
				vertex.x = (u - intrDepth.cx) * z / intrDepth.fx;
				vertex.y = (v - intrDepth.cy) * z / intrDepth.fy;
				vertex.z = z;
				if (extrinsics && intrinsicsRgb)
				{
					float3 tmp = vertex;
					tmp.x += extrinsics->translation[0];
					tmp.y += extrinsics->translation[1];
					tmp.z += extrinsics->translation[2];
					float3 tmp2;
					tmp2.x = tmp.x * extrinsics->rotation[0] + tmp.y * extrinsics->rotation[1] + tmp.z * extrinsics->rotation[2];
					tmp2.y = tmp.x * extrinsics->rotation[3] + tmp.y * extrinsics->rotation[4] + tmp.z * extrinsics->rotation[5];
					tmp2.z = tmp.x * extrinsics->rotation[6] + tmp.y * extrinsics->rotation[7] + tmp.z * extrinsics->rotation[8];

					float fu = (intrinsicsRgb->fx * tmp2.x / tmp2.z + intrinsicsRgb->cx);
					float fv = (intrinsicsRgb->fy * tmp2.y / tmp2.z + intrinsicsRgb->cy);
					if (fu >= 0.00001f && fu < intrinsicsRgb->width && fv >= 0.00001f && fv < intrinsicsRgb->height)
					{
						textureCoord = float2(fu / intrinsicsRgb->width, fv / intrinsicsRgb->height);
					}
				}
			}
		}

		/**
		* @~chinese
		* @brief 将点云数据导出到文件
		* @param[in] filename			保存文件名
		* @param[in] texture			RGB图像的地址，如果不带颜色则传入空
		* @param[in] textureWidth		RGB图像的宽度
		* @param[in] textureHeight		RGB图像的高度
		* @param[in] binary				数据格式, 二进制/文本
		* @~english
		* @brief Export pointcloud to file
		* @param[in] filename			Target save filename
		* @param[in] texture			The address of RGB image, it should be setted to nullptr if no texture
		* @param[in] textureWidth		The width of RGB image
		* @param[in] textureHeight		The height of RGB image
		* @param[in] binary				File format, Binary/Ascii
		**/
		virtual void exportToFile(const std::string& filename, unsigned char *texture, int textureWidth, int textureHeight, bool binary = false)
		{
			const auto vertices = getVertices();
			const auto texcoords = getTexcoords();
			const auto normals = getNormals();

			std::string ext = filename.substr(filename.size() - 3, 3);
			std::string fileName = filename.substr(0, filename.size() - 4);
			if (ext == "ply")
			{
				std::ofstream out(filename);
				out << "ply\n";
				if (binary)
					out << "format binary_little_endian 1.0\n";
				else
					out << "format ascii 1.0\n";
				out << "comment pointcloud saved from 3DCamera\n";
				out << "element vertex " << size() << "\n";
				out << "property float x\n";
				out << "property float y\n";
				out << "property float z\n";
				out << "property float nx\n";
				out << "property float ny\n";
				out << "property float nz\n";
				if (texture)
				{
					out << "property uchar red\n";
					out << "property uchar green\n";
					out << "property uchar blue\n";
				}
				out << "end_header\n";
				out.close();

				if (binary)
				{
					out.open(filename, std::ios_base::app | std::ios_base::binary);
					for (int i = 0; i < size(); ++i)
					{
						// we assume little endian architecture on your device
						out.write(reinterpret_cast<const char*>(&(vertices[i].x)), sizeof(float));
						out.write(reinterpret_cast<const char*>(&(vertices[i].y)), sizeof(float));
						out.write(reinterpret_cast<const char*>(&(vertices[i].z)), sizeof(float));
						out.write(reinterpret_cast<const char*>(&(normals[i].x)), sizeof(float));
						out.write(reinterpret_cast<const char*>(&(normals[i].y)), sizeof(float));
						out.write(reinterpret_cast<const char*>(&(normals[i].z)), sizeof(float));

						if (texture)
						{
							int x, y;
							x = int(texcoords[i].u * textureWidth);
							if (x >= textureWidth) x = textureWidth - 1;
							if (x < 0)	x = 0;
							y = int(texcoords[i].v * textureHeight);
							if (y >= textureHeight) y = textureHeight - 1;
							if (y < 0)	y = 0;
							unsigned char *color = texture + (y * textureWidth + x) * 3;
							uint8_t r = color[0];
							uint8_t g = color[1];
							uint8_t b = color[2];
							out.write(reinterpret_cast<const char*>(&r), sizeof(uint8_t));
							out.write(reinterpret_cast<const char*>(&g), sizeof(uint8_t));
							out.write(reinterpret_cast<const char*>(&b), sizeof(uint8_t));
						}
					}
					out.close();
				}
				else
				{

					out.open(filename, std::ios_base::app);
					for (int i = 0; i < size(); ++i)
					{
						// we assume little endian architecture on your device
						out << (vertices[i].x) << " ";
						out << (vertices[i].y) << " ";
						out << (vertices[i].z) << " ";
						out << (normals[i].x) << " ";
						out << (normals[i].y) << " ";
						out << (normals[i].z) << " ";

						if (texture)
						{
							int x, y;
							x = int(texcoords[i].u * textureWidth);
							if (x >= textureWidth) x = textureWidth - 1;
							if (x < 0)	x = 0;
							y = int(texcoords[i].v * textureHeight);
							if (y >= textureHeight) y = textureHeight - 1;
							if (y < 0)	y = 0;
							unsigned char *color = texture + (y * textureWidth + x) * 3;
							uint8_t r = color[0];
							uint8_t g = color[1];
							uint8_t b = color[2];
							out << int(r) << " ";
							out << int(g) << " ";
							out << int(b) << " ";
						}
						out << "\n";
					}
					out.close();
				}
			}
			else if (ext == "csv")
			{
				FILE *fpout;
				fpout = fopen(filename.c_str(), "w+");
				//out.open(filename, std::ios_base::app);
				for (int i = 0; i < size(); ++i)
				{
					fprintf(fpout, "%.6f,%.6f,%.6f \n", vertices[i].x, vertices[i].y, vertices[i].z);
				}
				fclose(fpout);
			}
			else
			{
				FILE *fpout;
				fpout = fopen(filename.c_str(), "wb");
				FILE *fmap;
				fmap = fopen((fileName + ".map").c_str(), "wb");
				for (int i = 0; i < vertices.size(); ++i)
				{
					if (vertices[i].z <= 0.f)
						continue;
					fprintf(fpout, "v %.6f %.6f %.6f \n", vertices[i].x, vertices[i].y, vertices[i].z);
					fprintf(fpout, "vn %.6f %.6f %.6f \n", normals[i].x, normals[i].y, normals[i].z);
					if (texture)
					{
						fprintf(fmap, "%d %d\n", int(texcoords[i].u * textureWidth), textureHeight - int(texcoords[i].v * textureHeight));
					}
				}
				fclose(fpout);
				fclose(fmap);
			}
		}

		/**
		* @~chinese
		* @brief 获取总点数
		* @~english
		* @brief Return the size of points(inclue size of invalid points)
		**/
		int size()
		{
			return int(_points.size());
		}
		/**
		* @~chinese
		* @brief 获取有效的点数
		* @~english
		* @brief Return the size of valid points(exclude size of invalid points)
		**/
		int validSize()
		{
			return _validSize;
		}
		/**
		* @~chinese
		* @brief 获取点云的顶点坐标
		* @~english
		* @brief Return the vertices of pointcloud
		**/
		std::vector<float3> & getVertices()
		{
			return _points;
		}

		/**
		* @~chinese
		* @brief 获取对应到RGB图像上的坐标(已归一化至0~1.f)，乘以RGB图像的分辨率即可得到绝对坐标值
		* @~english
		* Return the corresponding texture coordinates of pointcloud (0~1.f), multiply the image width/height to get the coordinates in image
		*/
		std::vector<float2> &getTexcoords()
		{
			return _textures;
		}
		/**
		* @~chinese
		* @brief 获取法线
		* @~english
		* @brief Return the normals of pointcloud.
		**/
		std::vector<float3> &getNormals()
		{
			return _normals;
		}

	private:
		float3 al_vector_cross(float3 a, float3 b)
		{
			float3 c;

			c.x = a.y*b.z - a.z*b.y;
			c.y = a.z*b.x - a.x*b.z;
			c.z = a.x*b.y - a.y*b.x;

			return c;
		}

		void calculateNormals(float3 *points, float3 *normals, int width, int height)
		{
			int w = width - 1;
			int h = height - 1;
#pragma omp parallel for
			for (int u = 0; u < w; ++u)
			{
				for (int v = 0; v < h; ++v)
				{
					float3 p0, p1, p2;
					float3 *n0, *n1, *n2;
					float3 normal, e1, e2;

					p0 = points[u + v * width];
					p1 = points[u + (v + 1) * width];
					p2 = points[u + 1 + (v + 1)* width];
					n0 = &normals[u + v * width];
					n1 = &normals[u + (v + 1) * width];
					n2 = &normals[u + 1 + (v + 1) * width];

					if (p0.z > 0.1f && p1.z > 0.1f && p2.z > 0.1f)
					{
						float depth_threshold = 5.f;

						float z0 = fabs(p2.z - p1.z);
						float z1 = fabs(p2.z - p0.z);
						float z2 = fabs(p0.z - p1.z);

						if (z0 > depth_threshold || z1 > depth_threshold || z2 > depth_threshold)
						{
							normal.x = 0.0;
							normal.y = 0.0;
							normal.z = 0.0;
						}
						else
						{
							e1.x = p1.x - p0.x;
							e1.y = p1.y - p0.y;
							e1.z = p1.z - p0.z;
							e2.x = p2.x - p0.x;
							e2.y = p2.y - p0.y;
							e2.z = p2.z - p0.z;
							normal = al_vector_cross(e1, e2);
						}
						n0->x += normal.x;
						n0->y += normal.y;
						n0->z += normal.z;
						n1->x += normal.x;
						n1->y += normal.y;
						n1->z += normal.z;
						n2->x += normal.x;
						n2->y += normal.y;
						n2->z += normal.z;
					}
				}
			}

#pragma omp parallel for
			for (int u = 0; u < w; u++)
			{
				for (int v = 0; v < h; v++)
				{
					float3 p0, p1, p2;
					float3 *n0, *n1, *n2;
					float3 normal, e1, e2;

					p0 = points[u + v * width];
					p1 = points[u + 1 + v * width];
					p2 = points[u + 1 + (v + 1)* width];
					n0 = &normals[u + v * width];
					n1 = &normals[u + 1 + v * width];
					n2 = &normals[u + 1 + (v + 1) * width];

					if (p0.z > 0.1 && p1.z > 0.1 && p2.z > 0.1)
					{
						float depth_threshold = 5.f;

						float z0 = fabs(p2.z - p1.z);
						float z1 = fabs(p2.z - p0.z);
						float z2 = fabs(p0.z - p1.z);

						if (z0 > depth_threshold || z1 > depth_threshold || z2 > depth_threshold)
						{
							normal.x += 0.0;
							normal.y += 0.0;
							normal.z += 0.0;
						}
						else
						{
							e1.x = p1.x - p0.x;
							e1.y = p1.y - p0.y;
							e1.z = p1.z - p0.z;
							e2.x = p2.x - p0.x;
							e2.y = p2.y - p0.y;
							e2.z = p2.z - p0.z;
							normal = al_vector_cross(e2, e1);
						}
						n0->x += normal.x;
						n0->y += normal.y;
						n0->z += normal.z;
						n1->x += normal.x;
						n1->y += normal.y;
						n1->z += normal.z;
						n2->x += normal.x;
						n2->y += normal.y;
						n2->z += normal.z;
					}
				}
			}

			int size = width * height;
			// normalize
#pragma omp parallel for 
			for (int ui = 0; ui < size; ui++)
			{
				float3 p = points[ui];
				if (p.z > 0.1) {
					normals[ui] = normalizeVector(normals[ui]);
				}
			}

		}

		float3 normalizeVector(const float3 &vector)
		{
			float l = sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
			if (l < 0.00001) {
				return vector;
			}
			else {
				float il = 1 / l;
				return float3(vector.x * il, vector.y * il, vector.z * il );
			}
		}

		virtual void insertPoints(float3 *points, float3 *normals, int width, int height, Extrinsics *extrinsics, Intrinsics *intrinsicsRgb, bool removeInvalid)
		{
			_validSize = 0;
			int index = 0;
			_textures.reserve(width * height);
			_points.reserve(width * height);
			_normals.reserve(width * height);
			for (int v = 0; v < height; ++v)
			{
				for (int u = 0; u < width; ++u, ++index)
				{
					float z = points[index].z;
					float x = points[index].x;
					float y = points[index].y;
					float3 pt(x,y,z);
					float3 tmp,tmp2;

					if (extrinsics && intrinsicsRgb)
					{
						tmp = pt;
						tmp.x += extrinsics->translation[0];
						tmp.y += extrinsics->translation[1];
						tmp.z += extrinsics->translation[2];
						tmp2.x = tmp.x * extrinsics->rotation[0] + tmp.y * extrinsics->rotation[1] + tmp.z * extrinsics->rotation[2];
						tmp2.y = tmp.x * extrinsics->rotation[3] + tmp.y * extrinsics->rotation[4] + tmp.z * extrinsics->rotation[5];
						tmp2.z = tmp.x * extrinsics->rotation[6] + tmp.y * extrinsics->rotation[7] + tmp.z * extrinsics->rotation[8];

						if (z > 0.f)
						{
							int fu = int(intrinsicsRgb->fx * tmp2.x / tmp2.z + intrinsicsRgb->cx);
							int fv = int(intrinsicsRgb->fy * tmp2.y / tmp2.z + intrinsicsRgb->cy);

							if (fu >= 0.00001 && fu < intrinsicsRgb->width && fv >= 0.00001 && fv < intrinsicsRgb->height)
							{
								_textures.emplace_back(float2(float(fu) / intrinsicsRgb->width, float(fv) / intrinsicsRgb->height));
								_points.emplace_back(pt);
								_normals.emplace_back(normals[u + v * width]);
								_validSize++;
							}
							else if (!removeInvalid)
							{
								_textures.emplace_back(float2(0.f, 0.f));
								_points.emplace_back(float3(0.f, 0.f, 0.f));
								_normals.emplace_back(float3(0.f, 0.f, 0.f));;
							}
						}
						else if (!removeInvalid)
						{
							_textures.emplace_back(float2(0.f, 0.f));
							_points.emplace_back(float3(0.f, 0.f, 0.f));
							_normals.emplace_back(float3(0.f, 0.f, 0.f));;
						}
						
					}
					else
					{
						if (pt.z > 0.f || !removeInvalid)
						{
							_points.emplace_back(pt);
							_normals.emplace_back(normals[index]);
							_textures.emplace_back(float2(float(u) / width, float(v) / height));
							if (pt.z > 0.f)
								_validSize++;
						}
					}
				}
			}

		}

	protected:
		std::vector<float3> _points;
		std::vector<float2> _textures;
		std::vector<float3> _normals;
		int _validSize;
	};

    static bool saveTexture(const char *data, int width, int height, int channel, const char *filename)
    {
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
                char *dst_b = bgrData.data();
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

    class PointcloudWithMesh : public Pointcloud
    {
    public:
        PointcloudWithMesh() {};
        ~PointcloudWithMesh() {};

        std::vector<int3> &getFaces()
        {
            return _faces;
        }

        void exportToFile(const std::string& filename, unsigned char *texture, int textureWidth, int textureHeight, bool binary)
        {
            const auto vertices = getVertices();
            const auto texcoords = getTexcoords();
            const auto normals = getNormals();
            const auto faces = getFaces();

            int index = filename.find_last_of('.');
            std::string ext = filename.substr(index+1);
            std::string fileName = filename.substr(0, index);
            std::string savePath = fileName + ".ply";
            std::string textureFilePath = fileName + ".bmp";
            std::string textureFileName = textureFilePath.substr(textureFilePath.find_last_of('/') + 1);
            if (ext == "mesh")
            {
                std::ofstream out(savePath);
                out << "ply\n";
                if (binary)
                    out << "format binary_little_endian 1.0\n";
                else
                    out << "format ascii 1.0\n";
                if (texture)
                {
                    out << "comment TextureFile " << textureFileName << "\n";
                }
                out << "comment pointcloud saved from 3DCamera\n";
                out << "element vertex " << size() << "\n";
                out << "property float x\n";
                out << "property float y\n";
                out << "property float z\n";
                out << "property float nx\n";
                out << "property float ny\n";
                out << "property float nz\n";
                out << "element face " << faces.size() << "\n";
                out << "property list uchar uint vertex_indices\n";
                if (texture)
                {
                   out << "property list uchar float texcoord\n";
                }
                out << "end_header\n";
                out.close();

                if(texture)
                {
                    saveTexture((const char *)texture, textureWidth, textureHeight, 3, textureFilePath.c_str());
                }
                if (binary)
                {
                    out.open(savePath, std::ios_base::app | std::ios_base::binary);
                    for (int i = 0; i < size(); ++i)
                    {
                        // we assume little endian architecture on your device
                        out.write(reinterpret_cast<const char*>(&(vertices[i].x)), sizeof(float));
                        out.write(reinterpret_cast<const char*>(&(vertices[i].y)), sizeof(float));
                        out.write(reinterpret_cast<const char*>(&(vertices[i].z)), sizeof(float));
                        out.write(reinterpret_cast<const char*>(&(normals[i].x)), sizeof(float));
                        out.write(reinterpret_cast<const char*>(&(normals[i].y)), sizeof(float));
                        out.write(reinterpret_cast<const char*>(&(normals[i].z)), sizeof(float));
                    }

                    unsigned char faces_point_size = 3;
                    unsigned char texture_coord_size = 6;

                    for (int i = 0; i < faces.size(); ++i)
                    {
                        // we assume little endian architecture on your device
                        float v;
                        out.write(reinterpret_cast<const char*>(&(faces_point_size)), sizeof(unsigned char));
                        out.write(reinterpret_cast<const char*>(&(faces[i].x)), sizeof(int));
                        out.write(reinterpret_cast<const char*>(&(faces[i].y)), sizeof(int));
                        out.write(reinterpret_cast<const char*>(&(faces[i].z)), sizeof(int));
                        if (texture)
                        {
                            out.write(reinterpret_cast<const char*>(&(texture_coord_size)), sizeof(unsigned char));
                            out.write(reinterpret_cast<const char*>(&(texcoords[faces[i].x].u)), sizeof(float));
                            v = (1.f - texcoords[faces[i].x].v);
                            out.write(reinterpret_cast<const char*>(&v), sizeof(float));
                            out.write(reinterpret_cast<const char*>(&(texcoords[faces[i].y].u)), sizeof(float));
                            v = (1.f - texcoords[faces[i].y].v);
                            out.write(reinterpret_cast<const char*>(&v), sizeof(float));
                            out.write(reinterpret_cast<const char*>(&(texcoords[faces[i].z].u)), sizeof(float));
                            v = (1.f - texcoords[faces[i].z].v);
                            out.write(reinterpret_cast<const char*>(&v), sizeof(float));
                        }
                    }
                    out.close();
                }
                else
                {
                    out.open(savePath, std::ios_base::app);
                    for (int i = 0; i < size(); ++i)
                    {
                        // we assume little endian architecture on your device
                        out << (vertices[i].x) << " ";
                        out << (vertices[i].y) << " ";
                        out << (vertices[i].z) << " ";
                        out << (normals[i].x) << " ";
                        out << (normals[i].y) << " ";
                        out << (normals[i].z) << " ";
                        out << "\n";
                    }

                    for (int i = 0; i < faces.size(); ++i)
                    {
                        // we assume little endian architecture on your device
                        out << "3 ";
                        out << faces[i].x << " ";
                        out << faces[i].y << " ";
                        out << faces[i].z << " ";
                        if (texture)
                        {
                            out << "6 ";
                            out << texcoords[faces[i].x].u << " ";
                            out << (1.f - texcoords[faces[i].x].v) << " ";
                            out << texcoords[faces[i].y].u << " ";
                            out << (1.f - texcoords[faces[i].y].v) << " ";
                            out << texcoords[faces[i].z].u << " ";
                            out << (1.f - texcoords[faces[i].z].v) << " ";
                        }
                        out << "\n";
                    }

                    out.close();
                }
            }
        }

        void insertPoints(float3 *points, float3 *normals, int width, int height, Extrinsics *extrinsics, Intrinsics *intrinsicsRgb, bool removeInvalid)
        {
            _validSize = 0;
            int index = 0;
            _textures.reserve(width * height);
            _points.reserve(width * height);
            _normals.reserve(width * height);
            _faces.reserve(width * height * 2);

            typedef std::unordered_map<unsigned int, unsigned int> IndexMap;
            IndexMap indexMap;

            for (int v = 0; v < height; ++v)
            {
                for (int u = 0; u < width; ++u, ++index)
                {
                    float z = points[index].z;
                    float x = points[index].x;
                    float y = points[index].y;
                    float3 pt(x, y, z);
                    float3 tmp, tmp2;

                    if (extrinsics && intrinsicsRgb)
                    {
                        tmp = pt;
                        tmp.x += extrinsics->translation[0];
                        tmp.y += extrinsics->translation[1];
                        tmp.z += extrinsics->translation[2];
                        tmp2.x = tmp.x * extrinsics->rotation[0] + tmp.y * extrinsics->rotation[1] + tmp.z * extrinsics->rotation[2];
                        tmp2.y = tmp.x * extrinsics->rotation[3] + tmp.y * extrinsics->rotation[4] + tmp.z * extrinsics->rotation[5];
                        tmp2.z = tmp.x * extrinsics->rotation[6] + tmp.y * extrinsics->rotation[7] + tmp.z * extrinsics->rotation[8];

                        if (z > 0.f)
                        {
                            int fu = int(intrinsicsRgb->fx * tmp2.x / tmp2.z + intrinsicsRgb->cx);
                            int fv = int(intrinsicsRgb->fy * tmp2.y / tmp2.z + intrinsicsRgb->cy);

                            if (fu >= 0.00001 && fu < intrinsicsRgb->width && fv >= 0.00001 && fv < intrinsicsRgb->height)
                            {
                                indexMap[v * width + u] = _points.size();
                                _textures.emplace_back(float2(float(fu) / intrinsicsRgb->width, float(fv) / intrinsicsRgb->height));
                                _points.emplace_back(pt);
                                _normals.emplace_back(normals[u + v * width]);
                                _validSize++;
                            }
                            else if (!removeInvalid)
                            {
                                indexMap[v * width + u] = _points.size();
                                _textures.emplace_back(float2(0.f, 0.f));
                                _points.emplace_back(float3(0.f, 0.f, 0.f));
                                _normals.emplace_back(float3(0.f, 0.f, 0.f));;
                            }
                        }
                        else if (!removeInvalid)
                        {
                            indexMap[v * width + u] = _points.size();
                            _textures.emplace_back(float2(0.f, 0.f));
                            _points.emplace_back(float3(0.f, 0.f, 0.f));
                            _normals.emplace_back(float3(0.f, 0.f, 0.f));;
                        }

                    }
                    else
                    {
                        if (pt.z > 0.f || !removeInvalid)
                        {
                            indexMap[v * width + u] = _points.size();
                            _points.emplace_back(pt);
                            _normals.emplace_back(normals[index]);
                            _textures.emplace_back(float2(float(u) / width, float(v) / height));
                            if (pt.z > 0.f)
                                _validSize++;
                        }
                    }
                }
            }

            float depth_threshold = 10.0f;

            auto addFaces = [&](unsigned int i0, unsigned int i1, unsigned int i2)
            {
                float3 p0 = _points[i0];
                float3 p1 = _points[i1];
                float3 p2 = _points[i2];

                float z0 = fabs(p2.z - p1.z);
                float z1 = fabs(p2.z - p0.z);
                float z2 = fabs(p0.z - p1.z);

                if (z0 > depth_threshold || z1 > depth_threshold || z2 > depth_threshold)
                    return;

                int3 face;
                face.x = i0;
                face.y = i1;
                face.z = i2;
                _faces.emplace_back(face);
            };

            for (int i = 0; i < height - 1; i++)
            {
                int curr_line_start = i * width;
                int next_line_start = (i + 1) * width;

                for (int j = 0; j < width - 1; ++j)
                {
                    //left top
                    IndexMap::iterator lt = indexMap.find(curr_line_start + j);
                    //left bottom
                    IndexMap::iterator lb = indexMap.find(next_line_start + j);
                    //right bottom
                    IndexMap::iterator rb = indexMap.find(next_line_start + j + 1);
                    //right top
                    IndexMap::iterator rt = indexMap.find(curr_line_start + j + 1);

                    if (lb != indexMap.end() && rb != indexMap.end() && rt != indexMap.end())
                    {
                        addFaces(lb->second, rb->second, rt->second);
                    }

                    if (lb != indexMap.end() && rt != indexMap.end() && lt != indexMap.end())
                    {
                        addFaces(lb->second, rt->second, lt->second);
                    }
                }
            }
        }
    private:
        std::vector<int3> _faces;
    };

	/**
	* @~chinese
	* @brief 将深度图转换为RGB图
	* @~english
	* @brief Convert depth map to RGB image
	**/
	class colorizer
	{
	public:
		colorizer() :_zmin(0), _zmax(5000)
		{
			initColormap(4000);
		}
		/**
		* @~chinese
		* @brief      设置渲染色带对应的最大深度值和最小深度值
		* @param[in]  zmin				：最小深度值
		* @param[in]  zmax				：最大深度值
		* @~english
		* @brief      set maximum depth and minimum depth corresponding to color map
		* @param[in]  zmin				：the minimum depth
		* @param[in]  zmax				：the maximum depth
		**/
		void setRange(int zmin, int zmax)
		{
			_zmin = zmin;
			_zmax = zmax;
		}

		/**
		* @~chinese
		* @brief      将深度图转换为RGB图
		* @param[in]  depthData			：深度数据
		* @param[in]  scale				：深度缩放系数
		* @param[in]  rgbData			：RGB数据保存地址
		* @param[in]  dataSize			：元素总数
		* @~english
		* @brief      set maximum depth and minimum depth corresponding to color map
		* @param[in]  depthData			：the pointer of depth data
		* @param[in]  scale				：the scale of depth
		* @param[in]  rgbData			：the pointer of RGB data
		* @param[in]  dataSize			：element size
		**/
		template < typename T >
		void process(T* depthData, float scale, unsigned char *rgbData, int dataSize)
		{
			T max = _zmax;
			T min = _zmin;
			T range = max - min;
			for (auto i = 0; i < dataSize; ++i)
			{
				auto d = depthData[i] * scale;
				if (d>0)
				{
					auto f = range != 0 ? (d - min)*1.0 / range : 0;
					auto c = getColor(f);
					rgbData[i * 3 + 0] = (uint8_t)c.x;
					rgbData[i * 3 + 1] = (uint8_t)c.y;
					rgbData[i * 3 + 2] = (uint8_t)c.z;
				}
				else
				{
					rgbData[i * 3 + 0] = 0;
					rgbData[i * 3 + 1] = 0;
					rgbData[i * 3 + 2] = 0;
				}
			}
		}
	private:

		inline float3 getColor(float value) const
		{
			if (value < 0) value = 0;
			if (value > 1) value = 1;
			return _cacheData[(int)(value * (_cacheSize - 1))];
		}
		inline float3 lerp(const float3& a, const float3& b, float t) const
		{
			float3 v = b * t + a * (1 - t);
			if (v.x < 0) v.x = 0;
			if (v.x > 255) v.x = 255;
			if (v.y < 0) v.y = 0;
			if (v.y > 255) v.y = 255;
			if (v.z < 0) v.z = 0;
			if (v.z > 255) v.z = 255;
			return v;
		}

		float3 calc(float value) const
		{
			// if we have exactly this value in the map, just return it
			if (_map.find(value) != _map.end()) return _map.at(value);

			auto lower = _map.lower_bound(value) == _map.begin() ? _map.begin() : --(_map.lower_bound(value));
			auto upper = _map.upper_bound(value);

			auto t = (value - lower->first) / (upper->first - lower->first);
			auto c1 = lower->second;
			auto c2 = upper->second;
			return lerp(c1, c2, t);
		}

		void initColormap(int steps)
		{
			_map[0] = float3(0, 0, 255);
			_map[0.25f] = float3(0, 255, 255);
			_map[0.50f] = float3(255, 255, 0);
			_map[0.75f] = float3(255, 0, 0);
			_map[1.00f] = float3(50, 0, 0);

			_cache.resize(steps + 1);
			for (int i = 0; i <= steps; i++) {
				auto t = (float)i / steps;
				_cache[i] = calc(t);
			}

			_cacheSize = _cache.size();
			_cacheData = _cache.data();
		}

	private:
		int _zmin;
		int _zmax;
		std::map<float, float3> _map;
		std::vector<float3> _cache;
		size_t _cacheSize; float3* _cacheData;
	};

	namespace filter
	{
	/**
	* @~chinese
	* @brief 枚举: 滤波方式
	* @~english
	* @brief enumeration: type of filter
	**/
	typedef enum
	{
		FILTER_TYPE_AVERAGE = 0x00,
		FILTER_TYPE_MEDIAN = 0x01,
		FILTER_TYPE_GAUSSIAN = 0x02,
	}FilterType;

	static void generateGaussianKernel(float *gaussianKernel, int filterSize, int sigma)
	{
		int m = filterSize / 2;
		int n = filterSize / 2;

		float s = 2.0f * sigma*sigma;
		float sum = 0;

		for (int i = 0; i < filterSize; i++)
		{
			for (int j = 0; j < filterSize; j++)
			{
				int x = i - m;
				int y = j - n;

				gaussianKernel[j*filterSize + i] = float(exp(-(x*x + y*y) / s) / (M_PI*s));
			}
		}

		for (int i = 0; i < filterSize * filterSize; i++)
		{
			sum += gaussianKernel[i];
		}

		for (int i = 0; i < filterSize * filterSize; i++)
		{
			gaussianKernel[i] = gaussianKernel[i] / sum;
		}
	}

	static void generateAverageKernel(float *averageKernel, int filterSize)
	{
		memset(averageKernel, 0, sizeof(float) * filterSize * filterSize);
		for (int i = 0; i < filterSize * filterSize; i++)
		{
			averageKernel[i] = 1.0f / (filterSize * filterSize);
		}
	}
	/**
	* @~chinese
	* @brief      高斯滤波
	* @param[in]  depthData			：深度数据
	* @param[in]  width				：图像宽度
	* @param[in]  height			：图像高度
	* @param[in]  filterSize		：模板尺寸，必须为奇数，数值越大滤波程度越大
	* @param[in]  sigma				：系数
	* @return     成功:SUCCESS, 失败:其他错误码
	* @~english
	* @brief      Execute gaussian filter
	* @param[in]  depthData			：pointer of data
	* @param[in]  width				：the width of image
	* @param[in]  height			：the height of image
	* @param[in]  filterSize		：the size of filter template，it must be odd 
	* @param[in]  sigma				：sigma
	* @return success:return 0, fail:other error code
	**/
	template <typename T>
	static int GaussianBlur(T *depthData, int width, int height, int filterSize, int sigma)
	{
		T *filterData = NULL;
		float *kernelData = NULL;
		T *calcData = NULL;

		filterData = (T *)malloc(sizeof(T) * width * height);
		memset(filterData, 0, sizeof(T) * width * height);

		calcData = (T*)malloc(sizeof(T) * filterSize * filterSize);
		memset(calcData, 0, sizeof(T) * filterSize * filterSize);

		kernelData = (float *)malloc(sizeof(float) * filterSize * filterSize);
		memset(kernelData, 0, sizeof(T) * filterSize * filterSize);

		generateGaussianKernel(kernelData, filterSize, sigma);

		for (int h = filterSize / 2; h < height - filterSize / 2; h++)
		{
			for (int w = filterSize / 2; w < width - filterSize / 2; w++)
			{
				bool flag = true;
				for (int i = 0; i < filterSize; i++)
				{
					if (!flag)
						break;

					for (int j = 0; j < filterSize; j++)
					{
						if (depthData[(h + i - filterSize / 2) * width + w + j - filterSize / 2] < 1)
						{
							flag = false;
							break;
						}
						else
						{
							calcData[i* filterSize + j] = depthData[(h + i - filterSize / 2) * width + w + j - filterSize / 2];
						}
					}
				}

				if (flag)
				{
					float tmp_data = 0;
					for (int k = 0; k < filterSize * filterSize; k++)
					{
						tmp_data += (calcData[k] * kernelData[k]);
					}
					filterData[h * width + w] = tmp_data;
				}
			}
		}

		int count = 0;

		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				if (filterData[h*width + w] > 1)
				{
					depthData[h*width + w] = filterData[h*width + w];
					count++;
				}
			}
		}
		free(filterData);
		filterData = NULL;

		free(calcData);
		calcData = NULL;

		if (kernelData)
		{
			free(kernelData);
			kernelData = NULL;
		}

		return 0;
	}
	/**
	* @~chinese
	* @brief      均值滤波
	* @param[in]  depthData			：深度数据
	* @param[in]  width				：图像宽度
	* @param[in]  height			：图像高度
	* @param[in]  filterSize		：模板尺寸，必须为奇数，数值越大滤波程度越大
	* @return     成功:SUCCESS, 失败:其他错误码
	* @~english
	* @brief      Execute average filter
	* @param[in]  depthData			：pointer of data
	* @param[in]  width				：the width of image
	* @param[in]  height			：the height of image
	* @param[in]  filterSize		：the size of filter template，it must be odd 
	* @return success:return 0, fail:other error code
	**/
	template <typename T>
	static int AverageBlur(T *depthData, int width, int height, int filterSize)
	{
		T *filterData = NULL;
		float *kernelData = NULL;
		T *calcData = NULL;

		filterData = (T *)malloc(sizeof(T) * width * height);
		memset(filterData, 0, sizeof(T) * width * height);

		calcData = (T*)malloc(sizeof(T) * filterSize * filterSize);
		memset(calcData, 0, sizeof(T) * filterSize * filterSize);

		kernelData = (float *)malloc(sizeof(float) * filterSize * filterSize);
		memset(kernelData, 0, sizeof(T) * filterSize * filterSize);

		generateAverageKernel(kernelData, filterSize);

		for (int h = filterSize / 2; h < height - filterSize / 2; h++)
		{
			for (int w = filterSize / 2; w < width - filterSize / 2; w++)
			{
				bool flag = true;
				for (int i = 0; i < filterSize; i++)
				{
					if (!flag)
						break;

					for (int j = 0; j < filterSize; j++)
					{
						if (depthData[(h + i - filterSize / 2) * width + w + j - filterSize / 2] < 1)
						{
							flag = false;
							break;
						}
						else
						{
							calcData[i* filterSize + j] = depthData[(h + i - filterSize / 2) * width + w + j - filterSize / 2];
						}
					}
				}

				if (flag)
				{
					float tmp_data = 0;
					for (int k = 0; k < filterSize * filterSize; k++)
					{
						tmp_data += (calcData[k] * kernelData[k]);
					}
					filterData[h * width + w] = tmp_data;
				}
			}
		}

		int count = 0;

		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				if (filterData[h*width + w] > 1)
				{
					depthData[h*width + w] = filterData[h*width + w];
					count++;
				}
			}
		}
		free(filterData);
		filterData = NULL;

		free(calcData);
		calcData = NULL;

		if (kernelData)
		{
			free(kernelData);
			kernelData = NULL;
		}
		return 0;
	}

	/**
	* @~chinese
	* @brief      中值滤波
	* @param[in]  depthData			：深度数据
	* @param[in]  width				：图像宽度
	* @param[in]  height			：图像高度
	* @param[in]  filterSize		：模板尺寸，必须为奇数，数值越大滤波程度越大
	* @return     成功:SUCCESS, 失败:其他错误码
	* @~english
	* @brief      Execute median filter
	* @param[in]  depthData			：pointer of data
	* @param[in]  width				：the width of image
	* @param[in]  height			：the height of image
	* @param[in]  filterSize		：the size of filter template，it must be odd 
	* @return success:return 0, fail:other error code
	**/
	template <typename T>
	static int MedianBlur(T *depthData, int width, int height, int filterSize)
	{
		T *filterData = NULL;
		T *calcData = NULL;

		filterData = (T *)malloc(sizeof(T) * width * height);
		memset(filterData, 0, sizeof(T) * width * height);

		calcData = (T*)malloc(sizeof(T) * filterSize * filterSize);
		memset(calcData, 0, sizeof(T) * filterSize * filterSize);

		for (int h = filterSize / 2; h < height - filterSize / 2; h++)
		{
			for (int w = filterSize / 2; w < width - filterSize / 2; w++)
			{
				bool flag = true;
				for (int i = 0; i < filterSize; i++)
				{
					if (!flag)
						break;

					for (int j = 0; j < filterSize; j++)
					{
						if (depthData[(h + i - filterSize / 2) * width + w + j - filterSize / 2] < 1)
						{
							flag = false;
							break;
						}
						else
						{
							calcData[i* filterSize + j] = depthData[(h + i - filterSize / 2) * width + w + j - filterSize / 2];
						}
					}
				}

				if (flag)
				{
					T tmp = 0;

					for (int m = 0; m < filterSize * filterSize; m++)
					{
						for (int n = 0; n < m; n++)
						{
							if (calcData[n] < calcData[m])
							{
								tmp = calcData[n];
								calcData[n] = calcData[m];
								calcData[m] = tmp;
							}
						}
					}
					filterData[h * width + w] = calcData[filterSize * filterSize / 2];
				}
			}
		}

		int count = 0;

		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				if (filterData[h*width + w] > 1)
				{
					depthData[h*width + w] = filterData[h*width + w];
					count++;
				}
			}
		}
		free(filterData);
		filterData = NULL;

		free(calcData);
		calcData = NULL;

		return 0;
	}
	}
/*@} */
}

#endif
