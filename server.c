#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "hash.c"

#define BACKLOG 25
#define MAX_BUF_SIZE 4096
#define MIN_BUF_SIZE 4
#define NAME_SIZE 40
#define PORT "3333"

void send_message(hash_table_t *hashtable, int sender_fd, int receiver_fd, char *buf, size_t nread) {
    char fd_s[MIN_BUF_SIZE], msg[MAX_BUF_SIZE], *chatline;

    memset(&fd_s, 0, MIN_BUF_SIZE);
    memset(&msg, 0, MAX_BUF_SIZE);

    // Get the sender's nickname...
    sprintf(fd_s, "%d", sender_fd);
    node_t *hash_entry = lookup_hash_entry(hashtable, fd_s);

//     printf("hash entry value %s\n", hash_entry->value);

    // ...and add it to the sender's chat message.
    chatline = strndup(hash_entry->value, strlen(hash_entry->value));

//     msg = "<";
    strncat(msg, chatline, strlen(chatline));
    // Add the caret separating the nickname from the chat text.
    strncat(msg, "> ", 2);
    strncat(msg, buf, nread + strlen(msg));

    send(receiver_fd, msg, strlen(msg), 0);
}

int main(int argc, char **argv) {
    int sock, maxfd, newfd, i;
    size_t r, j, nread, yes = 1, table_size = BACKLOG;

    struct addrinfo hints, *res, *p;
    struct sockaddr_storage client;

    char buf[MAX_BUF_SIZE], name[NAME_SIZE], fd_s[8];

    fd_set master, readfds;
    socklen_t sin_size;

    hash_table_t *hashtable = create_hashtable(table_size);

    memset(&fd_s, 0, strlen(fd_s));
    memset(&buf, 0, MAX_BUF_SIZE);
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((r = getaddrinfo(NULL, PORT, &hints, &res)) == -1) {
        perror(gai_strerror(r));
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        // Allow the socket to be reused.
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            close(sock);
            perror("setsockopt");
            continue;
        }

        if ((r = bind(sock, p->ai_addr, p->ai_addrlen)) == -1) {
            close(sock);
            perror("bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: could not connect\n");
        exit(2);
    }

    freeaddrinfo(res);

    if ((r = listen(sock, BACKLOG)) == -1) {
        perror("listen");
        exit(3);
    }

    printf("chat server started on port %s...\n", PORT);

    maxfd = sock;
    FD_ZERO(&master);
    FD_ZERO(&readfds);
    FD_SET(sock, &master);

    for (;;) {
        readfds = master;

        if ((r = select(maxfd + 1, &readfds, NULL, NULL, NULL)) == -1) {
            perror("select");
            exit(4);
        }

        for (i = 0; i <= maxfd; ++i) {
            if (FD_ISSET(i, &readfds)) {
                if (i == sock) {
                    sin_size = sizeof(client);

                    if ((newfd = accept(i, (struct sockaddr *) &client, &sin_size)) == -1) {
                        perror("accept");
                        exit(5);
                    }

                    FD_SET(newfd, &master);
                    maxfd = newfd;

                    char greeting[] = "Hello, what is your name? ";
                    char msg[] = "Hi ";

                    send(newfd, greeting, strlen(greeting), 0);
                    nread = recv(newfd, name, NAME_SIZE, 0);

                    printf("Received a new connection from %s", name);

                    // The hashtable key for the user will be the stringified file descriptor.
                    sprintf(fd_s, "%d", newfd);

                    // Remove the newline (remember it's null-terminated).
                    name[strlen(name) - 2] = '\0';

                    add_hash_entry(hashtable, fd_s, name);

                    strncat(name, "\n", 1);
                    strncat(msg, name, nread);

                    send(newfd, msg, strlen(msg), 0);
                } else {
                    if ((nread = recv(i, buf, MAX_BUF_SIZE, 0)) < 0) {
                        perror("recv");
                        exit(6);
                    } else {
                        for (j = 0; j <= maxfd; ++j) {
                            // Don't send to either the server or the socket that wrote the message that was just received.
                            if (j > 2 && j != sock && j != i) {
                                // Get the sender's nickname...
                                sprintf(fd_s, "%d", i);
                                node_t *hash_entry = lookup_hash_entry(hashtable, fd_s);

                                // ...and add it to the sender's chat message.

                                char msg[MAX_BUF_SIZE];

                                memset(&msg, 0, MAX_BUF_SIZE);
                                if ((snprintf(msg, 4 + strlen(hash_entry->value) + nread, "%s%s%s %s", "<", hash_entry->value, ">", buf)) == -1) {
                                    perror("snprintf");
                                    exit(7);
                                }

                                send(j, msg, nread + strlen(msg), 0);
//                                 send_message(hashtable, i, j, buf, nread);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

