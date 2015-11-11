//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#include "StdAfx.h"
#include "IocpHandler.h"
#include "IocpServer.h"

namespace iocp {


void CIocpHandler::OnNewConnection( uint64_t, ConnectionInformation const & )
{

}

void CIocpHandler::OnServerError( int /*errorCode*/ )
{

}

void CIocpHandler::OnSentData( uint64_t /*cid*/, uint64_t /*byteTransferred*/ )
{

}

void CIocpHandler::OnReceiveData( uint64_t /*cid*/, std::vector<uint8_t> const &/*data*/ )
{

}

void iocp::CIocpHandler::OnClientDisconnect( uint64_t cid, int32_t )
{
	try
	{
		GetIocpServer().Shutdown(cid, SD_SEND);

		GetIocpServer().Disconnect(cid);
	}
	catch (iocp::CWin32Exception const &)
	{
	}
	catch (iocp::CIocpException const &)
	{
	}
}

void CIocpHandler::OnServerClose( int32_t /*errorCode*/ )
{

}

void CIocpHandler::OnDisconnect( uint64_t /*cid*/, int32_t /*errorcode*/)
{

}


CIocpServer & CIocpHandler::GetIocpServer()
{
	return *m_iocpServer;
}

} // end namespace