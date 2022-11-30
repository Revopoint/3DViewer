#ifndef LIB_3DCAMERA_FRAME_H
#define LIB_3DCAMERA_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../hpp/Types.hpp"
#include "types.h"
/**
* Retrieve timestamp from frame handle in milliseconds
* \param[in] frame      handle returned from a callback
* \return               the timestamp of the frame in milliseconds
*/
	CS_API double frameGetTimestamp(const CFrame* frame);

/**
* Retrieve data from frame handle
* \param[in] frame      handle returned from a callback
* \return               the pointer to the start of the frame data
*/
	CS_API const void* frameGetData(const CFrame* frame);

	CS_API const void* frameGetDataByFormat(const CFrame* frame, FRAME_DATA_FORMAT format);

/**
* Retrieve frame size in bytes
* \param[in] frame      handle returned from a callback
* \return               the size of the frame data
*/
	CS_API int frameGetDataSize(const CFrame* frame);

/**
* Retrieve frame width in pixels
* \param[in] frame      handle returned from a callback
* \return               frame width in pixels
*/
	CS_API int frameGetWidth(const CFrame* frame);

/**
* Retrieve frame height in pixels
* \param[in] frame      handle returned from a callback
* \return               frame height in pixels
*/
	CS_API int frameGetHeight(const CFrame* frame);


	CS_API STREAM_FORMAT frameGetFormat(const CFrame* frame);

/*
* Gets the extra information
* \param[in] frame	Handle returned from a callback
* \param[out] info	Output the stream information
*/
	CS_API ExtraInfo* frameGetExtraInfo(const CFrame* frame);

#ifdef __cplusplus
}
#endif
#endif
