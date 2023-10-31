#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFFER_SIZE 10

int main() {
    key_t key = ftok("producer", 'a');
    int shm_id = shmget(key, sizeof(int) * (BUFFER_SIZE + 2), 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    int *buffer = (int *)shmat(shm_id, NULL, 0);
    if (buffer == (int *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(key, 3, 0666);
    if (sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    int out = 0;
    for (int i = 0; i < 5; i++) {
        struct sembuf wait_full = {1, -1, SEM_UNDO};
        struct sembuf wait_mutex = {2, -1, SEM_UNDO};
        struct sembuf signal_mutex = {2, 1, SEM_UNDO};
        struct sembuf signal_empty = {0, 1, SEM_UNDO};

        if (semop(sem_id, &wait_full, 1) == -1) {
            perror("semop - wait_full");
            exit(EXIT_FAILURE);
        }

        if (semop(sem_id, &wait_mutex, 1) == -1) {
            perror("semop - wait_mutex");
            exit(EXIT_FAILURE);
        }

        int item = buffer[out];
        printf("Consumed: %d\n", item);
        out = (out + 1) % BUFFER_SIZE;

        if (semop(sem_id, &signal_mutex, 1) == -1) {
            perror("semop - signal_mutex");
            exit(EXIT_FAILURE);
        }

        if (semop(sem_id, &signal_empty, 1) == -1) {
            perror("semop - signal_empty");
            exit(EXIT_FAILURE);
        }

        sleep(1); // To simulate consumption time
    }

    if (shmdt(buffer) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    return 0;
}

