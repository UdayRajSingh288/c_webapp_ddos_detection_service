#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "sqlite/sqlite3.h"

#define BUFLEN 6 * 1024

static int QUERY_EXEC;

void send_response(const char *http_code, const char *style, const char *msg, SOCKET client_socket);


static inline void register_client(char *req, SOCKET socket){
	const char *red = "HTTP/1.1 302 Found\r\nLocation: /otp.html\r\nSet-Cookie: session_id=%s\r\nPath=/otp; HttpOnly\r\nConnection: keep-alive\r\n\r\n";
	const char *ins = "insert into tmp values('%s', '%s', '%s', %d);";

	char buf[400];
	char sessid[100];
	char stmt[500];
	time_t tm;
	char *email, *pswd, *end;
	int otp;
	sqlite3 *db;

	tm = time(NULL);
	sprintf(sessid, "DDoS%d", tm);
	sprintf(buf, red, sessid);
	send(socket, buf, strlen(buf), 0);

	email = strstr(req, "email=");
	email += 6;
	pswd = strstr(email, "pswd1=");
	pswd += 6;
	end = strstr(email, "\r\n");
	*end = '\0';
	end = strstr(pswd, "\r\n");
	*end = '\0';

	srand(tm);
	otp = (tm + rand()) % 1000000;
	printf("OTP: %d\n", otp);

	sprintf(stmt, ins, sessid, email, pswd, otp);

	sqlite3_open("ddos_db", &db);
	sqlite3_exec(db, stmt, NULL, NULL, NULL);
	sqlite3_close(db);
}

static inline int callback(void *neg, int n, char **row, char **cols){
	QUERY_EXEC = 1;
	return 0;
}

static inline void validate_otp(char *req, SOCKET socket){
	const char *srch = "select * from tmp where sessid = '%s' and otp = %s;";
	const char *del_tmp = "delete from tmp where sessid = '%s';";
	const char *srch_client = "select * from client where email = (select email from tmp where sessid = '%s');";
	const char *ins = "insert into client select sessid, email, pswd from tmp where sessid = '%s';";
	char *sessid, *otp, *end;
	sqlite3 *db;
	char stmt[500];

	sessid = strstr(req, "Cookie: session_id=");
	if (sessid){
		sessid += 19;
	}
	else {
		send_response("200 OK", "error", "Session does not exist!", socket);
		return;
	}
	otp = strstr(req, "otp=");
	otp += 4;
	end = strstr(sessid, "\r\n");
	*end = '\0';

	printf("Session ID: %s\nOTP: %s\n", sessid, otp);

	sqlite3_open("ddos_db", &db);
	sprintf(stmt, srch, sessid, otp);
	QUERY_EXEC = 0;
	sqlite3_exec(db, stmt, callback, NULL, NULL);
	if (QUERY_EXEC){
		sprintf(stmt, srch_client, sessid);
		QUERY_EXEC = 0;
		sqlite3_exec(db, stmt, callback, NULL, NULL);
		if (QUERY_EXEC){
			send_response("200 OK", "error", "Email already used", socket);
		}
		else {
			sprintf(stmt, ins, sessid);
			sqlite3_exec(db, stmt, NULL, NULL, NULL);
			send_response("200 OK", "success", "Client created successfully", socket);
		}
		sprintf(stmt, del_tmp, sessid);
		sqlite3_exec(db, stmt, NULL, NULL, NULL);
	}
	else {
		send_response("200 OK", "error", "Invalid Session or incorrect otp!", socket);
	}
	sqlite3_close(db);
}

static inline void delete_client(char *req, SOCKET socket){
	const char *del = "delete from client where email = '%s' and pswd = '%s';";
	char *email, *pswd, *end;
	char stmt[500];
	sqlite3 *db;

	email = strstr(req, "email=");
	email += 6;
	pswd = strstr(email, "pswd=");
	pswd += 5;
	end = strstr(email, "\r\n");
	*end = '\0';
	end = strstr(pswd, "\r\n");
	*end = '\0';
	sprintf(stmt, del, email, pswd);
	sqlite3_open("ddos_db", &db);
	sqlite3_exec(db, stmt, NULL, NULL, NULL);
	if (sqlite3_changes(db)){
		send_response("200 OK", "success", "Client deleted successfully", socket);
	}
	else {
		send_response("200 OK", "error", "Email or password incorrect", socket);
	}
	sqlite3_close(db);
}

void handle_post(char *req, SOCKET socket){
	req += 5;
	if (strncmp(req, "/register ", 10) == 0){
		register_client(req, socket);
	}
	else if (strncmp(req, "/cancel ", 8) == 0){
		delete_client(req, socket);
	}
	else if (strncmp(req, "/otp ", 5) == 0){
		validate_otp(req, socket);
	}
	else if (strncmp(req, "/ntraffic ", 10) == 0){
		send_response("200 OK", "info", "Work in progress...", socket);
	}
	else {
		send_response("400 Bad Request", "error", "Incorrect POST endpoint!", socket);
	}
}