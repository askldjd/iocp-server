//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#include "StdAfx.h"
#include "IocpContext.h"

namespace iocp { namespace detail {

CIocpContext::CIocpContext(SOCKET socket, 
						   uint64_t cid, 
						   CIocpContext::Type t,
						   uint32_t rcvBufferSize)
: m_socket(socket)
, m_cid(cid)
, m_type(t)
, m_rcvBufferSize(rcvBufferSize)
{
	if(Rcv == t || Accept == t)
	{
		m_data.resize(m_rcvBufferSize);
		ResetWsaBuf();
	}
	
	// Clear out the overlapped struct. Apparently, you must be do this, 
	// otherwise the overlap object will be rejected.
	memset(this, 0, sizeof(OVERLAPPED));

	// Create the event for waiting on IOCP. 
	hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}

CIocpContext::~CIocpContext()
{
	CloseHandle(hEvent);
}

void CIocpContext::ResetWsaBuf()
{
	m_wsaBuffer.buf = reinterpret_cast<char *>(&m_data[0]);
	m_wsaBuffer.len = static_cast<u_long>(
		m_data.size() * sizeof(m_data[0])
		);
}

} } // namespace