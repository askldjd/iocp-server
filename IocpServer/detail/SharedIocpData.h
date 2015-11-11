//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef SHAREDIOCPDATA_H_2010_09_21_23_51_40
#define SHAREDIOCPDATA_H_2010_09_21_23_51_40

#include "ConnectionManager.h"
#include "IocpContext.h"
#include "../ConnectionInformation.h"

namespace iocp { class CIocpHandler; }

namespace iocp { namespace detail {

class CSharedIocpData : boost::noncopyable
{
public:
	CSharedIocpData() 
		: m_shutdownEvent(INVALID_HANDLE_VALUE)
		, m_ioCompletionPort(INVALID_HANDLE_VALUE)
		, m_listenSocket(INVALID_SOCKET)
		, m_acceptContext(INVALID_SOCKET, 0, CIocpContext::Accept, 4096)
		, m_rcvBufferSize(0)
		, m_currentId(0)
		, m_acceptExFn(NULL)

	{

	}


	LONGLONG GetNextId()
	{
		{
			mutex::scoped_lock l(m_cidMutex);
			++m_currentId;
		}

		return m_currentId;
	}


	HANDLE m_shutdownEvent;
	HANDLE m_ioCompletionPort;
	SOCKET m_listenSocket;
	CConnectionManager m_connectionManager;
	shared_ptr<CIocpHandler> m_iocpHandler;
	CIocpContext m_acceptContext;
	uint32_t m_rcvBufferSize;
	LPFN_ACCEPTEX m_acceptExFn;

private:
	mutex m_cidMutex;
	LONGLONG m_currentId;
};

} } // end namespace

#endif // SHAREDIOCPDATA_H_2010_09_21_23_51_40