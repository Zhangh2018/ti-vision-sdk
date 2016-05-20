 /*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

/**
 *******************************************************************************
 *
 * \file network_api.h Wrapper API's to use TCP/IP sockets
 *
 *******************************************************************************
 */

#ifndef NETWORK_API_H_
#define NETWORK_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *  Include files
 *******************************************************************************
 */

#include <include/link_api/networkCtrl_api.h>

#include <osal/bsp_osal.h>

/* NDK Dependencies */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/tools/servers.h>
#include <ti/ndk/inc/stkmain.h>
#include <ti/ndk/inc/stack/stack.h>

/*******************************************************************************
 *  Defines
 *******************************************************************************
 */


/*******************************************************************************
 *  Data structure's
 *******************************************************************************
 */

/*******************************************************************************
 *  \brief Information related to network socket
 *******************************************************************************
 */
typedef struct {

    SOCKET sockFd;
    /**< Server socket handle */

    UInt32 port;
    /**< port on which server is listening */

    SOCKET connectedSockFd;
    /**< socket handle of client that is connected */

    FDPOLLITEM pollitem[1U];
    /**< Polling structure to use with fdPoll() */

} Network_SockObj;

/*******************************************************************************
 *  Function's
 *******************************************************************************
 */

Int32 Network_open(Network_SockObj *pObj, UInt32 port);
Int32 Network_close(Network_SockObj *pObj, Bool closeServerSock);
Int32 Network_waitConnect(Network_SockObj *pObj, UInt32 timeout);
Int32 Network_read(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 *dataSize);
Int32 Network_write(Network_SockObj *pObj, UInt8 *dataBuf, UInt32 dataSize);
Int32 Network_sessionOpen(BspOsal_TaskHandle handle);
Int32 Network_sessionClose(BspOsal_TaskHandle handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/* @} */


