#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "hash.c"

#define BACKLOG 25
#define BUF_SIZE 4096
#define NAME_SIZE 40
#define PORT "3333"

int main(int argc, char **argv) {
    size_t sock, r, maxfd, newfd, i, j, nread, len,
        yes = 1, table_size = BACKLOG;

    struct addrinfo hints, *res, *p;
    struct sockaddr_storage client;

    char buf[BUF_SIZE], name[NAME_SIZE], fd_s[8], *chatline;

    fd_set master, readfds;
    socklen_t sin_size;

    node_t *hash_entry;

    hash_table_t *hashtable = create_hashtable(table_size);

    memset(&fd_s, 0, strlen(fd_s));
    memset(&buf, 0, BUF_SIZE);
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
                    add_hash_entry(hashtable, fd_s, name);
                    strncat(msg, name, nread);

                    send(newfd, msg, strlen(msg), 0);
                } else {
                    if ((nread = recv(i, buf, BUF_SIZE, 0)) < 0) {
                        perror("recv");
                        exit(6);
                    } else {
                        for (j = 0; j <= maxfd; ++j) {
                            // Don't send to either the server or the socket that wrote the message that was just received.
                            if (j > 2 && j != sock && j != i) {
                                // Get the sender's nickname...
                                sprintf(fd_s, "%d", i);
                                hash_entry = lookup_hash_entry(hashtable, fd_s);

                                // ...and add it to the sender's chat message.
                                chatline = strndup(hash_entry->value, strlen(hash_entry->value));

                                // TODO: This is hacky, but we need to replace the newline so it's all on the same line.
                                int p;
                                for (p = 0, len = strlen(chatline); p < len; ++p)
                                    if (chatline[p] == 13)
                                        chatline[p] = '\0';

                                // Add the caret separating the nickname from the chat text.
                                strncat(chatline, "> ", 2);

                                len = nread + strlen(chatline);
                                strncat(chatline, buf, len);

                                send(j, chatline, len, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

