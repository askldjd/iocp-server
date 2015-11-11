//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef UTILS_H_2010_09_28_10_55_30
#define UTILS_H_2010_09_28_10_55_30

#include "../ConnectionInformation.h"

namespace iocp { namespace detail { class CSharedIocpData; } };
namespace iocp { namespace detail { class CIocpContext; } };
namespace iocp { namespace detail { class CConnection; } };

namespace iocp { namespace detail {

	int 
	GetNumIocpThreads();

	SOCKET 
	CreateOverlappedSocket();

	ConnectionInformation 
	GetConnectionInformation(SOCKET socket);

	void 
	PostAccept(CSharedIocpData &iocpData);

	int 
	PostRecv( CIocpContext &iocpContext );

	int 
	PostSend(CIocpContext &iocpContext );

	int 
	PostDisconnect(CSharedIocpData &iocpData, CConnection &c);

	void 
	AssociateDevice(HANDLE h, CSharedIocpData &iocpData);

	HANDLE 
	CreateIocp(int maxConcurrency = 0);

	LPFN_ACCEPTEX
	LoadAcceptEx(SOCKET s);


} } // end namespace
#endif // UTILS_H_2010_09_28_10_55_30
