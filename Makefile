CC = gcc

LDFLAGS = -lws2_32

TARGET = server_main

$(TARGET): server_main.o handle_GET.o handle_POST.o sqlite3.o
	$(CC) server_main.o handle_GET.o handle_POST.o sqlite3.o -o server_main $(LDFLAGS)

server_main.o: server_main.c
	$(CC) -c server_main.c

handle_GET.o: handle_GET.c
	$(CC) -c handle_GET.c

handle_POST.o: handle_POST.c
	$(CC) -c handle_POST.c

sqlite3.o: sqlite3.c
	$(CC) -c sqlite3.c

clean:
	cmd /C del server_main.o
	cmd /C del handle_GET.o
	cmd /C del handle_POST.o
	cmd /C del sqlite3.o
