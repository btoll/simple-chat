#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 4096
#define PORT "3333"

int main(int argc, char **argv) {
    struct addrinfo hints, *res, *p;
    int sock, r;
    char buf[BUF_SIZE];

    memset(&buf, 0, BUF_SIZE);
    memset(&res, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((r = getaddrinfo("localhost", PORT, &hints, &res)) == -1) {
        fprintf(stderr, gai_strerror(r));
        exit(1);
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, 0)) == -1) {
            perror("sock");
            continue;
        }

        if ((r = connect(sock, p->ai_addr, p->ai_addrlen)) == -1) {
            close(sock);
            perror("sock");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: could not establish connection\n");
        exit(2);
    }

    freeaddrinfo(res);

    for (;;) {
        int nread;

        while (printf("> "), fgets(buf, BUF_SIZE, stdin), !feof(stdin)) {
            if ((r = send(sock, buf, strlen(buf), 0)) == -1) {
                perror("send");
                exit(4);
            }

            if ((nread = recv(sock, buf, BUF_SIZE, 0)) < 0) {
                perror("recv");
                exit(5);
            } else {
                buf[nread] = '\0';
                printf("%s\n", buf);
            }
        }
    }

    return 0;
}

