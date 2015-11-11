//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#include "StdAfx.h"
#include "Utils.h"
#include "SharedIocpData.h"
#include "IocpContext.h"
#include "Connection.h"
#include "../IocpHandler.h"
namespace iocp { namespace detail {

	SOCKET CreateOverlappedSocket()
	{
		return WSASocket(
			AF_INET, 
			SOCK_STREAM, 
			IPPROTO_TCP, 
			NULL, 
			0, 
			WSA_FLAG_OVERLAPPED);
	}


	//****************************************************************************
	//! @details
	//! This method extracts the Connection Information object in real time.
	//! Therefore, the accept socket must be valid.
	//!
	//! @param[in] socket 
	//! The socket to extract connection information from
	//!
	//! @return ndc::tcpip::ConnectionInformation
	//! Returns the Connection Information based on the accept socket
	//!
	//! @remark
	//! If the returned Connection Information object holds no information,
	//! it implies that the function has failed.
	//!
	//****************************************************************************
	ConnectionInformation GetConnectionInformation(SOCKET socket)
	{
		ConnectionInformation ci;

		// initialize the sockaddr_in object and its length
		sockaddr_in name;
		memset(&name, 0, sizeof(name));
		int namelen = sizeof(name);

		// getpeername to extract the remote party information
		if(::getpeername(socket, (sockaddr *) &name, &namelen) !=0)
		{
			return ci;
		}

#ifdef UNICODE
		// Unicode version of converting address to a unicode string
		int ansiLen = lstrlenA(inet_ntoa(name.sin_addr));
		LPWSTR unicodeIp = (LPWSTR)malloc((ansiLen) * sizeof(TCHAR));
		if(::MultiByteToWideChar(
			CP_ACP, 
			0, 
			inet_ntoa(name.sin_addr), 
			ansiLen, 
			unicodeIp, 
			ansiLen) != 0)
		{
			// The converted unicode IP address string is not NULL
			// terminated. So we need to append the exact count.
			ci.m_remoteIpAddress.append(unicodeIp, ansiLen);
		}
		
		free(unicodeIp);

#else
		ci.m_remoteIpAddress = inet_ntoa(name.sin_addr);
#endif
		ci.m_remotePortNumber = ntohs(name.sin_port);

		// Call getnameinfo to extract the hostname from the IP address
		TCHAR hostname[NI_MAXHOST] = {0};
		TCHAR servInfo[NI_MAXSERV] = {0};

#ifdef UNICODE
		if(GetNameInfoW(
			(sockaddr *) &name,
			sizeof (sockaddr), 
			hostname, // hostname
			NI_MAXHOST, // size of host name
			servInfo,  // service info = port
			NI_MAXSERV, // size of service info
			NI_NUMERICSERV) != 0) // use numeric port option
		{
			return ci;
		}
#else
		if(getnameinfo(
			(sockaddr *) &name,
			sizeof (sockaddr), 
			hostname, // hostname
			NI_MAXHOST, // size of host name
			servInfo,  // service info = port
			NI_MAXSERV, // size of service info
			NI_NUMERICSERV) != 0) // use numeric port option
		{
			return ci;
		}
#endif 

		ci.m_remoteHostName = hostname;

		return ci;
	}

	void PostAccept(CSharedIocpData &iocpData) 
	{
		DWORD bytesReceived_ = 0;
		DWORD addressSize = sizeof(sockaddr_in) + 16;

		if (iocpData.m_acceptExFn(
			iocpData.m_listenSocket,  // listen socket
			iocpData.m_acceptContext.m_socket, // accept socket
			&iocpData.m_acceptContext.m_data[0], // holds local/remote address
			0, // receive data length = 0 for no receive on accept
			addressSize, // local address length
			addressSize, // remote address length
			&bytesReceived_,
			&iocpData.m_acceptContext) == FALSE)
		{
			DWORD lastError = GetLastError();
			if (lastError != ERROR_IO_PENDING)
			{
				if(iocpData.m_iocpHandler != NULL)
				{
					iocpData.m_iocpHandler->OnServerError(lastError);
				}
			}
		}
		else
		{
			PostQueuedCompletionStatus(
				iocpData.m_ioCompletionPort, 
				0, 
				(DWORD) 
				(ULONG_PTR)&iocpData, 
				&iocpData.m_acceptContext);
		}
	}


	int PostRecv( CIocpContext &iocpContext ) 
	{
		DWORD dwBytes = 0, dwFlags = 0;

		if(WSARecv(
			iocpContext.m_socket,
			&iocpContext.m_wsaBuffer, 
			1, 
			&dwBytes, 
			&dwFlags, 
			&iocpContext, 
			NULL) == SOCKET_ERROR)
		{
			return WSAGetLastError();
		}

		return WSA_IO_PENDING;
	}


	int PostSend( CIocpContext &iocpContext )
	{
		DWORD dwBytes = 0;

		if(WSASend(
			iocpContext.m_socket, 
			&iocpContext.m_wsaBuffer, 
			1, 
			&dwBytes, 
			0, 
			&iocpContext, 
			NULL) == SOCKET_ERROR)
		{
			return WSAGetLastError();
		}

		return WSA_IO_PENDING;
	}

	void AssociateDevice(HANDLE h, CSharedIocpData &iocpData) 
	{
		if (::CreateIoCompletionPort(
			h, 
			iocpData.m_ioCompletionPort, 
			(ULONG_PTR)&iocpData, 
			0) != iocpData.m_ioCompletionPort)
		{
			if(iocpData.m_iocpHandler != NULL)
			{
				iocpData.m_iocpHandler->OnServerError(::GetLastError());
			}
		}
	}

	HANDLE CreateIocp(int maxConcurrency)
	{
		//Create I/O completion port
		// See http://msdn.microsoft.com/en-us/library/aa363862%28VS.85%29.aspx
		HANDLE h = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE, 
			NULL, 
			0, 
			maxConcurrency
			);

		assert(NULL != h);

		return h;
	}

	int GetNumIocpThreads()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors*2;
	}

	int PostDisconnect(CSharedIocpData &iocpData, CConnection &c)
	{
		CIocpContext * disconnectContext = new CIocpContext(
			c.m_socket, 
			c.m_id, 
			CIocpContext::Disconnect,
			0);

		//Help threads get out of blocking - GetQueuedCompletionStatus()
		if(PostQueuedCompletionStatus(
			iocpData.m_ioCompletionPort,
			0, 
			(ULONG_PTR)&iocpData, 
			(LPOVERLAPPED)disconnectContext) == FALSE)
		{
			return GetLastError();
		}

		return NO_ERROR;
	}

	//!***************************************************************************
	//! @details
	//! From Network Programming for Microsoft Windows - APIs and Scalability
	//!
	//! Quote:
	//! Applications should always load the extension functions themselves to 
	//! avoid the performance penalty of the exported extension functions from 
	//! MSWSOCK.DLL, because for each call they simply end up loading the 
	//! same function.
	//!
	//! @param[in] s
	//! Listen socket
	//!
	//! @return LPFN_ACCEPTEX
	//! AcceptEx function, NULL if not found.
	//!
	//!***************************************************************************
	LPFN_ACCEPTEX LoadAcceptEx(SOCKET s)
	{
		LPFN_ACCEPTEX lpfnAcceptEx=NULL;
		DWORD dwBytes = 0;

		GUID GuidAcceptEx=WSAID_ACCEPTEX;

		if(WSAIoctl(
			s,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx,
			sizeof(GuidAcceptEx),
			&lpfnAcceptEx,
			sizeof(lpfnAcceptEx),
			&dwBytes,
			NULL,
			NULL
			) == SOCKET_ERROR)
		{
			return NULL;
		}

		return lpfnAcceptEx;
	}
} } // end namespace