CC = g++
CFLAGS = -Wall -std=c++11
TARGET = client server
	
normal: clean $(TARGET)

manpages: client.man server.man
	groff -man -t client.man server.man > client_server.pdf

client: client.o tands.o
	$(CC) $(CFLAGS) client.o tands.o -o client

server: server.o tands.o
	$(CC) $(CFLAGS) server.o tands.o -o server

client.o: client.cpp client.h
	$(CC) $(CFLAGS) -c client.cpp -o client.o

server.o: server.cpp server.h
	$(CC) $(CFLAGS) -c server.cpp -o server.o

tands.o: tands.c tands.h
	$(CC) $(CFLAGS) -c tands.c -o tands.o

clean:
	rm -f *~out.* *.o