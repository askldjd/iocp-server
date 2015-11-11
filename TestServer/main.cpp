#include <tchar.h>
#include "EchoHandler.h"
using namespace iocp;

void RunNormalServer(bool sendGracefulShutdownMessage)
{
	boost::shared_ptr<CEchoHandler> h(
		new CEchoHandler(sendGracefulShutdownMessage)
		);

	//! Simple echo server that receives and sends at normal rate.
	CIocpServer *echoServer = new CIocpServer(50000,h);

	// enter something in to exit
	char c;
	std::cin >> c;

	delete echoServer;
}

void RunSlowReader(bool sendGracefulShutdownMessage)
{
	boost::shared_ptr<CEchoHandler> h(
		new CEchoHandler(sendGracefulShutdownMessage)
		);

	//! Stress test server that receives with 1 byte buffer. This will
	//! maximize the number of IO Completion port traffic.
	CIocpServer *echoServer = 
		new CIocpServer(50000,h, INADDR_ANY, 1);

	// enter something in to exit
	char c;
	std::cin >> c;

	delete echoServer;
}

int main()
{
	//RunSlowReader(false);
	RunNormalServer(false);

	return 0;
}