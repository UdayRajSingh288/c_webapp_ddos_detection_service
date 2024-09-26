#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int main(int argc, char **argv){
	WSADATA wsa_data;
	int result;
	struct addrinfo hints, *server_address;
	char address_buffer[128];
	char service_buffer[128];

	if (argc < 3){
		printf("usage: tcp_client <hostname> <port>\n");
		return EXIT_FAILURE;
	}

	result = WSStartup(MAKEWORD(2, 2), &wsa_data);
	if (result){
		printf("WSAStatrup() failed: %d\n", result);
		return EXIT_FAILURE;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	result = getaddrinfo(argv[1], argv[2], &hints, &server_address);
	if (result){
		printf("getaddrinfo() failed: %d\n", WSAGetLastError());
		WSACleanup();
		return EXIT_FAILURE;
	}

	printf("Remote address is: ");
	getnameinfo(server_address->ai_addr, server_address->ai_addrlen, server_address->ai_protocol);
	
}