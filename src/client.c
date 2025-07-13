#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/time.h> 


int resolve_host(const client_config_t *config, struct addrinfo **result) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);

    // Set socket type based on protocol
    hints.ai_socktype = (config->protocol == PROTO_TCP) ? SOCK_STREAM : SOCK_DGRAM;

    // Set family based on IP mode
    switch (config->ip_mode) {
        case IP_V4:
            hints.ai_family = AF_INET;
            break;
        case IP_V6:
            hints.ai_family = AF_INET6;
            break;
        case IP_ANY:
        default:
            hints.ai_family = AF_UNSPEC;
            break;
    }

    int status = getaddrinfo(config->host, config->port, &hints, result);
    if (status != 0) {
        if (config->verbose) {
            fprintf(stderr, "[!] getaddrinfo error: %s\n", gai_strerror(status));
        }
        return 1;
    }

    if (config->verbose) {
        printf("[+] Host '%s' resolved successfully on port %s\n", config->host, config->port);
    }

    return 0;
}

int run_client(const client_config_t *config) {
    struct addrinfo *res, *p;
    int sockfd = -1;
    int status;

    if (resolve_host(config, &res) != 0) {
        return 1;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        if ((config->protocol == PROTO_TCP && p->ai_socktype != SOCK_STREAM) ||
            (config->protocol == PROTO_UDP && p->ai_socktype != SOCK_DGRAM)) {
            continue;
        }

        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            if (config->verbose) perror("[!] socket");
            continue;
        }

        if (config->protocol == PROTO_TCP) {
            status = connect(sockfd, p->ai_addr, p->ai_addrlen);
            if (status == -1) {
                if (config->verbose) perror("[!] connect");
                close(sockfd);
                continue;
            }
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "[!] Failed to create socket and connect (if TCP).\n");
        freeaddrinfo(res);
        return 2;
    }

    if (config->verbose) {
        fprintf(stderr, "[+] Socket ready.\n");
    }

    bool stdin_closed = false;

    while (1) {
        fd_set readfs;
        FD_ZERO(&readfs);
        if (!stdin_closed) FD_SET(STDIN_FILENO, &readfs);
        FD_SET(sockfd, &readfs);

        int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        struct timeval tv;
        struct timeval *tv_ptr = NULL;
        if (config->timeout > 0) {
            tv.tv_sec = config->timeout;
            tv.tv_usec = 0;
            tv_ptr = &tv;
        }

        int ready = select(maxfd + 1, &readfs, NULL, NULL, tv_ptr);

        if (ready == -1) {
            perror("[!] select");
            break;
        } else if (ready == 0) {
            if (config->verbose) {
                fprintf(stderr, "[!] Timeout reached, no data received.\n");
            }
            break;
        }

        if (!stdin_closed && FD_ISSET(STDIN_FILENO, &readfs)) {
            char buf[1024];
            ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
            if (len == 0) {
                if (config->protocol == PROTO_TCP) {
                    shutdown(sockfd, SHUT_WR);
                }
                stdin_closed = true;
            } else if (len < 0) {
                perror("[!] read stdin");
                break;
            } else {
                ssize_t sent;
                if (config->protocol == PROTO_TCP) {
                    sent = write(sockfd, buf, len);
                } else {
                    sent = sendto(sockfd, buf, len, 0, p->ai_addr, p->ai_addrlen);
                }
                if (sent < 0) {
                    perror("[!] send/write");
                    break;
                }
            }
        }

        if (FD_ISSET(sockfd, &readfs)) {
            char buf[1024];
            ssize_t len;
            if (config->protocol == PROTO_TCP) {
                len = read(sockfd, buf, sizeof(buf));
            } else {
                struct sockaddr_storage src_addr;
                socklen_t addrlen = sizeof(src_addr);
                len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&src_addr, &addrlen);
            }

            if (len == 0) {
                if (config->verbose) {
                    fprintf(stderr, "[+] Server closed connection.\n");
                }
                break;
            } else if (len < 0) {
                perror("[!] recv/read");
                break;
            } else {
                ssize_t written = write(STDOUT_FILENO, buf, len);
                if (written < 0) {
                    perror("[!] write stdout");
                    break;
                }
            }
        }
    }

    close(sockfd);
    freeaddrinfo(res);
    return 0;
}
