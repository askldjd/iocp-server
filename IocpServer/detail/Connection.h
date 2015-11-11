//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef CONNECTION_H_2010_09_25_17_15_00
#define CONNECTION_H_2010_09_25_17_15_00

#include "IocpContext.h"
#include "SendQueue.h"

namespace iocp { namespace detail {

class CConnection
{
public:

	CConnection(SOCKET socket, uint64_t cid, uint32_t rcvBufferSize);
	~CConnection();
	bool CloseRcvContext();

	shared_ptr<CIocpContext> CreateSendContext();

	bool HasOutstandingContext();

	SOCKET m_socket;
	uint64_t m_id;

	long m_disconnectPending;
	long m_sendClosePending;
	long m_rcvClosed;  

	CIocpContext m_rcvContext;

	CSendQueue m_sendQueue;

	CIocpContext m_disconnectContext;

	mutex m_connectionMutex;
};

} } // end namespace
#endif // CONNECTION_H_2010_09_25_17_15_00