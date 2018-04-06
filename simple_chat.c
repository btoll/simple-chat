#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "hashtable.c"
#include "simple_chat.h"

/**
 * simple_chat [PORT]
 */
int main(int argc, char **argv) {
    int sock, maxfd, newfd, nread, i, j, k, r;
    size_t yes = 1, table_size = BACKLOG;

    struct addrinfo hints, *res, *p;
    struct sockaddr_storage client;

    char buf[MAX_BUF_SIZE], msg[MAX_BUF_SIZE], name[NAME_SIZE], fd_s[8];

    fd_set master, readfds;
    socklen_t sin_size;

    char *port = argc > 1 ?
        argv[1] :
        PORT;

    hash_table_t *hashtable = create_hashtable(table_size);

    memset(&fd_s, 0, strlen(fd_s));
    memset(&buf, 0, MAX_BUF_SIZE);
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((r = getaddrinfo(NULL, port, &hints, &res)) == -1) {
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

    fprintf(stderr, "chat server started on port %s...\n", port);

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
                    char *newline;

                    send(newfd, greeting, strlen(greeting), 0);
                    nread = recv(newfd, name, NAME_SIZE, 0);

                    // The hashtable key for the user will be the stringified file descriptor.
                    sprintf(fd_s, "%d", newfd);

                    // Remove the newline (telnet) or newline & carriage return (netcat).
                    if (!(newline = strstr(name, "\r\n"))) {
                        newline = strstr(name, "\n");
                    }

                    // Copy the original string from up to the newline using pointer arithmetic.
                    char buf[NAME_SIZE];
                    strncpy(buf, name, newline - name);
                    buf[newline-name] = '\0';

                    fprintf(stderr, "Received a new connection from %s (fd %d)\n", buf, newfd);

                    add_hash_entry(hashtable, fd_s, buf);

                    // Add a return character so the welcome message to the user isn't on the same
                    // line as the first chat message.
                    strncat(buf, "\n", 1);
                    strncat(msg, buf, nread);

                    send(newfd, msg, strlen(msg), 0);
                } else {
                    if ((nread = recv(i, buf, MAX_BUF_SIZE, 0)) == -1) {
                        perror("recv");
                        exit(6);
                    } else {
                        for (j = 0; j <= maxfd; ++j) {
//                             printf("i %d, j %d, maxfd %d, sock %d\n", i, j, maxfd, sock);
                            // Don't send to either the server or the socket that wrote the message that was just received.
                            if (j > sock && j != sock && j != i) {
                                // Get the sender's nickname.
                                sprintf(fd_s, "%d", i);
                                node_t *hash_entry;

                                if ((hash_entry = lookup_hash_entry(hashtable, fd_s)) != NULL) {
                                    // Handle Ctl-C.
                                    if (buf[0] == -1) {
                                        if ((snprintf(msg, strlen(hash_entry->value) + 20, "%s%s", hash_entry->value, " has left the chat\n")) == -1) {
                                            perror("snprintf");
                                            exit(6);
                                        }

                                        fprintf(stderr, "(fd %d) %s", i, msg);

                                        FD_CLR(i, &master);
                                        close(i);

                                        // Alert the other users that the user has left the chat.
                                        for (k = sock + 1; k <= maxfd; ++k)
                                            send(k, msg, strlen(msg), 0);
                                    } else {
                                        // Add the sender's nickname to the sender's chat message.
                                        memset(&msg, 0, MAX_BUF_SIZE);

                                        if ((snprintf(msg, 4 + strlen(hash_entry->value) + nread, "%s%s%s %s", "<", hash_entry->value, ">", buf)) == -1) {
                                            perror("snprintf");
                                            exit(7);
                                        }

                                        send(j, msg, nread + strlen(msg), 0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    free_hashtable(hashtable);

    return 0;
}

