CC = gcc


%.o: %.c  
	$(CC) -c -o $@ $<

all: t3_server t3_client

t3_server: t3_server.o
	gcc -pthread  -o t3_server t3_server.o 


t3_client: t3_client.o
	gcc -o t3_client t3_client.o 


clean:
	rm -f *.o t3_server
	rm -f *.o t3_client
