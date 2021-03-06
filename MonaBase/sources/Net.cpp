/*
This file is a part of MonaSolutions Copyright 2017
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or
modify it under the terms of the the Mozilla Public License v2.0.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License v. 2.0 received along this program for more
details (or else see http://mozilla.org/MPL/2.0/).

*/

#include "Mona/Net.h"
#if !defined(_WIN32)
#include <sys/resource.h>
#endif

using namespace std;

namespace Mona {

Net Net::_Net;

const char* Net::ErrorToMessage(int error) {
	// To fix target where NET_EAGAIN!=NET_EWOULDBLOCK
	if (error == NET_EAGAIN)
		error = NET_EWOULDBLOCK;
	switch (error) {
		case NET_ESYSNOTREADY: return "Net subsystem not ready";
		case NET_ENOTINIT: return "Net subsystem not initialized";
		case NET_EINTR: return "Interrupted";
		case NET_EACCES: return "Permission denied";
		case NET_EFAULT: return "Bad address parameter";
		case NET_EINVAL: return "Invalid argument";
		case NET_EMFILE: return "Too many open sockets";
		case NET_EWOULDBLOCK: return "Operation would block";
		case NET_EINPROGRESS: return "Operation now in progress";
		case NET_EALREADY: return "Operation already in progress";
		case NET_ENOTSOCK: return "Socket operation attempted on a non-socket or uninitialized socket";
		case NET_EDESTADDRREQ: return "Destination address required";
		case NET_EMSGSIZE: return "Message too long";
		case NET_EPROTOTYPE: return "Wrong protocol type";
		case NET_ENOPROTOOPT: return "Protocol not available";
		case NET_EPROTONOSUPPORT: return "Protocol not supported";
		case NET_ESOCKTNOSUPPORT: return "Socket type not supported";
		case NET_ENOTSUP: return "Operation not supported";
		case NET_EPFNOSUPPORT: return "Protocol family not supported";
		case NET_EAFNOSUPPORT: return "Address family not supported";
		case NET_EADDRINUSE: return "Address already in use";
		case NET_EADDRNOTAVAIL: return "Cannot assign requested address";
		case NET_ENETDOWN: return "Network is down";
		case NET_ENETUNREACH: return "Network is unreachable";
		case NET_ENETRESET: return "Network dropped connection on reset";
		case NET_ECONNABORTED: return "Connection aborted";
		case NET_ECONNRESET: return "Connection reseted";
		case NET_ENOBUFS: return "No buffer space available";
		case NET_EISCONN: return "Socket is already connected";
		case NET_ENOTCONN: return "Socket is not connected";
		case NET_ESHUTDOWN: return "Cannot read/write after socket shutdown";
		case NET_ETIMEDOUT: return "Timeout";
		case NET_ECONNREFUSED: return "Connection refused";
		case NET_EHOSTDOWN: return "Host is down";
		case NET_EHOSTUNREACH: return "No route to host";
		case NET_VERNOTSUPPORTED: return "Net subsystem version required not available";
		case NET_EPROCLIM: return "Net subsystem maximum load reached";
#if !defined(_WIN32)
		case EPIPE: return "Broken pipe";
#endif
		default: break;
	}
	return "I/O error";
}


Net::Net() {
#if defined(_WIN32)
	WORD    version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0)
		FATAL_ERROR("Impossible to initialize network (version ", version, "), ", Net::LastErrorMessage());
#else
	// remove ulimit!
	struct rlimit limit;
	limit.rlim_cur = RLIM_INFINITY;
	limit.rlim_max = RLIM_INFINITY;
	if (setrlimit(RLIMIT_NOFILE, &limit) != 0 && getrlimit(RLIMIT_NOFILE, &limit) == 0) {
		limit.rlim_cur = limit.rlim_max;
		setrlimit(RLIMIT_NOFILE, &limit);
	}
#endif
	// Get Default Recv/Send buffer size, connect before because on some targets environment it's required (otherwise SO_RCVBUF/SO_SNDBUF returns 0)
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(12345);
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	NET_SOCKET sockfd;;
	if ((sockfd = ::socket(AF_INET, SOCK_DGRAM, 0)) == NET_INVALID_SOCKET || ::connect(sockfd, (const sockaddr*)&address, sizeof(address)))
		FATAL_ERROR("Impossible to initialize socket system, ", Net::LastErrorMessage());
	int size;
	NET_SOCKLEN length(sizeof(size));
	if (::getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char*>(&size), &length) == -1)
		FATAL_ERROR("Impossible to initialize socket receiving buffer size, ", Net::LastErrorMessage());
	_recvBufferSize = size;
	if (::getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char*>(&size), &length) == -1)
		FATAL_ERROR("Impossible to initialize socket sending buffer size, ", Net::LastErrorMessage());
	_sendBufferSize = size;
	NET_CLOSESOCKET(sockfd);
}

Net::~Net() {
#if defined(_WIN32)
	WSACleanup();
#endif
}


} // namespace Mona
