//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#include "StdAfx.h"
#include "WorkerThread.h"
#include "SharedIocpData.h"
#include "IocpContext.h"
#include "Connection.h"
#include "Utils.h"

#include "../IocpHandler.h"

namespace iocp { namespace detail {

CWorkerThread::CWorkerThread(CSharedIocpData &iocpData)
: m_iocpData(iocpData)
{
	m_thread = thread(bind(&CWorkerThread::Run, this));
}

CWorkerThread::~CWorkerThread()
{
	m_thread.join();
}

void CWorkerThread::Run()
{
	for(;;)
	{
		void *key = NULL;
		OVERLAPPED *overlapped = NULL;
		DWORD bytesTransferred = 0;

		BOOL completionStatus = GetQueuedCompletionStatus(
			m_iocpData.m_ioCompletionPort,
			&bytesTransferred,
			(LPDWORD)&key,
			&overlapped,
			INFINITE);

		if(FALSE == completionStatus)
		{
			HandleCompletionFailure(overlapped, bytesTransferred, GetLastError());
			continue;
		}

		// NULL key packet is a special status that unblocks the worker
		// thread to initial a shutdown sequence. The thread should be going
		// down soon.
		if(NULL == key)
		{
			break;
		}

		CIocpContext &iocpContext = 
			*reinterpret_cast<CIocpContext *>(overlapped);

		HandleIocpContext(iocpContext, bytesTransferred);
	}
}

void CWorkerThread::HandleReceive( CIocpContext &rcvContext, DWORD bytesTransferred )
{
	shared_ptr<CConnection> c = 
		m_iocpData.m_connectionManager.GetConnection(rcvContext.m_cid);
	if(c == NULL)
	{
		assert(false);
		return;
	}

	// If nothing is transferred, we are about to be disconnected. In this
	// case, don't notify the client that nothing is received because
	// they are about to get a disconnection callback.
	if(0 != bytesTransferred)
	{
		// Shrink the buffer to fit the byte transferred
		rcvContext.m_data.resize(bytesTransferred);
		assert(rcvContext.m_data.size() == bytesTransferred);

		if(m_iocpData.m_iocpHandler != NULL)
		{
			// Invoke the callback for the client
			m_iocpData.m_iocpHandler->OnReceiveData(
				rcvContext.m_cid,
				rcvContext.m_data);
		}
	}

	// Resize it back to the original buffer size and prepare to post
	// another completion status.
	rcvContext.m_data.resize(rcvContext.m_rcvBufferSize);
	rcvContext.ResetWsaBuf();

	int lastError = NO_ERROR;

	// 0 bytes transferred, or if a recv context can't be posted to the 
	// IO completion port, that implies the socket at least half-closed.
	if( (0 == bytesTransferred) || 
		WSA_IO_PENDING != (lastError = PostRecv(rcvContext)) )
	{
		uint64_t cid = rcvContext.m_cid;
		if (c->CloseRcvContext() == true)
		{
			::shutdown(c->m_socket, SD_RECEIVE);

			if(m_iocpData.m_iocpHandler != NULL)
			{
				m_iocpData.m_iocpHandler->OnClientDisconnect(
					cid,
					lastError);
			}
		}
	}
}

void CWorkerThread::HandleSend( CIocpContext &iocpContext, DWORD bytesTransferred )
{
	shared_ptr<CConnection> c = 
		m_iocpData.m_connectionManager.GetConnection(iocpContext.m_cid);
	if(c == NULL)
	{
		assert(false);
		return;
	}

	uint64_t cid = iocpContext.m_cid;

	if(bytesTransferred > 0 )
	{
		if(m_iocpData.m_iocpHandler != NULL)
		{
			m_iocpData.m_iocpHandler->OnSentData(cid, bytesTransferred);
		}
	}
	//No bytes transferred, that means send has failed.
	else
	{
		// what to do here?
	}

	//! @remark
	//! Remove the send context after notifying the user. Otherwise
	//! there is a race condition where a disconnect context maybe waiting 
	//! for the send queue to go to zero at the same time. In this case,
	//! the disconnect notification will come before we notify the user.
	int outstandingSend = c->m_sendQueue.RemoveSendContext(&iocpContext);

	// If there is no outstanding send context, that means all sends 
	// are completed for the moment. At this point, if we have a half-closed 
	// socket, and the connection is pending to be disconnected, post a 
	// disconnect context for a graceful shutdown.
	if(0 == outstandingSend)
	{
		if( (::InterlockedExchangeAdd(&c->m_rcvClosed, 0) > 0) &&
			(::InterlockedExchangeAdd(&c->m_disconnectPending, 0) > 0) )
		{
			// Disconnect context is special (hacked) because it is not
			// tied to a connection. During graceful shutdown, it is very
			// difficult to determine when exactly is a good time to 
			// remove the connection. For example, a disconnect context 
			// might have already been sent by the user code, and you
			// wouldn't know it unless mutex are used. To keep it as 
			// lock-free as possible, the disconnect handler
			// will gracefully handle redundant disconnect context.
			PostDisconnect(m_iocpData, *c);
		}
	}
}

void CWorkerThread::HandleAccept( CIocpContext &acceptContext, DWORD bytesTransferred )
{
	// We should be accepting immediately without waiting for any data.
	// If this has change, we need to account for that accept buffer and post
	// it to the receive side.
	assert(0 == bytesTransferred);

	// Update the socket option with SO_UPDATE_ACCEPT_CONTEXT so that
	// getpeername will work on the accept socket.
	if(setsockopt(
		acceptContext.m_socket, 
		SOL_SOCKET, 
		SO_UPDATE_ACCEPT_CONTEXT, 
		(char *)&m_iocpData.m_listenSocket, 
		sizeof(m_iocpData.m_listenSocket)
		) != 0)
	{
		if(m_iocpData.m_iocpHandler != NULL)
		{
			// This shouldn't happen, but if it does, report the error. 
			// Since the connection has not been established, it is not necessary
			// to notify the client to remove any connections.
			m_iocpData.m_iocpHandler->OnServerError(WSAGetLastError());
		}
	}
	// If the socket is up, allocate the connection and notify the client.
	else
	{
		ConnectionInformation cinfo = 
			GetConnectionInformation(acceptContext.m_socket);

		shared_ptr<CConnection> c(new CConnection(
			acceptContext.m_socket, 
			m_iocpData.GetNextId(),
			m_iocpData.m_rcvBufferSize
			));

		m_iocpData.m_connectionManager.AddConnection(c);

		AssociateDevice((HANDLE)c->m_socket, m_iocpData);

		if(m_iocpData.m_iocpHandler != NULL)
		{
			m_iocpData.m_iocpHandler->OnNewConnection(c->m_id, cinfo);
		}

		int lasterror = PostRecv(c->m_rcvContext);

		// Failed to post a queue a receive context. It is likely that the
		// connection is already terminated at this point (by user or client).
		// In such case, just remove the connection.
		if( WSA_IO_PENDING != lasterror)
		{
			if(true == c->CloseRcvContext())
			{
				PostDisconnect(m_iocpData, *c);
			}
		}
	}

	// Post another Accept context to IOCP for another new connection.
	//! @remark
	//! For higher performance, it is possible to preallocate these sockets
	//! and have a pool of accept context waiting. That adds complexity, and
	//! unnecessary for now.
	acceptContext.m_socket = CreateOverlappedSocket();

	if(INVALID_SOCKET != acceptContext.m_socket)
	{
		PostAccept(m_iocpData);
	}
	else
	{
		if(m_iocpData.m_iocpHandler != NULL)
		{
			m_iocpData.m_iocpHandler->OnServerError(WSAGetLastError());
		}
	}
}

void CWorkerThread::HandleIocpContext(CIocpContext &iocpContext, 
									  DWORD bytesTransferred )
{
	switch(iocpContext.m_type)
	{
	case CIocpContext::Rcv:
		HandleReceive(iocpContext, bytesTransferred);
		break;
	case CIocpContext::Send:
		HandleSend(iocpContext, bytesTransferred);
		break;
	case CIocpContext::Accept:
		HandleAccept(iocpContext, bytesTransferred);
		break;
	case CIocpContext::Disconnect:
		HandleDisconnect(iocpContext);
		break;
	default:
		assert(false);
	}
}

void CWorkerThread::HandleCompletionFailure(OVERLAPPED * overlapped, 
											DWORD bytesTransferred, 
											int error )
{
	if (NULL != overlapped ) 
	{
		// Process a failed completed I/O request
		// dwError contains the reason for failure
		CIocpContext &iocpContext = 
			*reinterpret_cast<CIocpContext *>(overlapped);
		HandleIocpContext(iocpContext, bytesTransferred);
	} 
	else 
	{
		// Currently, GetQueuedCompletionStatus is called with an INFINITE
		// timeout flag, so it should not be possible for the status to 
		// timeout.
		assert(WAIT_TIMEOUT != error);
		
		if(m_iocpData.m_iocpHandler != NULL)
		{
			// GetQueuedCompletionStatus failed. Notify the user on this event.
			m_iocpData.m_iocpHandler->OnServerError(error);
		}
	}
}

void CWorkerThread::HandleDisconnect( CIocpContext &iocpContext )
{
	uint64_t cid = iocpContext.m_cid;

	// Disconnect context isn't tied to the connection. Therefore, it must
	// be deleted manually at all times.
	delete &iocpContext;

	shared_ptr<CConnection> c = 
		m_iocpData.m_connectionManager.GetConnection(cid);
	if(c == NULL)
	{
		//! @remark
		//! Disconnect context may come from several different source at 
		//! once, and only the first one to remove the connection. If the
		//! connection is already removed, gracefully handle it.

		//std::cout <<"Extra disconnect packet received" << std::endl;
		return;
	}

	if(c->HasOutstandingContext() == true)
	{
		return;
	}

	if(true == m_iocpData.m_connectionManager.RemoveConnection(cid) )
	{
		if(m_iocpData.m_iocpHandler != NULL)
		{
			m_iocpData.m_iocpHandler->OnDisconnect(cid,0);
		}
	}
}
} } // end namespace
