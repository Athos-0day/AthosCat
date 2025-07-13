#include "exec.h"
#include <unistd.h>     // fork(), dup2(), execvp()
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid()
#include <stdio.h>      // perror()
#include <stdlib.h>     // exit()
#include <stdbool.h>    // bool

int run_exec(int sockfd, char *const argv[], bool verbose) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("[!] fork");
        close(sockfd);
        return 1;
    }
    
    if (pid == 0) {
        // Child process

        // Redirect socket to stdin, stdout, stderr
        if (dup2(sockfd, STDIN_FILENO) == -1 ||
            dup2(sockfd, STDOUT_FILENO) == -1 ||
            dup2(sockfd, STDERR_FILENO) == -1) {
            perror("[!] dup2");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        close(sockfd); // sockfd no longer needed after dup2

        if (verbose) {
            fprintf(stderr, "[*] Executing program: %s\n", argv[0]);
        }

        // Execute the program
        execvp(argv[0], argv);

        // If execvp returns, it failed
        perror("[!] execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(sockfd); // Close socket in parent if not needed here

        if (verbose) {
            fprintf(stderr, "[*] Spawned child process PID=%d\n", pid);
        }

        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("[!] waitpid");
            return 2;
        }

        if (WIFEXITED(status)) {
            if (verbose) {
                fprintf(stderr, "[*] Child exited with status %d\n", WEXITSTATUS(status));
            }
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            if (verbose) {
                fprintf(stderr, "[*] Child terminated by signal %d\n", WTERMSIG(status));
            }
            return 3;
        }

        return 0;
    }
}
