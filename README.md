# IOCP Server Library

A reusable TCP/IP server library that utilizes Windows I/O Completion Port (IOCP).

It is open source, licensed under Boost Software License 1.0.

See this [post](http://askldjd.com/2010/10/26/iocp-server-library/) for its introduction.

## Requirement:

OS: Window XP, Window 7.

Compiler: Visual Studio 2008 SP1

Boost: Boost 1.40-1.45

Build Type: ANSI, Unicode.

## Latest version

### IOCP Server 1.2

Fixed a bug under Unicode build where the remote address is not NULL terminated.
Older versions

### IOCP Server 1.1

Fixed a bug where the IOCP server stops accepting connection when setsockopt() fails.
Removed an unnecessary event per Len Holgate's suggestion.

### IOCPServer 1.0

Creation
