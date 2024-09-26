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


int handle_get(char *buf, SOCKET client_socket);


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

static inline int handle_client(SOCKET client_socket){
	char buf[BUFLEN];
	int rec = 0;

	while (1){
		rec += recv(client_socket, buf, BUFLEN - rec, 0);

		if (rec < BUFLEN - 1){
			buf[rec] = '\0';
			if (strstr(buf, "\r\n\r\n")){
				if (strncmp(buf, "GET ", 4) == 0){
					return handle_get(buf, client_socket);
				}
				else if (strncmp(buf, "POST ", 5) == 0){
					;
				}
				else {
					;
				}
			}
		}
		else {
			closesocket(client_socket);
			return 1;
		}
	}
}


int main(void){
	SOCKET server_socket, client_socket;
	int result;
	int loop = 1;

	server_socket = server_init();
	printf("DDoS Detection Server starting on port %s...\n", DEFAULT_PORT);

	result = listen(server_socket, SOMAXCONN);
	if (result == INVALID_SOCKET){
		printf("listen() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	while (loop){
		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) {
			printf("accept() failed: %d\n", WSAGetLastError());
			closesocket(server_socket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		printf("Connection estd..\n");
		loop = handle_client(client_socket);
	}
}