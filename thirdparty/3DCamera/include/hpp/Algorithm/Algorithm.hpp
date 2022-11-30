/*****************************************************************************
*  3DCamera SDK header
*
*
*  @file     Algorithm.hpp
*  @brief    3DCamera sdk header
*
*  @version  1.0
*  @date     2021 / 02 / 23
*
*****************************************************************************/
#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

#include <vector>
#include "hpp/APIExport.hpp"
#include "hpp/Processing.hpp"

namespace cs
{

/**
* @~chinese
* @brief 计算点云中线与线/面与面夹角
* @param[in] pointCloud			点云数据
* @param[out] angle				线与线/面与面的夹角(单位:弧度)
* @return     成功:SUCCESS, 失败:其他错误码
* @~english
* @brief Calculate the angle between the line and the line/surface and the surface of the point cloud
* @param[in] pointCloud			The point clound data
* @param[out] angle				The angle between line and line/surface and surface(Unit:Rad)
* @return success:return SUCCESS, fail:other error code
*/	
CS_API ERROR_CODE calulatePointCloundAngle(Pointcloud &pointCloud, float &angle);
	
}
#endif