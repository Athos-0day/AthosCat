#ifndef CLIENT_H
#define CLIENT_H

#include "utils.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @file client.h
 * @brief Header for the Netcat-like client in the Athoscat project.
 *
 * Supports:
 * - TCP and UDP connections
 * - IPv4 and IPv6
 * - Connection timeout
 * - Verbose/debug mode
 * - File input/output redirection
 */

/**
 * @struct client_config_t
 * @brief Configuration struct for the Netcat-like client
 */
typedef struct {
    const char *host;         ///< Target hostname or IP
    const char *port;         ///< Target port as string
    protocol_t protocol;      ///< TCP or UDP
    ip_mode_t ip_mode;        ///< IPv4, IPv6 or any
    int timeout;              ///< Timeout in seconds for connection (-1 = no timeout)
    bool verbose;             ///< Verbose/debug mode
    const char *input_file;   ///< Path to input file (NULL = use stdin)
    const char *output_file;  ///< Path to output file (NULL = use stdout)
} client_config_t;

/**
 * @brief Run the client with the given configuration
 *
 * @param config Pointer to a fully populated client_config_t structure
 * @return 0 on success, non-zero on error
 */
int run_client(const client_config_t *config);

/**
 * @brief Resolve a hostname and port using getaddrinfo().
 *
 * This function uses the configuration struct to determine protocol,
 * address family (IPv4, IPv6), and verbosity.
 *
 * @param config Pointer to the client configuration
 * @param result Pointer to a struct addrinfo* that will receive the results
 * @return 0 on success, non-zero on error
 */
int resolve_host(const client_config_t *config, struct addrinfo **result);

#endif // CLIENT_H


