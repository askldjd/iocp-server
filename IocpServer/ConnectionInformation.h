//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef CONNECTIONINFORMATION_H_20090416_155746
#define CONNECTIONINFORMATION_H_20090416_155746

#if _MSC_VER > 1000 
#pragma once  
#endif // _MSC_VER > 1000

namespace iocp {

	class ConnectionInformation
	{
	public:
		ConnectionInformation()
			: m_remotePortNumber(0)
		{

		}
		tstring m_remoteIpAddress;
		tstring m_remoteHostName;
		boost::uint16_t m_remotePortNumber;
	};

} // end namespace

#endif //CONNECTIONINFORMATION_H_20090416_155746