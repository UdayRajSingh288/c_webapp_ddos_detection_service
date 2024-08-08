#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFLEN 6 * 1024
#define FILEPATHLEN 256


static inline int req_valid(const char *req){
	if (strstr(req, "..")){
		return 0;
	}
	if (strstr(req, "CON") || strstr(req, "PRN") || strstr(req, "AUX") || strstr(req, "NUL") || strstr(req, "COM") || strstr(req, "LPT")){
		return 0;
	}
	return 1;
}

static inline void get_file_name(const char *req, char *file_name){
	if (strcmp(req, "/") == 0){
		sprintf(file_name, "public/index.html");
	}
	else {
		sprintf(file_name, "public%s", req);
	}
}

static inline char *get_file_type(const char *file_name){
	const char *last_dot;

	last_dot = strrchr(file_name, '.');
	if (last_dot) {
		if (strcmp(last_dot, ".css") == 0) return "text/css";
		if (strcmp(last_dot, ".csv") == 0) return "text/csv";
		if (strcmp(last_dot, ".gif") == 0) return "image/gif";
		if (strcmp(last_dot, ".htm") == 0) return "text/html";
		if (strcmp(last_dot, ".html") == 0) return "text/html";
		if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
		if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
		if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
		if (strcmp(last_dot, ".js") == 0) return "application/javascript";
		if (strcmp(last_dot, ".json") == 0) return "application/json";
		if (strcmp(last_dot, ".png") == 0) return "image/png";
		if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
		if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
		if (strcmp(last_dot, ".txt") == 0) return "text/plain";
	}
	return "application/octet-stream";
}

void send_header(const char *http_code, const char *file_type, int file_len, SOCKET client_socket){
	char buf[200];
	const char *header = "HTTP/1.1 %s\r\nServer: DDoS Detection Server\r\nConnection: close\r\nContent-type: %s\r\nContent-Length: %d\r\n\r\n";

	sprintf(buf, header, http_code, file_type, file_len);
	send(client_socket, buf, strlen(buf), 0);
}

int send_response(const char *http_code, const char *msg, SOCKET client_socket){
	char buf[128];
	const char *page = "<html><head><title>DDoS Detection Service</title><head><body><h1>%s<h1></body></html>";

	sprintf(buf, page, msg);
	send_header(http_code, "text/html", strlen(buf), client_socket);
	send(client_socket, buf, strlen(buf), 0);
	closesocket(client_socket);
	return 1;
}

int handle_get(char *req, SOCKET client_socket){
	char buf[BUFLEN];
	char file_name[FILEPATHLEN];
	char *end, *type;
	FILE *fp;
	int fsize;

	req += 4;
	end = strstr(req, " ");
	*end = '\0';

	if (req_valid(req)){
		get_file_name(req, file_name);
		printf("Serving %s\n", file_name);
		fp = fopen(file_name, "r");
		if (fp){
			fseek(fp, 0, SEEK_END);
			fsize = ftell(fp);
			rewind(fp);
			type = get_file_type(file_name);
			send_header("200 OK", type, fsize, client_socket);
			fsize = fread(buf, 1, BUFLEN, fp);
			while (fsize){
				send(client_socket, buf, fsize, 0);
				fsize = fread(buf, 1, BUFLEN, fp);
			}
			fclose(fp);
			printf("%s served\n", file_name);
			closesocket(client_socket);
			printf("Connection closed...\n");
			return 1;
		}
		else {
			return send_response("404 Not Found", "Resource Not Found", client_socket);
		}
	}
	else {
		return send_response("400 Bad Request", "Malicious Request", client_socket);
	}
}