#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PORT 9000
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

/*               SERVER AND CLIENT SOCKETS
Declaring global variables for server and client sockets.
Initializing them to -1 to indicate that they are not created
or been assigned yet
*/
int server_socket = -1, client_socket = -1;


/*		      cleanup_and_exit
 *A signal handler function that executes when the program 
 *receives interrupt signal SIGINT (Ctrl+C) or termination
 *signal SIGTERM.
 */
void cleanup_and_exit(int signum)
{
	syslog(LOG_INFO, "SIGINT or SIGTERM detected, exiting");
	if (server_socket != -1) close(server_socket);
	if (client_socket != -1) close(client_socket);
	remove(FILE_PATH);
	closelog();
	exit(EXIT_SUCCESS);
}


