#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define BUFFER_SIZE 10

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

void initialize_semaphore(int sem_id, int num, int val) {
    union semun sem_union;
    sem_union.val = val;
    if (semctl(sem_id, num, SETVAL, sem_union) == -1) {
        perror("semctl - SETVAL");
        exit(EXIT_FAILURE);
    }
}

int main() {
    key_t key = ftok("producer", 'a');
    int shm_id = shmget(key, sizeof(int) * (BUFFER_SIZE + 2), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    int *buffer = (int *)shmat(shm_id, NULL, 0);
    if (buffer == (int *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(key, 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    initialize_semaphore(sem_id, 0, BUFFER_SIZE); // Empty
    initialize_semaphore(sem_id, 1, 0);           // Full
    initialize_semaphore(sem_id, 2, 1);           // Mutex

    int in = 0;
    for (int i = 0; i < 10; i++) {
        struct sembuf wait_empty = {0, -1, SEM_UNDO};
        struct sembuf wait_mutex = {2, -1, SEM_UNDO};
        struct sembuf signal_mutex = {2, 1, SEM_UNDO};
        struct sembuf signal_full = {1, 1, SEM_UNDO};

        if (semop(sem_id, &wait_empty, 1) == -1) {
            perror("semop - wait_empty");
            exit(EXIT_FAILURE);
        }

        if (semop(sem_id, &wait_mutex, 1) == -1) {
            perror("semop - wait_mutex");
            exit(EXIT_FAILURE);
        }

        buffer[in] = rand() % 100;
        printf("Produced: %d\n", buffer[in]);
        in = (in + 1) % BUFFER_SIZE;

        if (semop(sem_id, &signal_mutex, 1) == -1) {
            perror("semop - signal_mutex");
            exit(EXIT_FAILURE);
        }

        if (semop(sem_id, &signal_full, 1) == -1) {
            perror("semop - signal_full");
            exit(EXIT_FAILURE);
        }

        sleep(1); // To simulate production time
    }

    if (shmdt(buffer) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }

    return 0;
}

