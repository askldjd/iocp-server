//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef IOCPHANDLER_H_2010_09_25_13_31_47
#define IOCPHANDLER_H_2010_09_25_13_31_47

#include "IocpException.h"
#include "ConnectionInformation.h"

namespace iocp {

class IOCPSERVER_API CIocpHandler
{
public:

	friend class CIocpServer;

	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when a new connection is accepted
	//! by the IOCP server. 
	//!
	//! @param[in] cid
	//! A unique Id that represents the connection. 
	//! This is the unique key that may be used for bookkeeping, and will remain
	//! valid until OnDisconnect is called.
	//!
	//! @param[in] c
	//! Information regarding the endpoints of the connection.
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnNewConnection(uint64_t cid, ConnectionInformation const & c);


	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when a data packet is received
	//! for a particular connection.
	//!
	//! @param[in] cid
	//! A unique Id that represents the connection. 
	//! This is the unique key that may be used for bookkeeping, and will remain
	//! valid until OnDisconnect is called.
	//!
	//! @param[in] data
	//! The received data packet from the connection. This packet buffer is 
	//! used by IOCP, and must not be kept or modified.
	//!
	//! @remark
	//! It is guaranteed that when this callback is invoked, some data has
	//! been received. Therefore, the "data" parameter is never empty.
	//!
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data);


	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when an outstanding send
	//! has been completed. It is not required to perform any function
	//! call to keep sending data. This call is merely used for bookkeeping
	//! purposes.
	//!
	//! @param[in,out] cid
	//! A unique Id that represents the connection. 
	//! This is the unique key that may be used for bookkeeping, and will remain
	//! valid until OnDisconnect is called.
	//!
	//! @param[in,out] byteTransferred
	//! Number of bytes that has just been transferred.
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnSentData(uint64_t cid, uint64_t byteTransferred);

	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when a connected client tries
	//! to signal end of session and that client has no more data to send.
	//!
	//! When this callback is invoked, the connection should be at half-closed
	//! state. There should be no data remaining to be received. Hence, 
	//! OnReceiveData will no longer be called for this connection.
	//!
	//! To perform a graceful shutdown, perform the following.
	//! 1. Send all the data that you need to send. It is not required to
	//! send all remaining data within this callback. The connection will 
	//! remain in half-closed state as long as the connected client also
	//! wish to perform graceful shutdown.
	//!
	//! 2. Once all remaining data are sent, Call 
	//! CIocpServer.Shutdown(cid, SD_SEND) to close the other half of
	//! of the connection.
	//!
	//! 3. Finally, call CIocpServer.Disconnect(cid) to fully close the socket. 
	//! After this, the OnDisconnect callback will be asynchronously called 
	//! through one of the IOCP thread. The connection is then fully 
	//! terminated.
	//! 
	//! @param[in] cid
	//! A unique Id that represents the connection. 
	//! This is the unique key that may be used for bookkeeping, and will remain
	//! valid until OnDisconnect is called.
	//!
	//! @param[in] errorcode
	//! Error code for the disconnect
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//! If this function is not overridden, the default behavior is to 
	//! immediately close the other half of the socket, and fully close the
	//! connection.	After this, the OnDisconnect callback will be called 
	//! through one of the IOCP thread. The connection is fully terminated.
	//!
	//!***************************************************************************
	virtual void OnClientDisconnect(uint64_t cid, int32_t errorcode);


	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when a client fully disconnects
	//! from the server. 
	//!
	//! For every connection that invoked OnNewConnection, there will be 
	//! a corresponding OnDisconnect called.
	//!
	//! @param[in] cid
	//! A unique Id that represents the connection. 
	//! The connection id is no longer valid after this call.
	//!
	//! @param[in] errorcode
	//! Error code for the disconnect
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnDisconnect(uint64_t cid, int32_t errorcode);

	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when the server encounter any
	//! type of errors after it has been initialized. Errors may or may not be
	//! fatal, and it is up to the user to decide whether or not to shut down
	//! the server.
	//!
	//! @param[in] errorCode
	//! The error code that the server encountered.
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnServerError(int errorCode);
	
	
	//!***************************************************************************
	//! @details
	//! This callback is invoked asynchronously when the server has completely
	//! closed. This callback is useful for state tracking. At this point,
	//! all connections are gone, and the server has closed the io completion
	//! port. 
	//!
	//! @param[in,out] errorCode
	//! Error code when the server exits.
	//!
	//! @remark
	//! This callback is invoked through the context of an IOCP thread, which
	//! may or may not be your main thread's context.
	//!
	//!***************************************************************************
	virtual void OnServerClose(int32_t errorCode);
	
	CIocpServer &GetIocpServer();

private:
	CIocpServer *m_iocpServer;

};

} // end namespace
#endif // IOCPHANDLER_H_2010_09_25_13_31_47