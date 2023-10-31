#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid1, pid2, pid3;

    char *producer_argv[] = {"./producer", NULL};
    char *consumer_argv[] = {"./consumer", NULL};

    pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // In first child process
        execv("./producer", producer_argv);
        // If execv fails
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        // In parent process
        pid2 = fork();
        if (pid2 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid2 == 0) {
            // In second child process
            execv("./consumer", consumer_argv);
            // If execv fails
            perror("execv");
            exit(EXIT_FAILURE);
        } else {
            // Still in parent process
            pid3 = fork();
            if (pid3 == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid3 == 0) {
                // In third child process
                execv("./consumer", consumer_argv);
                // If execv fails
                perror("execv");
                exit(EXIT_FAILURE);
            }
        }
    }

    // In parent process, waiting for children to finish
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    printf("Producer and consumers have finished.\n");
    return 0;
}

