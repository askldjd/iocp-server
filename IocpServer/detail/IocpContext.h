//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef IOCPCONTEXT_H_2010_09_25_17_35_46
#define IOCPCONTEXT_H_2010_09_25_17_35_46

namespace iocp { namespace detail {

//! @details
//! The overlapped object in IOCP serves as a context (or metadata) for
//! each completion packet. Each overlapped operations has its unique
//! context data, so that we know how to route and operate on them.
//! The overlapped data structure is very C-like, and requires special
//! care of start and stops. 
class CIocpContext : public OVERLAPPED
{
public:

	enum Type
	{ 
		Rcv,
		Send,
		Accept,
		Disconnect,
	};


	CIocpContext(
		SOCKET socket, 
		uint64_t cid, 
		CIocpContext::Type t, 
		uint32_t rcvBufferSize);

	~CIocpContext();

	//! Reset the WSA buffer. Should be called each time the context is used.
	void ResetWsaBuf();

	//! the actual buffer that holds all the data
	std::vector<uint8_t> m_data;

	//! ptr to the winsock buffer (which points to m_data)
	WSABUF m_wsaBuffer;

	//! the socket for this connection
	SOCKET m_socket;

	//! connection id
	uint64_t m_cid;

	//! the type of iocp context
	Type m_type;

	uint32_t m_rcvBufferSize;
};

} } // end namespace
#endif // IOCPCONTEXT_H_2010_09_25_17_35_46