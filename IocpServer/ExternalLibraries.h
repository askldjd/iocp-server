//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXTERNALLIBRARIES_H_2010_09_21_23_22_49
#define EXTERNALLIBRARIES_H_2010_09_21_23_22_49

// Disable Secure SCL for performance reason.
#define _SECURE_SCL 0
#define _SECURE_SCL_THROWS 0
#define _HAS_ITERATOR_DEBUGGING 0

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN		
#include "windows.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
// Mswsock.h must be included after WinSock2.h
#include <Mswsock.h>

#include "Export.h"
#include <iostream>

// Disable boost datetime warnings where it is implicitly casting integer to short.
#pragma warning(disable:4244)
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#pragma warning(default:4244)

#include <tchar.h>


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")



namespace iocp
{
	using boost::shared_ptr;
	using boost::uint8_t;
	using boost::uint16_t;
	using boost::uint32_t;
	using boost::uint64_t;
	using boost::int8_t;
	using boost::int16_t;
	using boost::int32_t;
	using boost::int64_t;
	using boost::thread;
	using boost::mutex;
	using boost::bind;
	using boost::function;
	using boost::noncopyable;

#ifdef _UNICODE
	typedef std::wstring tstring;
#else
	typedef std::string tstring;
#endif

#ifndef UNICODE
	#define tcout std::cout
#else
	#define tcout std::wcout
#endif

}

#endif // EXTERNALLIBRARIES_H_2010_09_21_23_22_49