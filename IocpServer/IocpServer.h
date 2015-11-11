//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef IOCPSERVER_H_2010_09_25_14_37_36
#define IOCPSERVER_H_2010_09_25_14_37_36

#include "Export.h"
#include "IocpHandler.h"

//////////////////////////////////////////////////////////////////////////
//
// History
// Oct. 26 2010
// Version 1.0
// Creation
//
// Dec. 10 2010 
// Version 1.1 
// Fixed a bug where the IOCP server stops accepting connection when
// setsockopt() fails.
//
// Removed an unnecessary event per Len Holgate's suggestion.
//
//////////////////////////////////////////////////////////////////////////

namespace iocp {

class IOCPSERVER_API CIocpServer
{
public:
	//!***************************************************************************
	//! @details
	//! Constructor
	//! Upon construction, CIocpServer will bind to the indicated port 
	//! and the provided address.
	//!
	//! @param[in] port
	//! The port to listen on.
	//!
	//! @param[in,out] iocpHander
	//! The handler for IOCP server events. This must be supplied, and IOCP
	//! server will hold a shared_ptr until its destruction.
	//!
	//! @param[in] addressToListenOn
	//! The IP address that the server will listen to.
	//!
	//! @param[in] rcvbufferSize
	//! The buffer size for receive. 0 = default = 4096 bytes.
	//!
	//! @param[in] numThread
	//! The number of IOCP threads in thread pool. 
	//! 0 = default = Number of processor *2.
	//!
	//! @throw
	//! CWin32Exception for any initialization failure.
	//!
	//!***************************************************************************
	CIocpServer(
		uint16_t port,
		shared_ptr<CIocpHandler> iocpHandler,
		uint32_t addressToListenOn = INADDR_ANY,
		uint32_t rcvbufferSize = 0,
		uint32_t numThread = 0
		);

	//!***************************************************************************
	//! @details
	//! Destructor
	//! Closes down the IO completion port, as well as ending all worker thread.
	//! All active connections will be shutdown abortively, and all outstanding 
	//! sends are discarded. OnDisconnect callback will not be invoked if there 
	//! are remaining connection in the IOCP server. 
	//!
	//! For graceful shutdown, it is the user's responsibility to ensure that 
	//! all connections are closed gracefully prior to destroying the IOCP server.
	//!
	//! OnServerClosed callback will be invoked when the server exits.
	//!
	//!***************************************************************************
	~CIocpServer();

	//!***************************************************************************
	//! @details
	//! Send data to a connected client. IOCP server will automatically queue
	//! up packets and send them in order when ready. A corresponding 
	//! OnSentData will be called when data are sent. 
	//!
	//! @param[in,out] cid
	//! The connection id to send the data to.
	//!
	//! @param[in,out] data
	//! Data to send. IOCP server will use the memory as is, and will not
	//! allocate another copy. 
	//! 
	//! If the function operate successfully, swap() will be called on the 
	//! vector, and the buffer will be emptied after this function returned.
	//!
	//! If the function fails (exception thrown), the data will be left 
	//! as is.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! CWin32Exception if the IOCP server failed to post this data to the
	//! IO Completion port.
	//!
	//! @remark
	//! Send should not be called on the same connection simultaneously from 
	//! different threads, because it can result in an unpredictable send order.
	//! If Send() must be invoked from multiple threads , user should 
	//! implement custom critical section on top of Send() for each connection.
	//!
	//!***************************************************************************
	void Send(uint64_t cid, std::vector<uint8_t> &data);

	//!***************************************************************************
	//! @details
	//! Shutdown certain operation on the socket.
	//!
	//! @param[in,out] cid
	//! The id of the connection to shut down.
	//!
	//! @param[in,out] how
	//! A flag that describes what types of operation will no longer be 
	//! allowed. Possible values for this flag are listed in the Winsock2.h 
	//! header file.
	//!
	//! SD_RECEIVE - Shutdown receive operations.
	//! SD_SEND - Shutdown send operations.
	//! SD_BOTH - Shutdown both send and receive operations.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! CWin32Exception if the IOCP server failed to post this data to the
	//! IO Completion port.
	//!
	//! @post
	//! If the function completes successfully, the specified operation
	//! on the socket will no longer be allowed.
	//!
	//! @remark
	//! Shutdown should not be called on the same connection simultaneously from 
	//! different threads. Otherwise, the post condition is undefined.
	//!
	//!***************************************************************************
	void Shutdown(uint64_t cid, int how);

	//!***************************************************************************
	//! @details
	//! Fully disconnect from a connected client. Once all outstanding sends
	//! are completed, a corresponding OnDisconnect callback will be invoked.
	//!
	//! @throw
	//! CIocpException if connection no longer exists.
	//!
	//! @param[in,out] cid
	//! The connection to disconnect from.
	//!
	//! @post
	//! If this function completes successfully, the socket will no longer
	//! be capable of sending or receiving new data (queued data prior to 
	//! disconnect will be sent).
	//!
	//! @remark
	//! If the server is initiating the disconnect, it is recommended to call 
	//! Shutdown(cid, SD_BOTH) and wait for OnClientDisconnected() callback prior 
	//! to calling Disconnect(). Otherwise, it is possible OnReceiveData() callback 
	//! to be called simultaneously with OnDisconnect() callback. After all, the 
	//! client may simultaneously send data to this server during the 
	//! Disconnect() call. In such scenario, you need to provide mutexes and
	//! additional logic to ignore the last sets of packets.
	//!
	//! Shutdown should not be called on the same connection simultaneously from 
	//! different threads. Otherwise, the post condition is undefined.
	//!
	//!***************************************************************************
	void Disconnect(uint64_t cid);

private:

	class CImpl;
	shared_ptr<CImpl> m_impl;
};

} // end namespace
#endif // IOCPSERVER_H_2010_09_25_14_37_36
