#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <pthread.h> // pthread
#include <stdlib.h> // EXIT_FAILURE, system clear
#include <sys/wait.h> // Wait

#define SIZE_MESSAGE 100	// Allowed char
#define LENGHT_MESSAGE 25 	// Lenght of the allowed char 


int parseCommand( char *line, char parse[SIZE_MESSAGE][LENGHT_MESSAGE] )
{
    int iterator, j, numberWords;

    numberWords = 0;
    j = 0;

    for( iterator=0; iterator<=strlen(line);iterator++ )
    {
        if( line[iterator] != ' ' )
            parse[numberWords][j++] = line[iterator];
        else{
            parse[numberWords][j++] = '\0';
            numberWords++;
            j = 0;
        }       
        if( line[iterator] == '\0' )    
            return numberWords;   
    }
}


void runDocker()
{
	pid_t pid = fork();

	if( pid < 0 ){
		perror( "ERROR: " );
		return;
	}
	else if( pid == 0 ){
		execlp( "sudo", "sudo", "/etc/init.d/docker" , "start", NULL );
	}
	else{
		wait( NULL );
	}
	return;

}


void *createContainer( void *arg )
{
	char *args, names_create[SIZE_MESSAGE][LENGHT_MESSAGE];
	pid_t pid;

	pid = fork();

	// Parse the client message by spaces
	args = (char*)arg;
	parseCommand( args, names_create );

	if( pid < 0 ){
		perror( "ERROR: " );
		return 0;
	}

	/* Ask if this is a good practice
		name[1] = name of the container
		name[2] = image
	*/ 
	else if( pid == 0 ){
		execlp( "sudo", "sudo", "docker", "run", "-di", "--name", names_create[1], names_create[2], NULL );
	}

	else{
		wait( NULL );
	}
	
	// Confirmation message for the function
	printf( "\nContainer named %s with the image %s was successfully created\n", names_create[1], names_create[2] );
	
	return 0;
}


void *listContainer( void *arg )
{
	pid_t pid;

	pid = fork();
	puts( "\n" );

	if( pid < 0 ){
		perror( "ERROR: " );
		return 0;
	}
	else if( pid == 0 ){
		execlp( "sudo", "sudo", "docker", "ps", "-a", NULL );
	}
	else{
		wait(NULL);
	}
	return 0;
}


void *stopContainer( void *arg )
{
	char *args, name_stop[SIZE_MESSAGE][LENGHT_MESSAGE];
	pid_t pid;

	pid = fork();

	args = (char*)arg;
	parseCommand( args, name_stop );

	if( pid < 0 ){
		perror( "ERROR: " );
		return 0;
	}
	else if( pid == 0 ){
		// name[1] = 
		execlp( "sudo", "sudo", "docker", "stop", name_stop[1], NULL );
		puts( "Pasé\n" );
	}
	else{
		wait(NULL);
	}
	
	printf( "\nContainer named %s was successfully stopped\n", name_stop[1] );
	return 0;
}


void *deleteContainer( void *arg )
{
	char *args, name_delete[SIZE_MESSAGE][LENGHT_MESSAGE];
	pid_t pid;

	pid = fork();

	args = (char*)arg;
	parseCommand( args, name_delete );

	if( pid < 0 ){
		perror( "ERROR: " );
	}
	else if( pid == 0 ){
		execlp( "sudo", "sudo", "docker", "rm", name_delete[1], NULL );
	}
	else{
		wait(NULL);
	}

	printf( "\nContainer named %s was successfully deleted\n", name_delete[1] );
	return 0;
}


int main( int argc , char *argv[] ) 
{
	int socket_desc, client_sock, c, terminate_server;
	struct sockaddr_in server, client; 
	char client_message[SIZE_MESSAGE], send_message[SIZE_MESSAGE];
	
	system( "clear" );
	puts( "SERVER\n\n" );
	runDocker();

	wait(NULL);

	// Create socket
    // AF_INET (IPv4 protocol) , AF_INET6 (IPv6 protocol) 
    // SOCK_STREAM: TCP(reliable, connection oriented)
    // SOCK_DGRAM: UDP(unreliable, connectionless)
    // Protocol value for Internet Protocol(IP), which is 0
	socket_desc = socket( AF_INET, SOCK_STREAM, 0 );
	if ( socket_desc == -1 ) {
		printf( "ERROR: Socket could not be created\n" );
	}
	
	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );
	
	// Bind the socket to the address and port number specified
	if( bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror( "ERROR: Bind failed\n" );
		return EXIT_FAILURE;
	}
	puts( "Bind done" );
	
	listen( socket_desc, 4 );
	
	// Accept and incoming connection
	puts( "Waiting for incoming connections..." );
	c = sizeof( struct sockaddr_in );
	
	// Accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if( client_sock < 0 ) {
		perror( "ERROR: Connection rejected\n" );
		return EXIT_FAILURE;
	}
	puts( "Connection accepted\n" );


	// Create one thread to each function 
	pthread_t threadToCreate, threadtoList, threadToStop, threadToDelete;
	
	// Flag to determine when to finish the program
	terminate_server = 1;
	while( terminate_server ) 
	{
		// Receive message from the client with the instruction
		if( recv( client_sock, client_message, SIZE_MESSAGE, 0 ) <= 0 ) {
			perror( "ERROR: " ); 		
		}
		
		client_message[strcspn(client_message, "\n")] = 0;

		// Switch to create a thread depending on the client message
		switch( atoi(&client_message[0] )  )
		{

			// Case to create container
			case 1:
				// Create the thread and notifies if there is an error
				if( pthread_create( &threadToCreate, NULL, createContainer, &client_message ) != 0 ){
					perror( "ERROR: " );
				}

				// Waits for the thread to finish it's task
				if( pthread_join( threadToCreate, NULL ) != 0 ){
					perror( "ERROR: " );
				}

				// Send confirmation message back to client
				strcpy( send_message, "The container was successfully created" );
				send( client_sock , send_message , strlen(send_message), 0 );

				break;


			// Case to list containers
			case 2 :
				if( pthread_create( &threadtoList, NULL, listContainer, NULL ) != 0 ){
					perror( "ERROR: " );
				}

				if( pthread_join( threadtoList, NULL ) != 0 ){
					perror( "ERROR: " );
				}

				// Ask if this needs to be send to client (command itself)
				strcpy( send_message, "The list was successfully done" );
				send( client_sock , send_message , strlen(send_message), 0 );

				break;


			// Case to stop container
			case 3:
				if( pthread_create( &threadToStop, NULL, stopContainer, &client_message ) != 0 ){
					perror( "ERROR: " );
				}
				
				if( pthread_join( threadToStop, NULL ) != 0 ){
					perror( "ERROR: " );
				}

				strcpy( send_message, "The container have been successfully stopped" );
				send( client_sock , send_message , strlen(send_message), 0 );

				break;


			// Case to delete container 
			case 4:
				if( pthread_create( &threadToDelete, NULL, deleteContainer, &client_message ) != 0 ){
					perror( "ERROR: " );
				}
				
				if( pthread_join( threadToStop, NULL ) != 0 ){
					perror( "ERROR: " );
				}

				strcpy( send_message, "The container have been successfully deleted" );
				send( client_sock , send_message , strlen(send_message), 0 );

				break;
			

			// Case to search another client if the current one exit
			case 0:
				puts( "Waiting for incoming connections..." );
				client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
				if( client_sock < 0 ) {
					perror( "ERROR: Connection rejected\n" );
					return EXIT_FAILURE;
				}
				puts( "Connection accepted\n" );				
				break;


			// Default case if the number is wrong
			default:
				puts( "Selected number is not currently available. Try again\n" );
		}

    }

	

	return 0;
}