//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifdef IOCPSERVER_EXPORTS
#define IOCPSERVER_API __declspec(dllexport)
#else
#define IOCPSERVER_API __declspec(dllimport)
#endif

#if defined(_WIN32)
#	if !defined(_DLL) || !defined(_MT)
#		error This library must be used with the multi-threaded DLL CRT
#	endif
#endif

#pragma warning(disable:4251 4275)
