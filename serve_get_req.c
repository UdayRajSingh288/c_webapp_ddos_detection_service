#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>


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
	const char *last_dot = strrchr(file_name, '.');
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

void send_header(const char *http_code, const char *file_type, const char *file_len, SOCKET client_socket){
	char buf[200];
	const char *header = "HTTP/1.1 %s\nServer: DDoS Detection Server\nConnection: keep-alive\nContent-Type: %s\nContent-Length: %s\n\n";

	sprintf(buf, header, http_code, file_type, file_len);
	send(client_socket, buf, strlen(buf), 0);
}

int send_response(const char *http_code, const char *msg, SOCKET client_socket){
	char buf[128];
	char num[10];
	const char *page = "<html><head><title>DDoS Detection Service</title><head><body><h1>%s<h1></body></html>";

	sprintf(buf, page, msg);
	itoa(strlen(buf), num, 10);
	send_header(http_code, "text/html", num, client_socket);
	send(client_socket, buf, strlen(buf), 0);
	return 1;
}

int serve_get(char *req, SOCKET client_socket){
	char buf[10240];
	char file_name[128];
	char num[10];
	char *read_ptr, *end, *type;
	FILE *fp;
	int fsize;

	read_ptr = req + 4;
	end = strstr(read_ptr, " ");
	if (NULL == end){
		return send_response("400 Bad Request", "Invalid Request Format", client_socket);
	}
	*end = '\0';

	if (req_valid(read_ptr)){
		get_file_name(read_ptr, file_name);
		fp = fopen(file_name, "r");
		if (fp){
			fseek(fp, 0, SEEK_END);
			fsize = ftell(fp);
			rewind(fp);
			type = get_file_type(file_name);
			itoa(fsize, num, 10);
			send_header("200 OK", type, num, client_socket);
			fsize = fread(buf, 1, 10240, fp);
			while (fsize){
				send(client_socket, buf, fsize, 0);
				fsize = fread(buf, 1, 1024, fp);
			}
			fclose(fp);
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