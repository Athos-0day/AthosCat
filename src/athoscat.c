#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
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
    fprintf(stderr, "  -e <cmd> [args...]   Execute command on client (client mode only, requires root)\n");
    fprintf(stderr, "  -y                   Confirm yes (listener mode only, bypass confirmation)\n");
}

int main(int argc, char *argv[]) {
    int is_listener = 0, is_client = 0;
    ip_mode_t ip_mode = IP_ANY;
    protocol_t protocol = PROTO_TCP;
    int verbose = 0;
    int timeout = -1;
    int confirm_yes = 0;
    const char *port = "1234";
    const char *host = NULL;
    char **exec_argv = NULL;

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
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            if (!is_client) {
                fprintf(stderr, "[!] -e option only valid with --client.\n");
                return 1;
            }
            int remaining = argc - (i + 1);
            exec_argv = malloc((remaining + 1) * sizeof(char *));
            if (!exec_argv) {
                perror("[!] malloc");
                return 1;
            }
            for (int j = 0; j < remaining; ++j) {
                exec_argv[j] = argv[i + 1 + j];
            }
            exec_argv[remaining] = NULL;
            break;
        } else if (strcmp(argv[i], "-y") == 0) {
            confirm_yes = 1;
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
        if (!confirm_yes) {
            FILE *tty = fopen("/dev/tty", "r");
            if (!tty) {
                fprintf(stderr, "[!] Cannot open /dev/tty for confirmation.\n");
                return 1;
            }

            fprintf(stderr, "Listening mode selected. Type 'y' to continue or 'n' to abort: ");
            char response[8];
            if (!fgets(response, sizeof(response), tty)) {
                fclose(tty);
                return 1;
            }
            fclose(tty);

            if (response[0] != 'y' && response[0] != 'Y') {
                fprintf(stderr, "[*] Aborted by user.\n");
                return 1;
            }
        }

        listen_config_t cfg = {
            .port = port,
            .ip_mode = ip_mode,
            .protocol = protocol,
            .verbose = verbose,
            .timeout = timeout,
            .exec_argv = NULL
        };
        return run_listener(&cfg);
    } else {
        if (!host) {
            fprintf(stderr, "[!] Client mode requires a host (-h).\n");
            return 1;
        }
        if (exec_argv && getuid() != 0) {
            fprintf(stderr, "[!] Executing command (-e) requires root privileges.\n");
            return 1;
        }
        client_config_t cfg = {
            .host = host,
            .port = port,
            .ip_mode = ip_mode,
            .protocol = protocol,
            .verbose = verbose,
            .timeout = timeout,
            .exec_argv = exec_argv
        };
        int ret = run_client(&cfg);
        if (exec_argv) free(exec_argv);
        return ret;
    }
}
