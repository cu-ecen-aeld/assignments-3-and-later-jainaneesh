#define _GNU_SOURCE
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
#include <pthread.h>
#include <time.h>
#include <sys/queue.h>

#define PORT 9000
#define FILE_PATH "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

int server_socket = -1;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t stop = 0;

struct thread_data {
    pthread_t thread_id;
    int client_socket;
    struct sockaddr_in client_addr;
    SLIST_ENTRY(thread_data) entries;
};

SLIST_HEAD(thread_list, thread_data) thread_head = SLIST_HEAD_INITIALIZER(thread_head);

void cleanup_and_exit(int signum) {
    (void)signum;
    syslog(LOG_INFO, "SIGINT or SIGTERM detected, exiting");
    stop = 1;
    if (server_socket != -1) shutdown(server_socket, SHUT_RDWR);
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "Failed to fork");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) exit(EXIT_SUCCESS); // Parent exits
    umask(0);
    if (setsid() < 0) exit(EXIT_FAILURE);
    if (chdir("/") < 0) exit(EXIT_FAILURE);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void *handle_client(void *arg) {
    struct thread_data *tdata = (struct thread_data *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes;

    pthread_mutex_lock(&file_mutex);
    int file_fd = open(FILE_PATH, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (file_fd < 0) {
        syslog(LOG_ERR, "Failed to open file for writing");
        pthread_mutex_unlock(&file_mutex);
        close(tdata->client_socket);
        return NULL;
    }

    while ((bytes = recv(tdata->client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        write(file_fd, buffer, bytes);
        if (buffer[bytes - 1] == '\n') break;
    }
    close(file_fd);
    pthread_mutex_unlock(&file_mutex);

    pthread_mutex_lock(&file_mutex);
    file_fd = open(FILE_PATH, O_RDONLY);
    if (file_fd >= 0) {
        while ((bytes = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
            send(tdata->client_socket, buffer, bytes, 0);
        }
        close(file_fd);
    }
    pthread_mutex_unlock(&file_mutex);

    syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(tdata->client_addr.sin_addr));
    close(tdata->client_socket);
    return NULL;
}

void *timestamp_writer(void *arg) {
    (void)arg;
    while (!stop) {
        sleep(10);
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_buf[128];
        strftime(time_buf, sizeof(time_buf), "timestamp:%a, %d %b %Y %H:%M:%S %z\n", tm_info);

        pthread_mutex_lock(&file_mutex);
        int fd = open(FILE_PATH, O_WRONLY | O_APPEND);
        if (fd >= 0) {
            write(fd, time_buf, strlen(time_buf));
            close(fd);
        }
        pthread_mutex_unlock(&file_mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int daemon_mode = (argc > 1 && strcmp(argv[1], "-d") == 0);
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    pthread_t timestamp_thread;

    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        syslog(LOG_ERR, "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        syslog(LOG_ERR, "Bind failed");
        exit(EXIT_FAILURE);
    }

    if (daemon_mode) daemonize();

    if (listen(server_socket, 5) < 0) {
        syslog(LOG_ERR, "Listen failed");
        exit(EXIT_FAILURE);
    }

    pthread_create(&timestamp_thread, NULL, timestamp_writer, NULL);

    while (!stop) {
        struct thread_data *tdata = malloc(sizeof(struct thread_data));
        if (!tdata) continue;

        tdata->client_socket = accept(server_socket,
                                      (struct sockaddr*)&tdata->client_addr,
                                      &addr_size);

        if (tdata->client_socket < 0) {
            free(tdata);
            continue;
        }

        syslog(LOG_INFO, "Accepted connection from %s",
               inet_ntoa(tdata->client_addr.sin_addr));

        pthread_create(&tdata->thread_id, NULL, handle_client, tdata);
        SLIST_INSERT_HEAD(&thread_head, tdata, entries);

        // Clean up completed threads
        struct thread_data *curr = SLIST_FIRST(&thread_head);
        struct thread_data *prev = NULL;

        while (curr != NULL) {
            struct thread_data *next = SLIST_NEXT(curr, entries);
            if (pthread_tryjoin_np(curr->thread_id, NULL) == 0) {
                if (prev == NULL) {
                    SLIST_REMOVE_HEAD(&thread_head, entries);
                } else {
                    SLIST_NEXT(prev, entries) = next;
                }
                free(curr);
            } else {
                prev = curr;
            }
            curr = next;
        }
    }

    // Join timestamp thread
    pthread_join(timestamp_thread, NULL);

    // Join remaining client threads
    struct thread_data *np;
    while (!SLIST_EMPTY(&thread_head)) {
        np = SLIST_FIRST(&thread_head);
        pthread_join(np->thread_id, NULL);
        SLIST_REMOVE_HEAD(&thread_head, entries);
        free(np);
    }

    remove(FILE_PATH);
    closelog();
    pthread_mutex_destroy(&file_mutex);
    return 0;
}
