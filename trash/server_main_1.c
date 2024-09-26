#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT "8080"
#define DEFAULT_BUFLEN 2048

static const char *resp = "HTTP/1.1 200 OK\nServer: DDoS Detection Server\nContent-Length: 47\nContent-Type: text/html\nConnection: Closed\n\n<html><body><h1>Hello, World!</h1></body></html>";

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
	if (result == INVALID_SOCKET){
		printf("bind() failed: %d\n", WSAGetLastError());
		freeaddrinfo(address);
		closesocket(server_socket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(address);

	return server_socket;
}

static inline void handle_client(SOCKET client_socket, SOCKET server_socket){
	char recvbuf[DEFAULT_BUFLEN];
	int send_res, recv_res;

	do {
		recv_res = recv(client_socket, recvbuf, DEFAULT_BUFLEN, 0);
		if (recv_res > 0){
			printf("Bytes recieved: %d\n\n", recv_res);
			printf("%.*s", recv_res, recvbuf);
			send_res = send(client_socket, resp, strlen(resp), 0);
			printf("Bytes send: %d\n", send_res);
		}
		else if (recv_res == 0){
			printf("Connection closing...");
		}
		else {
			printf("recv() failed: %d\n", WSAGetLastError());
			closesocket(client_socket);
			closesocket(server_socket);
			WSACleanup();
			exit(EXIT_FAILURE);
		}
	} while (recv_res > 0);
}


int main(void){
	SOCKET server_socket, client_socket;
	int result;

	server_socket = server_init();

	while (1){
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

		handle_client(client_socket, server_socket);
	}
}