#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>


#define BUFLEN 6 * 1024


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


static inline void send_header(const char *http_code, const char *file_type, int file_size, SOCKET client_socket){
	char buf[400];
	int sent_b, sz;
	const char *header = "HTTP/1.1 %s\r\nServer: DDoS Detection Server\r\nLanguage: C\r\nProgrammer: Uday Raj Singh\r\nGithub: https://github.com/udayRajSingh288/\r\nLinkedIn: https://www.linkedin.com/in/uday-raj-singh-367537224/\r\nConnection: close\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n";

	sprintf(buf, header, http_code, file_size, file_type);
	sz = strlen(buf);
	sent_b = send(client_socket, buf, sz, 0);
	while (sent_b < sz){
		sent_b += send(client_socket, buf + sent_b, sz - sent_b, 0);
	}
}


void send_response(const char *http_code, const char *style, const char *msg, SOCKET client_socket){
	char buf[300];
	int sent_b, sz;
	const char *page = "<html><head><link rel = \"stylesheet\" href = \"styles.css\"><title>DDoS Detection Service</title><head><body><div class = \"message-box %s\"><p>%s<p></div></body></html>";

	sprintf(buf, page, style, msg);
	sz = strlen(buf);
	send_header(http_code, "text/html", sz, client_socket);
	sent_b = send(client_socket, buf, strlen(buf), 0);
	while (sent_b < sz){
		sent_b += send(client_socket, buf + sent_b, sz - sent_b, 0);
	}
}


void handle_get(char *req, SOCKET client_socket){
	char buf[BUFLEN];
	char file_name[200];
	char *end, *type;
	FILE *fp;
	int fsize, sz, sent_b;

	req += 4;
	end = strstr(req, " ");
	*end = '\0';

	if (req_valid(req)){
		get_file_name(req, file_name);
		fp = fopen(file_name, "rb");
		if (fp){
			fseek(fp, 0, SEEK_END);
			fsize = ftell(fp);
			rewind(fp);
			type = get_file_type(file_name);
			send_header("200 OK", type, fsize, client_socket);
			fsize = fread(buf, 1, BUFLEN, fp);
			while (fsize){
				sent_b = send(client_socket, buf, fsize, 0);
				while (sent_b < fsize){
					sent_b += send(client_socket, buf + sent_b, fsize - sent_b, 0);
				}
				fsize = fread(buf, 1, BUFLEN, fp);
			}
			fclose(fp);
		}
		else {
			return send_response("404 Not Found", "error", "Resource Not Found", client_socket);
		}
	}
	else {
		return send_response("400 Bad Request", "error", "Malicious Request", client_socket);
	}
}