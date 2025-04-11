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
	(void)signum;
	syslog(LOG_INFO, "SIGINT or SIGTERM detected, exiting");
	if (server_socket != -1) close(server_socket);
	if (client_socket != -1) close(client_socket);
	remove(FILE_PATH);
	closelog();
	exit(EXIT_SUCCESS);
}

/*		          daemonize
 *A function that daemonizes the aesdsocket process if the
 *passes a "-d" flag with aesdsocket.
 */
void daemonize()
{
	pid_t pid = fork();
	if (pid < 0){
		syslog(LOG_INFO, "Could not daemonize aesdsocket");
		syslog(LOG_INFO, "Exiting...");
		exit(EXIT_FAILURE);
	}
	if (pid > 0) exit(EXIT_SUCCESS);
	umask(0);
	if (setsid() < 0){
		syslog(LOG_INFO, "Could not create a new session");
		exit(EXIT_FAILURE);
	}
	if (chdir("/") < 0) exit(EXIT_FAILURE);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

int main(int argc, char *argv[])
{
	int daemon_mode = (argc > 1 && (strcmp(argv[1], "-d") == 0));
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_size = sizeof(client_addr);
	char buffer[BUFFER_SIZE];
	ssize_t bytes_received;

	openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
	signal(SIGINT, cleanup_and_exit);
	signal(SIGTERM, cleanup_and_exit);

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		syslog(LOG_ERR, "Socket creating failed");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		syslog(LOG_ERR, "Setsocket failed");
		exit(EXIT_FAILURE);
	}


	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		syslog(LOG_ERR, "Binding failed");
		exit(EXIT_FAILURE);
	}

	if (daemon_mode) daemonize();

	if (listen(server_socket, 5) < 0) {
		syslog(LOG_ERR, "Listen failed");
		exit(EXIT_FAILURE);
	}

	while(1) {
		client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
		if (client_socket < 0){
			syslog(LOG_ERR, "Accept failed");
		//	continue;
		
		}

		syslog(LOG_INFO, "Accepted conection from %s", inet_ntoa(client_addr.sin_addr));
		int file_fd = open(FILE_PATH, O_CREAT | O_WRONLY | O_APPEND, 0644);
		if (file_fd < 0) {
			syslog(LOG_ERR, "File open failed");
			close(client_socket);
			//continue;
			return;
		}

		while((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0){
			write(file_fd, buffer, bytes_received);
			if (buffer[bytes_received - 1] == '\n') break;
		}
		close(file_fd);

		file_fd = open(FILE_PATH, O_RDONLY);
		if (file_fd < 0) {
			syslog(LOG_ERR, "File read failed");
			close(client_socket);
			continue;
		}

		while((bytes_received = read(file_fd, buffer, BUFFER_SIZE)) > 0)
			send(client_socket, buffer, bytes_received, 0);
		close(file_fd);

		syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(client_addr.sin_addr));
		close(client_socket);
	}
	return 0;


}
