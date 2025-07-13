#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "listen.h"
#include "client.h"
#include "utils.h"

void print_usage(const char *progname) {
    fprintf(stderr, "Usage: %s (--listen | --client) [options]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --listen             Start in listener mode\n");
    fprintf(stderr, "  --client             Start in client mode\n");
    fprintf(stderr, "  -p <port>            Port number (default: 1234)\n");
    fprintf(stderr, "  -4                   Use IPv4\n");
    fprintf(stderr, "  -6                   Use IPv6\n");
    fprintf(stderr, "  -u                   Use UDP (default is TCP)\n");
    fprintf(stderr, "  -v                   Verbose output\n");
    fprintf(stderr, "  -t <timeout>         Timeout in seconds (UDP only)\n");
    fprintf(stderr, "  -h <host>            Remote host (client mode only)\n");
}

int main(int argc, char *argv[]) {
    int is_listener = 0, is_client = 0;
    ip_mode_t ip_mode = IP_ANY;
    protocol_t protocol = PROTO_TCP;
    int verbose = 0;
    int timeout = -1;
    const char *port = "1234";
    const char *host = NULL;

    // Parsing des arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--listen") == 0) {
            is_listener = 1;
        } else if (strcmp(argv[i], "--client") == 0) {
            is_client = 1;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = argv[++i];
        } else if (strcmp(argv[i], "-4") == 0) {
            ip_mode = IP_V4;
        } else if (strcmp(argv[i], "-6") == 0) {
            ip_mode = IP_V6;
        } else if (strcmp(argv[i], "-u") == 0) {
            protocol = PROTO_UDP;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            timeout = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            host = argv[++i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (is_listener && is_client) {
        fprintf(stderr, "[!] Cannot use both --listen and --client.\n");
        return 1;
    }

    if (!is_listener && !is_client) {
        fprintf(stderr, "[!] You must specify either --listen or --client.\n");
        print_usage(argv[0]);
        return 1;
    }

    if (is_listener) {
        listen_config_t cfg = {
            .port = port,
            .ip_mode = ip_mode,
            .protocol = protocol,
            .verbose = verbose,
            .timeout = timeout
        };
        return run_listener(&cfg);

    } else { // client mode
        if (!host) {
            fprintf(stderr, "[!] Client mode requires a host (-h).\n");
            return 1;
        }

        client_config_t cfg = {
            .host = host,
            .port = port,
            .ip_mode = ip_mode,
            .protocol = protocol,
            .verbose = verbose,
            .timeout = timeout
        };
        return run_client(&cfg);
    }
}
