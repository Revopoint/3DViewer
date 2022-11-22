#ifndef UPGRADE_H_
#define UPGRADE_H_
/* Include the libailook C header files */
#include "hpp/Types.hpp"



#ifdef __cplusplus
extern "C" {
#endif

#ifndef HANDLE_DEF
#define HANDLE_DEF
	typedef void* HANDLE;
#endif

/**
* @~chinese
* @brief      启动升级(异步接口)
* @param[in]  cInfo				指定需要升级的设备信息，如果不指定则将变量置空
* @param[in]  upExeDir			指定升级的可执行文件目录，在此目录需要能够找到up、up2、adb三个可执行文件
* @param[in]  upgradePackage		升级包的本地路径
* @param[in]  upgradePackageMd5			：升级包的md5值，用于验证包文件的完整性
* @return     成功:升级句柄, 失败:nullptr
* @~english
* @brief     upgrade device firmware,cannot terminate after startup
* @param[in]  cInfo				specify the device information to be upgraded.if not specified,set the variable to null
* @param[in]  upExeDir			specify the directory of executable file to be upgraded.in this directory,can find the up\up2\adb executable files.
* @param[in]  upgradePackage		: local path of upgrade firmware package
* @param[in]  upgradePackageMd5			：the md5 of upgradePackage,to check the package is ok.
* @return success:return SUCCESS, fail:other error code
**/
CS_API HANDLE upgradeCamera(CameraInfo cInfo,const char* upExeDir,const char* upgradePackage, const char* upgradePackageMd5);

/**
 * @~chinese
 * @brief      获取当前升级进度(异步接口)
 * @hUpgrade	升级句柄，通过upgradeCamera创建
 * @return     成功:[0,100],表示进度的百分比, 失败:0xff
 * @~english
 * @brief	get the progress of upgrade firmware
 * @hUpgrade	the upgrade handle,create by upgradeCamera.
 * @return  success:the value of [0,100],represents the percentage of progress, 
 *			fail:0xff
 **/
CS_API int getUpgradeCameraProgress(HANDLE hUpgrade);


 /**
 * @~chinese
 * @brief      完成升级，释放升级句柄
 * @hUpgrade	升级句柄，通过upgradeCamera创建
 * @return     
 * @~english
 * @brief		finish the upgrading,release the handle 
 * @hUpgrade	the upgrade handle,create by upgradeCamera.
 * @return  
 **/
CS_API void releaseUpgradeCamera(HANDLE hUpgrade);

/**
*		进入升级模式(recovery mode)
*@pcSerial		需要升级的序列号(当前接口内部没有使用)
*/
CS_API bool enterRecoveryMode(const char* pcSerial,const char* upExeDir="./");

/**
*		leave recovery mode(enter normal mode)
*@cInfo	camera info.
*/
CS_API bool leaveRecoveryMode(CameraInfo cInfo,const char* upExeDir="./");


#ifdef __cplusplus
}
#endif


#endif
