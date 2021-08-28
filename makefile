output: client.o server.o
	cc client.o -o client
	cc server.o -o server

client.o: client.c
	cc  -c client.c
	
server.o: server.c
	cc -c server.c
	
clean:
	rm -rf *.o