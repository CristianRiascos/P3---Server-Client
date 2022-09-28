#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>
#include <stdlib.h>	// EXIT_FAILURE, system clear


int main( int argc, char *argv[] ) 
{
	int sock, terminate_client;
	struct sockaddr_in server;
	char message[100], server_reply[2000];
	
	system( "clear" );
	puts( "CLIENT\n\n" );
	// Create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf( "ERROR: Could not create socket client\n" );
	}
	
	server.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	server.sin_family = AF_INET;
	server.sin_port = htons( 8888 );

	// Connect to remote server
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror( "ERROR: Connection to server failed\n" );
		return EXIT_FAILURE;
	}

	puts( "Opciones disponibles:\n" );
	puts( "1. Crear contenedor" );
    puts( "2. Listar contenedor" );
    puts( "3. Detener contenedor" );
    puts( "4. Borrar contenedor" );
	puts( "0. Salir" );

	puts( "\n" );

	// Flag to determine when to finish the program
	terminate_client = 1;
	while( terminate_client )
	{	
		printf( "Enter message: " );
		fgets( message, 1000, stdin );


		// Send some data
		if (send(sock, message, strlen(message)+1, 0) < 0) {
			puts( "ERROR: Message could not be send\n" );
			return EXIT_FAILURE;
		}
		
		// If message = 0, then close the socker and finish de program.
		if( atoi( message ) == 0 ){
			close( sock );
			return EXIT_SUCCESS;
		}

		// Receive a reply from the server
		memset(server_reply, 0, 2000);
		if ( recv(sock, server_reply, 2000, 0 ) < 0 ) {
			puts( "ERROR: Upcoming message failed\n" );
			//break;
		}

		printf( "Server reply: %s\n", server_reply );
		/*
		puts( "Server reply: " );
		puts( server_reply );
		*/
		puts( "\n" );
	}
	
}