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
	char buf[BUFLEN + 1];
	SOCKET socket;
	int rcvd;
	struct cinfo_t *next;
};


void handle_get(char *req, SOCKET client_socket);
void handle_post(char *req, SOCKET client_socket);
void send_response(const char *http_code, const char *style, const char *msg, SOCKET client_socket);


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


static inline void destroy_clients(struct cinfo_t *head, fd_set *read_master){
	struct cinfo_t *next;

	while (head){
		next = head->next;
		FD_CLR(head->socket, read_master);
		closesocket(head->socket);
		free(head);
		head = next;
	}
}


static inline void destroy_client(struct cinfo_t **head, struct cinfo_t *client){
	struct cinfo_t *prev;

	closesocket(client->socket);
	if (client == *head){
		*head = client->next;
	}
	else {
		prev = *head;
		while (prev->next != client){
			prev = prev->next;
		}
		prev->next = client->next;
	}
	free(client);
}


static inline void create_client(struct cinfo_t **head, SOCKET socket){
	struct cinfo_t *client;

	client = (struct cinfo_t *)calloc(1, sizeof(struct cinfo_t));
	client->socket = socket;
	client->next = *head;
	*head = client;
}


static inline void handle_client(struct cinfo_t **head, struct cinfo_t *client, fd_set *read_master, SOCKET server_socket){
	int rcvd;

	rcvd = recv(client->socket, client->buf + client->rcvd, BUFLEN - client->rcvd, 0);
	if (rcvd <= 0 || (rcvd + client->rcvd) >= BUFLEN){
		FD_CLR(client->socket, read_master);
		destroy_client(head, client);
	}
	else {
		client->rcvd += rcvd;
		client->buf[client->rcvd] = '\0';
		if (strstr(client->buf, "\r\n\r\n")){
			if (strncmp(client->buf, "GET ", 4) == 0){
				if (strncmp(client->buf, "GET /PASS=MANGO", 15) == 0){
					printf("Shutting down server...");
					destroy_clients(*head, read_master);
					closesocket(server_socket);
					WSACleanup();
					exit(EXIT_SUCCESS);
				}
				else {
					handle_get(client->buf, client->socket);
				}
			}
			else if (strncmp(client->buf, "POST ", 5) == 0){
				handle_post(client->buf, client->socket);
			}
			else {
				send_response("200 OK", "error", "Unsupported HTTP Method", client->socket);
			}
		}
	}
}


int main(void){
	SOCKET server_socket, client_socket, max_socket;
	int result;
	fd_set read_master, read_copy;
	struct cinfo_t *head = NULL, *client, *next;

	server_socket = server_init();
	printf("DDoS Detection Server starting on port %s...\n", DEFAULT_PORT);

	result = listen(server_socket, SOMAXCONN);
	if (INVALID_SOCKET == result){
		printf("listen() failed: %d\n", WSAGetLastError());
		closesocket(server_socket);
		WSACleanup();
		return EXIT_FAILURE;
	}

	FD_ZERO(&read_master);
	FD_SET(server_socket, &read_master);
	max_socket = server_socket;

	while (1){
		read_copy = read_master;
		select(max_socket + 1, &read_copy, NULL, NULL, NULL);

		if (FD_ISSET(server_socket, &read_copy)){
			client_socket = accept(server_socket, NULL, NULL);
			if (INVALID_SOCKET == client_socket){
				printf("accept() failed: %d\n", WSAGetLastError());
				closesocket(server_socket);
				destroy_clients(head, &read_master);
				WSACleanup();
				return EXIT_FAILURE;
			}
			else {
				create_client(&head, client_socket);
				FD_SET(client_socket, &read_master);
				if (client_socket > max_socket){
					max_socket = client_socket;
				}
			}
		}

		client = head;
		while (client){
			next = client->next;
			if (FD_ISSET(client->socket, &read_copy)){
				handle_client(&head, client, &read_master, server_socket);
			}
			client = next;
		}
	}
}