#ifndef EXEC_H
#define EXEC_H

#include <stdbool.h>

/**
 * @file exec.h
 * @brief Functions to execute a program redirecting its input/output to a socket.
 *
 * This module provides functionality to launch an external program or shell,
 * with its standard input, output, and error streams redirected through a
 * network socket. This enables remote command execution or shell sessions over
 * the network connection established by AthosCat.
 */

/**
 * @brief Run a program with arguments, redirecting stdio to the socket.
 * 
 * @param sockfd Socket file descriptor for I/O redirection.
 * @param argv Null-terminated array of strings for program and its arguments (argv[0] = program).
 * @param verbose If true, print debug info.
 * @return int 0 on success, non-zero on error.
 */
int run_exec(int sockfd, char *const argv[], bool verbose);

#endif // EXEC_H
