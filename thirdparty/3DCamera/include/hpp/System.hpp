 /*****************************************************************************
*  3DCamera SDK header
*
*
*  @file     System.hpp
*  @brief    3DCamera sdk header
*
*  @version  1.0
*  @date     2019 / 08 / 17
*
*****************************************************************************/
#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <vector>
#include <memory>
#include "Types.hpp"
#include "hpp/APIExport.hpp"
#include "hpp/Camera.hpp"

namespace cs
{
class ISystem;

/**
* @~chinese
* \defgroup System 相机接入监听
* @brief 发现相机，监听相机接入列表变动
* @{
* @~english
* \defgroup System Camera monitor
* @brief Discover camera, monitor camera change
* @{
*/

/** @brief camera state change callback */
typedef void (*CameraChangeCallback)(std::vector<CameraInfo>& addedCameras, std::vector<CameraInfo>& removedCameras, void * userData);

/**
* @~chinese
* \defgroup System 相机接入监听
* @brief 上报相机警告信息通道
* @param[in]	jsonData	上报数据，json数据格式
* @param[in]	iDataLen	json数据长度
* @param[in]	userData	回调用户数据，在设置回调时传入
* @{
* @~english
* \defgroup System Camera monitor
* @brief reporting channel of camera alarm informattion
* @param[in]	jsonData	report data,orgainze the data in json form
* @param[in]	iDataLen	length of report data content
* @param[in]	userData	context parameter of callback,which is the context passed in when setting
* @{
*/

/** @brief camera state change callback */
typedef void(*CameraAlarmCallback)(const char* jsonData,int iDataLen, void * userData);

/**
* @~chinese
* @brief     ISystem类对象指针类型定义
* @~english
* @brief     ISytem object point type define
**/
typedef std::shared_ptr<ISystem> ISystemPtr;

/*!\class ISystem
* @~chinese
* @brief System接口抽象类
* @~english 
* @brief System interface
**/
class CS_API ISystem
{
public:

	virtual ~ISystem() {};

	/**
	* @~chinese
	* @brief      查询当前已接入相机设备
	* @param[out] cameras 已接入机机设备信息列表
	* @param[in]  timeout	超时时间
	* @return     成功:SUCCESS, 失败:其它错误码
	* @~english
	* @brief      Query valid 3d cameras
	* @param[out] cameras		    :return valid 3d cameras
	* @param[in]	timeout			:timeout in ms
	* @return success:return SUCCESS, fail:other error code
	**/
    virtual ERROR_CODE queryCameras(std::vector<CameraInfo> &cameras,int timeout=0) = 0;

	/**
	* @~chinese
	* @brief      设置相机接入状态变动回调
	* @param[in]  callback		变动回调函数
	* @param[in]  userData		回调函数用户数据
	* @return     成功:SUCCESS, 失败:其它错误码
	* @~english
	* @brief      Set camera state change callback
	* @param[in]  callback		camera state change callback
	* @param[in]  userData		pointer of user data
	* @return success:return SUCCESS, fail:other error code
	**/ 
    virtual ERROR_CODE setCameraChangeCallback(CameraChangeCallback callback, void *userData) = 0;

	/**
	* @~chinese
	* @brief      设置相机警告回调函数
	* @param[in]  callback		警告回调函数
	* @param[in]  userData		回调函数用户数据
	* @return     成功:SUCCESS, 失败:其它错误码
	* @~english
	* @brief      Set camera alarm reporting channel
	* @param[in]  callback		report alarm callback
	* @param[in]  userData		pointer of user data
	* @return success:return SUCCESS, fail:other error code
	**/
	virtual ERROR_CODE setCameraAlarmCallback(CameraAlarmCallback callback, void *userData) = 0;

	/**
	* @~chinese
	* @brief      增加一个usb连接的相机文件描述符,安卓对接会使用
	* @param[in]  iFd		文件描述符
	* @return     成功:SUCCESS, 失败:其它错误码
	* @~english
	* @brief     add a usb camera fd,used in android.
	* @param[in]  iFd		usb fd
	* @return success:return SUCCESS, fail:other error code
	**/
	virtual ERROR_CODE addUsbCameraFd(int iFd) = 0;

	/**
	* @~chinese
	* @brief      删除一个usb连接的相机文件描述符,安卓对接会使用
	* @param[in]  iFd		文件描述符
	* @return     成功:SUCCESS, 失败:其它错误码
	* @~english
	* @brief     remove a usb camera fd,used in android.
	* @param[in]  iFd		usb fd
	* @return success:return SUCCESS, fail:other error code
	**/
	virtual ERROR_CODE removeUsbCameraFd(int iFd) = 0;

	/**
	* @~chinese
	* @brief      网络方式探测控制,安卓对接会使用
	* @param[in]  emPT		探测状态
	* @return     成功:SUCCESS, 失败:其它错误码
	* @~english
	* @brief      stop net probe,used in android.
	* @param[in]  emPT       probe state
	* @return success:return SUCCESS, fail:other error code
	**/
	virtual ERROR_CODE netProbeCtrl(ProbeType emPT) = 0;

	/**
	* @~chinese
	* @brief      初始化驱动
	* @code		初始化序列号
	* @return     成功:SUCCESS, 失败:其他错误码
	* @~english
	* @brief		initializate the driver
	* @code	the initializate code of driver
	* @return  success:return SUCCESS, fail:other error code
	*/
	virtual ERROR_CODE initDriver(const char* code) = 0;

	/**
	* @~chinese
	* @brief      获取相机的智能指针
	* @return     相机的智能指针
	* @~english
	* @brief		get the shared pointer of camera.
	* @return  shared pointer of camera.
	*/
	virtual ICameraPtr getCameraPtr() = 0;
};

/**
* @~chinese
* @brief     获取System对象智能指针
* @~english
* @brief     Get System object smart point
**/
CS_API std::shared_ptr<ISystem> getSystemPtr();

/**
* @~chinese
* @brief     获取相机设备接入状态
* @~english
* @brief     judge whether the camera is connected.
**/
CS_API CAMERA_STATUS getCameraStatus(const char* pcSerial);

/**
* @~chinese
* @brief     获取SDK版本
* @~english
* @brief     Get SDK verison
**/
CS_API ERROR_CODE getSdkVersion(CS_SDK_VERSION** ppCsSdkVersion);

/**
* @~chinese
* @brief     设置SDK日志报务器信息
* @~english
* @brief     Set SDK server infomation
**/
CS_API void setSdkLogServerInfo(const char* logServerIp, unsigned short wLogServerPort);

/**
* @~chinese
* @brief     设置SDK库UVC是否可用
* @~english
* @brief     Set SDK libuvc enable
**/
CS_API void setSdkEnableLibuvc(bool isEnableLibuvc);

/**
* @~chinese
* @brief     设置SDK库网络是否可用
* @~english
* @brief     Set SDK networking enable
**/
CS_API void setSdkEnableNetworking(bool isEnableNetWorking);

/**
* @~chinese
* @brief     设置日志文件保存路径
* @~english
* @brief     Set log file save path
**/
CS_API void setLogSavePath(const char* savePath);

/**
* @~chinese
* @brief     获取日志文件保存路径
* @~english
* @brief     Get log file save path
**/
CS_API const char* getLogSavePath();

/**
* @~chinese
* @brief     日志开关
* @~english
* @brief     Log enable
**/
CS_API void enableLoging(bool enableLoging);

/**
* @~chinese
* @brief     通过SN获取相机类型
* @~english
* @brief     Get camera type by SN
**/
CS_API CameraType getCameraTypeBySN(const char* sn);

/**
* @~chinese
* @brief     通过相机类型获取名称
* @~english
* @brief     get camera type name by cameraType
**/
CS_API const char* getCameraTypeName(CameraType tCt);

/**
* @~chinese
* @brief     网络功能开关
* @~english
* @brief     网络功能开关
**/
CS_API void setEnableNetworking(bool bEnable);
CS_API bool getEnableNetworking();

/**
*		get error string
*/
CS_API const char* getCameraErrorString(ERROR_CODE code);


/**
*   set check inited enable or not
*/
CS_API void setCheckedCameraInited(bool enable);

/*
*   set check inited state
*/
CS_API bool getCheckedCameraInited();

}

#endif
