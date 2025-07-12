#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include "utils.h"

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

void fatal_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void log_info(const client_config_t *config, const char *format, ...) {
    if (!config->verbose)
        return;
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}