#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include <string.h>

#define BUFLEN 1024

static const char *resp = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-type: text/plain\r\n\r\nLocal time is: %s";

int main(void){
	WSADATA wsa_data;
	int result;
	struct addrinfo hints, *address;
	SOCKET server_socket, client_socket;
	char buf[BUFLEN];
	int recvln, sendln;
	time_t timer;

	result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result){
		printf("WSAStartup() failed: %d\n", result);
		return EXIT_FAILURE;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, "8080", &hints, &address);
	if (result){
		printf("getaddrinfo() failed: %d\n", WSAGetLastError());
		WSACleanup();
		return EXIT_FAILURE;
	}

	server_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
	if (INVALID_SOCKET == server_socket){
		printf("socket() failed: %d\n", WSAGetLastError());
		freeaddrinfo(address);
		WSACleanup();
		return EXIT_FAILURE;
	}

	result = bind(server_socket, address->ai_addr, address->ai_addrlen);
	freeaddrinfo(address);
	if (INVALID_SOCKET == result){
		printf("bind() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	result = listen(server_socket, 1);
	if (INVALID_SOCKET == result){
		printf("listen() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return EXIT_SUCCESS;
	}

	client_socket = accept(server_socket, NULL, NULL);
	if (INVALID_SOCKET == client_socket){
		printf("accept() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return EXIT_SUCCESS;
	}

	recvln = recv(client_socket, buf, BUFLEN, 0);
	printf("Recieved %d bytes\n", recvln);

	time(&timer);
	sprintf(buf, resp, ctime(&timer));
	sendln = send(client_socket, buf, strlen(buf), 0);
	printf("Sent %d bytes\n", sendln);
	closesocket(client_socket);
	closesocket(server_socket);
	WSACleanup();
	return EXIT_SUCCESS;
}