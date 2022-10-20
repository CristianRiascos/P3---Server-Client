#include<stdio.h>	
#include<string.h>	//strlen
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include <pthread.h> // pthread, mutex
#include <stdlib.h> // EXIT_FAILURE, system clear
#include <sys/wait.h> // Wait

#define SIZE_MESSAGE 100	// Allowed char
#define LENGHT_MESSAGE 25 	// Lenght of the allowed char 
#define FILENAME "containers.txt" 

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
	0 = Everything is correct
	-1 = An error has ocurred
	(special for deleteContainer) -2 = The container is not stopped
*/
int functResult;

// Function to parse a char by spaces
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


// Function to run docker
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


// Function to create the file "containers.txt" at the begining of the program 
void createFile()
{
	FILE *fp;
	fp = fopen( FILENAME, "w" );
	fclose(fp);
	return;
}


// Function to search a container name in a txt
int searchWord( char *word )
{
	FILE *fp;
	int numberLine = 1;
	char line[150], parseLine[SIZE_MESSAGE][LENGHT_MESSAGE];

	// Locking while reading the file
	pthread_mutex_lock( &mutex );
	fp = fopen( FILENAME, "r" );
	
	// Reads each line until there is nothing else to read
	while( fgets( line, 150, fp ) != NULL ) 
	{
		// Parse each line by spaces
		parseCommand( line, parseLine );

		// Compares the name of the container with the word to search
		if( ( strcmp( parseLine[3], word ) ) == 0 ) {
			pthread_mutex_unlock( &mutex );
			return numberLine;
		}
		numberLine++;
	}
	fclose( fp );

	// Unlocking when finished
	pthread_mutex_unlock( &mutex );
	
	return -1;
}  


// Function to add container info in a txt
void addContainerWord( char *name, char *image )
{
	FILE *fp;
	// Lock while printing in the file
	pthread_mutex_lock( &mutex );

	fp = fopen( FILENAME, "a" );
	fprintf( fp, "status: running name: %s image: %s\n", name, image );
	fclose(fp);

	pthread_mutex_unlock( &mutex );
	
	return;
}


// Function to change status of a container in a txt 
void changeStatus( char *name )
{
	FILE *fpOld, *fpNew;
	char line[150], parseLine[SIZE_MESSAGE][LENGHT_MESSAGE];

	// Locking while reading the file
	pthread_mutex_lock( &mutex );

	fpOld = fopen( FILENAME, "r" );

	// Creates a new temporal file to store the old file and line to replace
	fpNew = fopen( "temporal.txt", "w" );

	// Reads each line until there is nothing else to read
	while( fgets( line, 150, fpOld ) != NULL ) 
	{
		// Parse each line by spaces
		parseCommand( line, parseLine );

		// If the container's name is found, change the status to stopped
		if( ( strcmp( parseLine[3], name ) ) == 0 ) {
			fprintf( fpNew, "status: stopped name: %s image: %s", name, parseLine[5] );
			functResult = 0;	// == 0 because the container's status was successfully stopped
		}

		else{	// If the container's name is not the chosen by the user then copies the line to the new file
			fprintf( fpNew, "%s", line );
		}
	}

	fclose( fpOld );
	fclose( fpNew );
	remove( FILENAME );	// Deletes the old containers file
	rename( "temporal.txt", FILENAME );	// Changes the name of the old file to containers.txt with the line changed


	// Unlocking when finished
	pthread_mutex_unlock( &mutex );
	
	return;
}


// Function to delete the container info in a txt
void deleteContainerFile( char *name )
{
	FILE *fpOld, *fpNew;
	char line[150], parseLine[SIZE_MESSAGE][LENGHT_MESSAGE];

	// Locking while reading the file
	pthread_mutex_lock( &mutex );

	fpOld = fopen( FILENAME, "r" );

	// Creates a new temporal file to store the old file and the line to delete
	fpNew = fopen( "temporal.txt", "w" );

	// Reads each line until there is nothing else to read
	while( fgets( line, 150, fpOld ) != NULL ) 
	{
		// Parse each line by spaces
		parseCommand( line, parseLine );

		// If the container's name is found, change the status to stopped
		if( ( strcmp( parseLine[3], name ) ) == 0 ) {
			functResult = 0;	// == 0 because the container's status was successfully deleted
		}

		else{	// If the container's name is not the chosen by the user then copies the line to the new file
			fprintf( fpNew, "%s", line );
		}
	}
	fclose( fpOld );
	fclose( fpNew );
	remove( FILENAME );	// Deletes the old containers file
	rename( "temporal.txt", FILENAME );	// Changes the name of the old file to containers.txt with the line deleted


	// Unlocking when finished
	pthread_mutex_unlock( &mutex );
}


int checkStatus( char *name )
{
	FILE *fp;
	char line[150], parseLine[SIZE_MESSAGE][LENGHT_MESSAGE];

	// Locking while reading the file
	pthread_mutex_lock( &mutex );
	fp = fopen( FILENAME, "r" );
	
	// Reads each line until there is nothing else to read
	while( fgets( line, 150, fp ) != NULL ) 
	{
		// Parse each line by spaces
		parseCommand( line, parseLine );

		// Compares the name of the container with the name to search
		if( ( strcmp( parseLine[3], name ) ) == 0 ) 
		{
			// Compares the status of the container with running
			if( strcmp( parseLine[1], "running" ) == 0 )
			{
				pthread_mutex_unlock( &mutex );
				return -2;
			}
			pthread_mutex_unlock( &mutex );
			return 0;
		}
	}
	fclose( fp );

	// Unlocking when finished
	pthread_mutex_unlock( &mutex );

}


void *createContainer( void *arg )
{
	char *args, names_create[SIZE_MESSAGE][LENGHT_MESSAGE];
	pid_t pid;
	FILE *file; 

	// Starts at 0 and could change to -1 if an error is found
	functResult = 0;

	puts( "\n\nCREATING CONTAINER...\n" );
	// Parse the client message by spaces
	args = (char*)arg;
	parseCommand( args, names_create );
	
	// If the number is == -1, then the container does not exists yet
	if( searchWord( names_create[1] ) == -1 ){	
		addContainerWord( names_create[1], names_create[2] );
	}
	else{
		puts( "ERROR: Container name already exist\n" );
		functResult = -1;	// == -1 because the container's name does not exists
		return 0;
	}	
	
	pid = fork();
	
	if( pid < 0 ){
		perror( "ERROR: " );
		return 0;
	}

	/* Ask if this is a good practice
		name[1] = name of the container
		name[2] = image 
	*/ 
	else if( pid == 0 ){
		execlp( "sudo", "sudo", "docker", "run", "-di", "--name", names_create[1], names_create[2], NULL ) > 0;
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
	FILE *fp;
	char line[150];

	puts( "\n\nLISTING CONTAINERS...\n" );
	functResult = 0;

	// Lock the mutex while reading the file
	pthread_mutex_lock( &mutex );

	fp = fopen( FILENAME, "r" );
	
	// Check if the file is empty
	if( fgets( line, 150, fp) == NULL ){
		printf( "%s\n", line );
		fclose(fp);
		functResult = -1;	// == -1 because the file is empty
		puts( "ERROR: There are no containers yet\n" );
		pthread_mutex_unlock( &mutex );
		return 0;
	}
	else{
		printf( "%s", line );
	}
	
	// Print everything written in the file
	while( fgets( line, 150, fp ) != NULL ) {		
		printf( "%s", line );
	} 

	fclose(fp);
	// Unlocking when finished
	pthread_mutex_unlock( &mutex );

	return 0;
}


void *stopContainer( void *arg )
{
	char *args, name_stop[SIZE_MESSAGE][LENGHT_MESSAGE];
	pid_t pid;

	puts( "\n\nSTOPPING CONTAINER...\n" );

	// Parse args by spaces into name_stop
	args = (char*)arg;
	parseCommand( args, name_stop );

	// Starts at -1 and could change to 0 if the container's name does exist
	functResult = -1;

	changeStatus( name_stop[1] );

	// If functResult == -1, then the container's name does not exist, then there is an error
	if( functResult == -1 ){
		puts( "ERROR: The container does not exist and can not be stopped\n" );
		return 0;
	}
	
	pid = fork();
	
	if( pid < 0 ){
		perror( "ERROR: " );
		return 0;
	}
	else if( pid == 0 ){
		// name[1] = container's name
		execlp( "sudo", "sudo", "docker", "stop", name_stop[1], NULL );
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

	puts( "\n\nDELETING CONTAINER...\n" );

	args = (char*)arg;
	parseCommand( args, name_delete );
	
	// If functResult == -1, then the container's name does not exist
	functResult = -1;	
	if( searchWord( name_delete[1] ) == -1 ){
		puts( "ERROR: The container does not exist and can not be deleted\n" );
		return 0;
	}

	// If functResult == -2, then the container is not stopped
	functResult = -2;
	if( checkStatus( name_delete[1] ) == -2 ){
		puts( "ERROR: Container must be stopped to be deleted\n" );
		return 0;
	}

	deleteContainerFile( name_delete[1] );

	pid = fork();

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
	createFile();

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
	system("clear");

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

				// Send the confirmation or error message
				if( functResult == -1 ){
					strcpy( send_message, "Container name already exist" );
				}
				else{
					strcpy( send_message, "The container was successfully created" );
				}
				
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

				// Send the confirmation or error message
				if( functResult == -1 ){
					strcpy( send_message, "There are no containers yet" );
				}
				else{
					strcpy( send_message, "The containers was successfully listed" );
				}

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

				// Send the confirmation or error message
				if( functResult == -1 ){
					strcpy( send_message, "The container does not exist and can not be stopped" );
				}
				else{
					strcpy( send_message, "The container have been successfully stopped" );
				}

				send( client_sock , send_message , strlen(send_message), 0 );

				break;


			// Case to delete container 
			case 4:
				if( pthread_create( &threadToDelete, NULL, deleteContainer, &client_message ) != 0 ){
					perror( "ERROR: " );
				}
				
				if( pthread_join( threadToDelete, NULL ) != 0 ){
					perror( "ERROR: " );
				}

				// Send the confirmation or error message
				if( functResult == -1 ){
					strcpy( send_message, "The container does not exist and can not be deleted" );
				}
				else if( functResult == -2 ){
					strcpy( send_message, "Container must be stopped to be deleted" );
				}
				else{
					strcpy( send_message, "The container have been successfully deleted" );
				}

				send( client_sock , send_message , strlen(send_message), 0 );

				break;
			

			// Case to search another client if the current one exit
			case 0:
				system("clear");
				puts( "\nWaiting for incoming connections..." );
				client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
				if( client_sock < 0 ) {
					perror( "ERROR: Connection rejected\n" );
					return EXIT_FAILURE;
				}
				puts( "Connection accepted\n" );				
				break;


			// Default case if the number is wrong
			default:
				strcpy( send_message, "Selected number is not currently available. Try again\n" );
				send( client_sock , send_message , strlen(send_message), 0 );
		}

    }

	return 0;
}