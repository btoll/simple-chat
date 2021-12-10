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
//    int sock, maxfd, newfd, nread, i, j, k, r;
    int sock, maxfd, newfd, i, j, k, r;
    size_t yes = 1, table_size = BACKLOG;

    struct addrinfo hints, *res, *p;
    struct sockaddr_storage client;

    char fd_s[8];

    fd_set master, readfds;
    socklen_t sin_size;

    char *port = argc > 1 ?
        argv[1] :
        PORT;

    hash_table_t *hashtable = create_hashtable(table_size);

    memset(&fd_s, 0, strlen(fd_s));
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;        // Fill in my IP for me.

    if ((r = getaddrinfo(NULL, port, &hints, &res)) == -1) {
        perror(gai_strerror(r));
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        // Sometimes, the bind call fails with "Address already in use."  A little bit of a socket that was
        // connected is still hanging around in the kernel, and it's hogging the port. You can either wait
        // for it to clear (a minute or so) or add code to your program allowing it to reuse the port.
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

    fprintf(stderr, "Simple chat server started on port %s...\n", port);
    fprintf(stderr, "Clients can connect using telnet, i.e.,\t`telnet 127.0.0.1 %s`\n", port);

    maxfd = sock;
    FD_ZERO(&master);
    FD_ZERO(&readfds);
    FD_SET(sock, &master);

    for (;;) {
        readfds = master;

        // This blocks until a new client request or new data request is made.
        if ((r = select(maxfd + 1, &readfds, NULL, NULL, NULL)) == -1) {
            perror("select");
            exit(4);
        }

        int nread;
        char buf[MAX_BUF_SIZE];
        memset(&buf, 0, MAX_BUF_SIZE);

        // If the master socket file descriptor is set, then a new request has been
        // made.  Recall that the kernel would redirect new requests to the master
        // socket file descriptor.
        if FD_ISSET(sock, &readfds) {
            sin_size = sizeof(client);

            if ((newfd = accept(sock, (struct sockaddr *) &client, &sin_size)) == -1) {
                perror("accept");
                exit(5);
            }

            // Add new client communication socket file descriptor to the set.
            FD_SET(newfd, &master);

            if (newfd > maxfd)
                maxfd = newfd;

            char name[NAME_SIZE];
            memset(&name, 0, NAME_SIZE);
            char greeting[] = "Hello, what is your name? ";
            char initialmsg[] = "Hi ";
            char *newline;

            send(newfd, greeting, strlen(greeting), 0);
            nread = recv(newfd, name, NAME_SIZE, 0);

            // The hashtable key for the user will be the stringified file descriptor.
            sprintf(fd_s, "%d", newfd);

            // Locate the substring of the newline (telnet) or newline & carriage return (netcat).
            if (!(newline = strstr(name, "\r\n"))) {
                newline = strstr(name, "\n");
            }

            // Copy the original string from up to the newline using pointer arithmetic.
            strncpy(buf, name, newline - name);
            buf[newline-name] = '\0';

            fprintf(stderr, "Received a new connection from %s (fd %d)\n", buf, newfd);

            add_hash_entry(hashtable, fd_s, buf);

            // Add a return character so the welcome message to the user isn't on the same
            // line as the first chat message.
            strncat(buf, "\n", 1);
            strncat(initialmsg, buf, nread);

            send(newfd, initialmsg, strlen(initialmsg), 0);
        } else // The communication file descriptor was activated.
        {
            for (i = 0; i <= maxfd; ++i) {
                if FD_ISSET(i, &readfds) {
                    if ((nread = recv(i, buf, MAX_BUF_SIZE, 0)) == -1) {
                        perror("recv");
                        exit(6);
                    }

                    // Don't send to either the server or the to fd that wrote the message.
                    for (j = 0; j <= maxfd; ++j) {
                        if (j > sock && j != sock && j != i) {
                            // Get the sender's nickname.
                            sprintf(fd_s, "%d", i);
                            node_t *hash_entry;

                            if ((hash_entry = lookup_hash_entry(hashtable, fd_s)) != NULL) {
                                char msg[MAX_BUF_SIZE];
                                memset(&msg, 0, MAX_BUF_SIZE);

                                // Client closed the connection || Client pressed Ctl-C.
                                if (nread == 0 || buf[0] == -1) {
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

                                    // Break here to avoid duplicate "X has left the chat" messages.
                                    break;
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

    free_hashtable(hashtable);
    return 0;
}

