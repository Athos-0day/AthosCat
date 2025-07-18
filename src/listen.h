#ifndef LISTEN_H
#define LISTEN_H

#include "utils.h"
#include <netdb.h>
#include "client.h"
 #include <stdbool.h>

/**
 * @file listen.h
 * @brief Header for the listening/server mode of AthosCat.
 *
 * This module provides a basic implementation of a listening server
 * using TCP or UDP, similar to Netcat's -l option.
 */

/* Config for server mode */
typedef struct {
    const char *port;      /* Port as string, e.g. "1234"          */
    protocol_t protocol;   /* PROTO_TCP or PROTO_UDP               */
    ip_mode_t  ip_mode;    /* IP_ANY, IP_V4, or IP_V6              */
    int  verbose;          /* 0 = quiet, 1 = verbose               */
    int  timeout;          /* seconds, 0 = no timeout (optional)   */
    char *const *exec_argv;  /* NULL-terminated array of strings for exec */
} listen_config_t;

/**
 * @brief Runs the AthosCat listener.
 *
 * This function starts a simple TCP or UDP server depending on the
 * configuration. It listens on a specified port, and:
 * - For TCP: accepts one client and echoes back or relays stdin/stdout.
 * - For UDP: receives datagrams and sends responses.
 *
 * @param config Pointer to a listen_config_t structure with listen parameters.
 * @return int 0 on success, or an error code on failure.
 */
int run_listener(const listen_config_t *config);

#endif // LISTEN_H
