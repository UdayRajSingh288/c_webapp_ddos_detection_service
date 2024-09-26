#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *DEFAULT_PORT = "8080";
#define BUFLEN 6 * 1024


int serve_get(char *req, SOCKET client_socket);


static inline SOCKET server_init(void){
	WSADATA wsa_data;
	int result;
	struct addrinfo *address = NULL, hints;
	SOCKET server_socket = INVALID_SOCKET;

	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result){
		printf("WSAStartup() failed: %d\n", result);
		exit(EXIT_FAILURE);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &address);
	if (result){
		printf("getaddrinfo() failed: %d\n", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	server_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	if (server_socket == INVALID_SOCKET){
		printf("socket() failed: %d\n", WSAGetLastError());
		freeaddrinfo(address);
		WSACleanup();
		exit(EXIT_FAILURE);
	}


	result = bind(server_socket, address->ai_addr, (int)address->ai_addrlen);
	freeaddrinfo(address);
	if (result == INVALID_SOCKET){
		printf("bind() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	return server_socket;
}

static inline int handle_client(SOCKET client_socket, SOCKET server_socket){
	return 1;
}


int main(void){
	SOCKET server_socket, client_socket;
	int result, run = 1;

	server_socket = server_init();
	printf("DDoS Detection Server starting on port %s...\n", DEFAULT_PORT);

	while (run){
		result = listen(server_socket, SOMAXCONN);
		if (result == INVALID_SOCKET){
			printf("listen() failed: %d\n", WSAGetLastError());
			closesocket(server_socket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) {
			printf("accept() failed: %d\n", WSAGetLastError());
			closesocket(server_socket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		run = handle_client(client_socket, server_socket);
	}
}