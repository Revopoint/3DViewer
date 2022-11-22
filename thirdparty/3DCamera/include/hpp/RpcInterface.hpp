 /*****************************************************************************
*  3DCamera SDK implementations
*
*
*  @file     cs_system.hpp
*  @brief    System class interface
*
*  @version  v1.0
*  @date     2020 / 03 / 17
*
*****************************************************************************/
#ifndef __RPC_INTERFACE_HPP__
#define __RPC_INTERFACE_HPP__
#include "APIExport.hpp"
#include "Types.hpp"

#ifdef __cplusplus
extern "C" {
#endif
	/**
	* @~chinese
	* @brief      启动RpcServer
	* @param[in]  listenPort	：监听端口
	* @return     成功:SUCCESS, 失败:其他错误码
	**/
	CS_API ERROR_CODE startRpcServer(unsigned short listenPort);

	/**
	* @~chinese
	* @brief     停止RpcServer
	* @return    成功:SUCCESS, 失败:其他错误码
	**/
	CS_API ERROR_CODE stopRpcServer();

	/**
	*	设置RPC client连接指定的服务器的IP地址和端口号
	*@pcRpcServerIp		连接目标的IP地址
	*@usRpcServerPort	连接目标的监听端口号
	*/
	CS_API ERROR_CODE setRpcConnectInfo(const char* pcRpcServerIp, unsigned short usRpcServerPort);

	CS_API ERROR_CODE getRpcConnectInfo(OUT char acRpcServerIp[30], OUT unsigned short& usRpcServerPort);

#ifdef __cplusplus
}
#endif

#endif
                                                                                                                                                             