#ifndef UTILS_H
#define UTILS_H

#include <netdb.h>
#include "client.h"  // for client_config_t

/**
 * @file utils.h
 * @brief Utility functions for networking and error handling in athoscat.
 */

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

/**
 * @brief Print a fatal error message and exit the program.
 *
 * @param msg The error message to display before exiting.
 */
void fatal_error(const char *msg);

/**
 * @brief Print an informational message if verbose mode is enabled.
 *
 * @param config The client config to check if verbose is enabled.
 * @param format printf-like format string
 * @param ... Arguments for the format string
 */
void log_info(const client_config_t *config, const char *format, ...);

#endif // UTILS_H
