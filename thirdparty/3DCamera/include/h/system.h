#ifndef LIB_3DCAMERA_SYSTEM_H
#define LIB_3DCAMERA_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../hpp/Types.hpp"
#include "types.h"

/**
*		camera change callback,when camera online or offline will call this function to notify.
*@param[in]		removed	:removed camera list.This pointer can be used temporarily,but cannot be saved for use elsewhere.not need to call systemDeleteCameraInfoList.
*@param[in]		added	:added camera list. the usage is the same as removed param.
*@param[in]		user	:user data,incoming through systemSetCameraChangeCallback.
*@return		none.
*/
typedef void(*CCameraChangeCallback)(CameraInfo* added,int iAddNum,CameraInfo* removed,int iRemoveNum,  void * user);

/**
* @~english
* @brief reporting channel of camera alarm informattion
* @param[in]	jsonData	report data,orgainze the data in json form
* @param[in]	iDataLen	length of report data content
* @param[in]	userData	context parameter of callback,which is the context passed in when setting
* @return		none
*/

/** @brief camera state change callback */
typedef void(*CCameraAlarmCallback)(const char* jsonData, int iDataLen, void * userData);


/**
* \brief Creates context that is required for the rest of the API.
* \return Context object
*/
CS_API CSystem* createSystem();

/**
* \brief Frees the relevant context object.
* \param[in] context Object that is no longer needed
*/
CS_API void deleteSystem(CSystem* sys);

/**
* create a static snapshot of all connected devices at the time of the call
* \param sys    Object representing lib3dcamera system
* \param count  count of connected cameras
* \return	point to info list of connected cameras
*/
CS_API CameraInfo* systemCreateCameraInfoList(const CSystem* sys,int* count);

/**
* add a usb camera fd
* \param sys    Object representing lib3dcamera system
* \param fd  usb fd
* @return success:return SUCCESS, fail:other error code
*/
CS_API ERROR_CODE addUsbCameraFd(const CSystem* sys,IN int fd);

/**
* remove a usb camera fd
* \param sys    Object representing lib3dcamera system
* \param fd  usb fd
* @return success:return SUCCESS, fail:other error code
*/
CS_API ERROR_CODE removeUsbCameraFd(const CSystem* sys, IN int fd);

/**
* Deletes device list, any devices created using this list will remain unaffected.
* \param[in]  info_list List to delete
*/
CS_API void systemDeleteCameraInfoList(CameraInfo* list);

/**
* set device changed callback
*/
CS_API void systemSetCameraChangeCallback(CSystem* sys, CCameraChangeCallback callback, void* user);

/**
* set system alarm callback
*/
CS_API void systemSetCameraAlarmCallback(CSystem* sys, CCameraAlarmCallback callback, void* user);

/**
* Create a camera by serial number. If serail number is empty, then connect to any camera
* \param[in] serail		Specifies the serial number of the desired connection
* \return               The requested camera, should be released by camera_disconnect
*/
CS_API CCamera* systemConnectCamera(CSystem* sys, CameraInfo* info);

/**
* Delete camera
*\param[in] camera		Handle of connected camera
*/
CS_API void systemDisconnectCamera(CSystem* sys, CCamera* camera);

#ifdef __cplusplus
}
#endif
#endif
