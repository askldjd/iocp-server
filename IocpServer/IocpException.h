//! Copyright Alan Ning 2010
//! Distributed under the Boost Software License, Version 1.0.
//! (See accompanying file LICENSE_1_0.txt or copy at
//! http://www.boost.org/LICENSE_1_0.txt)

#ifndef IOCPEXCEPTION_H_2010_09_22_23_03_38
#define IOCPEXCEPTION_H_2010_09_22_23_03_38

namespace iocp {

class CWin32Exception : public std::exception
{
public:
	explicit CWin32Exception(int m_errorCode) 
		: m_errorCode(m_errorCode) 
	{
		SetErrorMessage();
	}
	int ErrorCode() const { return m_errorCode; }
	void ErrorCode(int val) { m_errorCode = val; }

	tstring What() const
	{
		return m_errorMessage;
	}

private:

	void SetErrorMessage()
	{
		static TCHAR errmsg[512];

		if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
			0,
			m_errorCode,
			0,
			errmsg, 
			511,
			NULL))
		{
			// if it fails, recursive find out why we failed...
			m_errorCode = GetLastError();
			SetErrorMessage();
		}

		m_errorMessage = errmsg;
	}


private:
	int m_errorCode;
	tstring m_errorMessage;
};

class CIocpException : public std::exception
{
public:
	explicit CIocpException(tstring const& msg) 
		: m_errorMessage(msg) 
	{

	}

	tstring What() const
	{
		return m_errorMessage;
	}

private:
	tstring m_errorMessage;
};

} // end namespace
#endif // IOCPEXCEPTION_H_2010_09_22_23_03_38