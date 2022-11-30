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
#ifndef __RPC_SERVER_INTERFACE_HPP__
#define __RPC_SERVER_INTERFACE_HPP__
#include "APIExport.hpp"
#include "Types.hpp"

namespace cs {
	#ifdef _cplusplus
		extern "C" {
	#endif
			/**
			 *		start rpc server
			 *
			 */
			CS_API ERROR_CODE startRpcServer(unsigned short listenPort);

			CS_API ERROR_CODE stopRpcServer();

	#ifdef _cplusplus
		}
	#endif
	#endif
};
                                                                                                                                                             