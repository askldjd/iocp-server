#include "../IocpServer/ExternalLibraries.h"
#include "../IocpServer/IocpServer.h"
#include <conio.h>
#include <fstream>
#include <boost/thread.hpp>
#include <string>
using namespace iocp;

//! @details
//! A small little class that acts like an echoer and reply everything 
//! sent by the connected client. The connected client must be the first 
//! sender.
class CEchoHandler : public CIocpHandler
{
public:

	// simple struct to keep track of statistics per connection
	struct __declspec(align(64)) Statistics
	{
		Statistics() :m_byteActuallySent(0), m_byteTriedToSent(0), m_byteRcv(0) {}
		int64_t m_byteActuallySent;
		int64_t m_byteTriedToSent;
		int64_t m_byteRcv;
	};

	//! @details
	//! Constructor
	//!
	//! @param[in,out] sendGracefulShutdownMessage
	//! Set to true to send a super secret graceful shutdown message to the
	//! connected client.
	//!
	CEchoHandler(bool sendGracefulShutdownMessage)
		: m_sendGracefulShutdownMessage(sendGracefulShutdownMessage)
	{
	}

	//! @details
	//! New connected client. Update the statistics table.
	virtual void OnNewConnection(uint64_t cid, ConnectionInformation const &c)
	{
		// critical section
		{
			mutex::scoped_lock l(m_mutex);
			tcout
				<< "New Connection  " 
				<< std::hex << cid 
				<< std::dec << _T(" from ") 
				<< c.m_remoteHostName << _T(":") << c.m_remotePortNumber
				<< std::endl;

			assert(m_statistics.find(cid) == m_statistics.end());

			m_statistics[cid];
		}
	}

	//! @details
	//! Connected client sent us data. So echo it back to the client.
	virtual void OnReceiveData(uint64_t cid, std::vector<uint8_t> const &data)
	{
		// critical section
		{
			mutex::scoped_lock l(m_mutex);

			m_statistics[cid].m_byteRcv+= data.size();
			m_statistics[cid].m_byteTriedToSent += data.size();
		}

		// Echo data back to the connected client
		std::vector<uint8_t> d(data);
		GetIocpServer().Send(cid, d);
	}

	//! @details
	//! IOCP notifies that certain data are sent. So keep track of what
	//! the server has sent, vs. how much we actually wanted to send.
	virtual void OnSentData(uint64_t cid, uint64_t byteTransferred)
	{
		// critical section
		{
			mutex::scoped_lock l(m_mutex);
			m_statistics[cid].m_byteActuallySent += byteTransferred;
		}
	}

	//! @details
	//! Client initiated a disconnect. When this is received, the TCP 
	//! connection is half-closed.
	virtual void OnClientDisconnect(uint64_t cid, int32_t e)
	{
		// critical section
		{
			mutex::scoped_lock l(m_mutex);
			std::cout 
				<< "client disconnected " 
				<< std::hex << cid 
				<< std::endl;
		}

		try
		{
			if(true == m_sendGracefulShutdownMessage)
			{

				// For fun, let's send two final message to the client to verify
				// that the server (and client) is handling graceful shutdown 
				// correctly.
				char *finalMessage1 = "All your base ";
				char *finalMessage2 = "belongs to askldjd";

				// send final message 1
				GetIocpServer().Send(
					cid, 
					std::vector<uint8_t> (
						finalMessage1, 
						finalMessage1+strlen(finalMessage1))
					);

				// send final message 2
				GetIocpServer().Send(
					cid, 
					std::vector<uint8_t> (
						finalMessage2, 
						finalMessage2+strlen(finalMessage2))
					);
			}

			// Close the other half of the socket to notify the client
			// that the server also have nothing left to send.
			GetIocpServer().Shutdown(cid, SD_SEND);

			// Finally, fully close the connection.
			GetIocpServer().Disconnect(cid);
		}
		catch (CIocpException const &e)
		{
			{
				mutex::scoped_lock l(m_mutex);
				std::cout << e.what()<< std::endl;
			}

			// Something went wrong. The client might have done an abortive
			// shutdown on the server. So fully disconnect.
			GetIocpServer().Disconnect(cid);
		}
		catch (CWin32Exception const &e)
		{
			{
				mutex::scoped_lock l(m_mutex);
				std::cout << e.what()<< std::endl;
			}

			// Something went wrong. The client might have done an abortive
			// shutdown on the server. So fully disconnect.
			GetIocpServer().Disconnect(cid);
		}

	}

	//! @details
	//! The connection is fully closed. So print a summary of the session
	//! erase the statistics.
	virtual void OnDisconnect(uint64_t cid, int32_t)
	{
		{
			mutex::scoped_lock l(m_mutex);
			std::cout 
				<< "Disconnected " 
				<< std::hex << cid 
				<< std::endl;

			std::cout << std::dec << std::endl;
			std::cout << "Tried Sent : " << m_statistics[cid].m_byteTriedToSent<< std::endl;
			std::cout << "Actually Sent : " << m_statistics[cid].m_byteActuallySent << std::endl;
			std::cout << "Received : " << m_statistics[cid].m_byteRcv<< std::endl;
			std::cout << std::dec << std::endl;
			m_statistics.erase(cid);
		}
	}

	//! @details
	//! The server has fully closed
	virtual void OnServerClose(int32_t)
	{
		// Lock is unnecessary here. This callback is actually in the user's
		// thread context.
		std::cout << "Server Closed " << std::endl;
	}

private:
	boost::mutex m_mutex;
	std::map<uint64_t, Statistics> m_statistics;

	bool m_sendGracefulShutdownMessage;
};
