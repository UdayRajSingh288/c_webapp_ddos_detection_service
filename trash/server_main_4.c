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


struct cinfo_t {
	char buf[BUFLEN];
	SOCKET socket;
	int rcvd;
	struct cinfo_t *next;
};


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
	if (INVALID_SOCKET == server_socket){
		printf("socket() failed: %d\n", WSAGetLastError());
		freeaddrinfo(address);
		WSACleanup();
		exit(EXIT_FAILURE);
	}


	result = bind(server_socket, address->ai_addr, (int)address->ai_addrlen);
	freeaddrinfo(address);
	if (INVALID_SOCKET == result){
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


static inline struct cinfo_t *get_client(struct cinfo_t *head, SOCKET client_socket){
	while (head){
		if (head->socket == client_socket){
			return head;
		}
		else {
			head = head->next;
		}
	}
}


int main(void){
	SOCKET server_socket, max_socket, client_socket, i;
	fd_set read_master, read_copy;
	int result;
	int loop = 1;
	struct cinfo_t *head = NULL, *client;
	int rdlen;
	struct cinfo_t *prev, *next;

	server_socket = server_init();
	printf("DDoS Detection Server starting on port %s...\n", DEFAULT_PORT);

	result = listen(server_socket, SOMAXCONN);
	if (INCALID_SOCKET == result){
		printf("listen() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	FD_ZERO(&read_master);
	FD_SET(server_socket, &read_master);
	max_soxket = server_socket;
	while (loop){
		read_copy = read_master;
		result = select(max_socket + 1, &read_copy, NULL, NULL, NULL);
		if (result < 0){
			printf("select() failed: %d\n", WSAGetLastError());
			closesocket(server_socket);
			WSACleanup();
			return EXIT_FAILURE;
		}

		for (i = 1; i <= max_sozket; ++i){
			if (FD_ISSET(i, &read_copy)){
				if (i == server_socket){
					client_socket = accept(server_socket, NULL, NULL);
					if (INVALID_SOCKET == client_socket){
						printf("accept() failed: %d\n", WSAGetLastError());
						closesocket(server_socket);
						WSACleanup();
						return EXIT_FAILURE;
					}
					else {
						client = (struct cinfo_t *)calloc(1, sizeof(struct c_info));
						client->socket = client_socket;
						client->next = head;
						head = client;
						if (max_socket < client_socket){
							max_socket = client_socket;
						}
					}
				}
				else {
					client = get_client(head, i);
					if (manage_client(client)){
						prev = NULL;
						next = head;
						while (next){
							if (next == client){
								break;
							}
							else {
								prev = next;
								next = next->next;
							}
						}
						closesocket(client->socket);
						FD_CLR(client->socket, &read_master);
						if (next == head){
							head = next->next;
						}
						else {
							prev->next = next->next;
						}
						free(next);
					}
				}
			}
		}
	}
}