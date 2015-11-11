//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#include "StdAfx.h"
#include "SendQueue.h"
#include "IocpContext.h"

namespace iocp { namespace detail {


CSendQueue::~CSendQueue()
{
	// This shouldn't be necessary because the server is programmed to
	// wait for all outstanding context to come back before going down.
	// But just in case...
	CloseAllSends();
}

void CSendQueue::AddSendContext( shared_ptr<CIocpContext> sendContext )
{
	mutex::scoped_lock l(m_mutex);

	bool inserted = m_sendContextMap.insert(
		std::make_pair(sendContext.get(), sendContext)
		).second;
	assert(true == inserted);
}

int CSendQueue::RemoveSendContext( CIocpContext* sendContext )
{
	mutex::scoped_lock l(m_mutex);
	m_sendContextMap.erase(sendContext);
	return m_sendContextMap.size();
}

uint32_t CSendQueue::NumOutstandingContext()
{
	mutex::scoped_lock l(m_mutex);
	return m_sendContextMap.size();
}

void CSendQueue::CloseAllSends()
{
	mutex::scoped_lock l(m_mutex);

	SendContextMap_t::iterator itr = m_sendContextMap.begin();
	while(m_sendContextMap.end() != itr)
	{
		if(INVALID_HANDLE_VALUE != itr->second->hEvent)
		{
			CloseHandle(itr->second->hEvent);
			itr->second->hEvent = INVALID_HANDLE_VALUE;
		}
		++itr;
	}
}
} } // end namespace