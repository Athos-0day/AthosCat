#ifndef UTILS_H
#define UTILS_H

#include <netdb.h>

/**
 * @file utils.h
 * @brief Utility functions for networking and error handling in athoscat.
 */

 /// Network protocol to use
typedef enum {
    PROTO_TCP,
    PROTO_UDP
} protocol_t;

/// IP version
typedef enum {
    IP_ANY,
    IP_V4,
    IP_V6
} ip_mode_t;

#endif // UTILS_H
