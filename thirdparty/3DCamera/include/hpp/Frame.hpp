 /*****************************************************************************
*  3DCamera SDK header
*
*
*  @file     Frame.hpp
*  @brief    3DCamera sdk header
*
*  @version  1.0
*  @date     2019 / 08 / 17
*
*****************************************************************************/
#ifndef __FRAME_HPP__
#define __FRAME_HPP__

#include <vector>
#include <memory>
#include "Types.hpp"
#include "hpp/APIExport.hpp"

namespace cs
{
class IFrame;
/**
* @~chinese
* \defgroup Frame 数据帧操作 
* @brief 数据帧对象接口
* @{
* @~english
* \defgroup Frame Frame Operations
* @brief Frame object interface
* @{
*/
typedef std::shared_ptr<IFrame> IFramePtr;

/*!\class IFrame
* @~chinese
* @brief 数据帧接口
* @~english
* @brief Frame interface
*/
class CS_API IFrame
{
public:

	virtual ~IFrame() {};

	/** 
	* @~chinese		
	* @brief		判断帧是否为空
	* @ return 如果帧为空返回true, 否则返回false
	* @~english
	* @brief		Check frame is valid
	* @return true if frame is empty，otherwise return false
	*/
	virtual const bool empty() const = 0;

	/**
	* @~chinese
	* @brief      获取该帧数据的时间戳，单位为毫秒
	* @~english
	* @brief      Retrieve the time at which the frame was captured in milliseconds
	**/
	virtual const double getTimeStamp() const = 0;

	/**
	* @~chinese
	* @brief      获取该帧数据的首地址指针
	* @~english
	* @brief      Retrieve the pointer to the start of the frame data
	**/
	virtual const char *getData() const = 0;

	/**
	* @~chinese
	* @brief      获取该帧数据指定部分的起始指针
	* @~english
	* @brief      Retrieve the pointer to the start of the specified format data
	**/
	virtual const char *getData(FRAME_DATA_FORMAT format) const = 0;
	
	/**
	* @~chinese
	* @brief      获取该帧数据的字节数
	* @~english
	* @brief      Retrieve size of frame in bytes
	**/
	virtual const int getSize() const = 0;

	/**
	* @~chinese
	* @brief      获取该帧宽度
	* @~english
	* @brief      Retrieve frame width in pixels
	**/
	virtual const int getWidth() const = 0;

	/**
	* @~chinese
	* @brief      获取该帧高度
	* @~english
	* @brief      Retrieve frame height in pixels
	**/
	virtual const int getHeight() const = 0;

	/**
	* @~chinese
	* @brief      获取帧的格式
	* @~english
	* @brief      Retrieve format of frame
	**/
	virtual const STREAM_FORMAT getFormat() const = 0;

    /**
    * @~chinese
    * @brief      获取帧头原始数据
    * @~english
    * @brief      Retrieve header of frame
    **/
    virtual const ExtraInfo* getExtraInfo() const = 0;

};
/*@} */
}
#endif