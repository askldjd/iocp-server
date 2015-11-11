//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef WORKERTHREAD_H_2010_09_22_22_50_27
#define WORKERTHREAD_H_2010_09_22_22_50_27

namespace iocp { namespace detail { class CSharedIocpData; } }
namespace iocp { namespace detail { class CIocpContext; } }

namespace iocp { namespace detail {

class CWorkerThread
{
public:

	explicit CWorkerThread(CSharedIocpData &sharedData);

	~CWorkerThread();

	void Run();

private:
	void HandleIocpContext( CIocpContext &iocpContext, DWORD bytesTransferred );

	void HandleCompletionFailure( OVERLAPPED * overlapped, DWORD bytesTransferred, int error );

	void HandleReceive( CIocpContext &iocpContext, DWORD bytesTransferred );

	void HandleSend( CIocpContext &iocpContext, DWORD bytesTransferred );

	void HandleAccept(CIocpContext &iocpContext, DWORD bytesTransferred );

	void HandleDisconnect(CIocpContext &iocpContext);
private:

	boost::thread m_thread;

	CSharedIocpData &m_iocpData;
};

} } // end namespace
#endif // WORKERTHREAD_H_2010_09_22_22_50_27