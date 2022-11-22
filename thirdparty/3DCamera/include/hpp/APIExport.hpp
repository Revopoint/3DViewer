 /*****************************************************************************
*  3DCamera SDK header
*
*
*  @file     APIExport.hpp
*  @brief    3DCamera sdk header
*
*  @version  1.0
*  @date     2019 / 08 / 17
*
*****************************************************************************/
#ifndef __APIEXPORT_HPP__
#define __APIEXPORT_HPP__

#if defined(CS_API_EXPORTS)
    #ifdef _WIN32
		#define CS_API __declspec(dllexport)
    #else
        #define CS_API __attribute__ ((visibility ("default")))
    #endif
#else
    #ifdef _WIN32
		#define CS_API __declspec(dllimport)
    #else
        #define CS_API
    #endif
#endif

#endif