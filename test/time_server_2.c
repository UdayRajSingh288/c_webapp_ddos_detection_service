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

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	result = getaddrinfo(NULL, "8080", &hints, &address);

	server_socket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	result = bind(server_socket, address->ai_addr, address->ai_addrlen);
	freeaddrinfo(address);

	result = listen(server_socket, 1);

	client_socket = accept(server_socket, NULL, NULL);

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