#include "shared.h"
#include <iostream>
#include <unistd.h>

void wait(int sem_id) {
    struct sembuf p = {0, -1, SEM_UNDO};
    semop(sem_id, &p, 1);
}

void signal(int sem_id) {
    struct sembuf v = {0, 1, SEM_UNDO};
    semop(sem_id, &v, 1);
}

int main() {
    // Create shared memory
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    auto *shared_data = static_cast<SharedData *>(shmat(shm_id, nullptr, 0));

    // Create semaphores
    int sem_mutex = semget(SEM_MUTEX_KEY, 1, IPC_CREAT | 0666);
    int sem_empty = semget(SEM_EMPTY_KEY, 1, IPC_CREAT | 0666);
    int sem_full = semget(SEM_FULL_KEY, 1, IPC_CREAT | 0666);

    // Initialize semaphores
    semctl(sem_mutex, 0, SETVAL, 1);
    semctl(sem_empty, 0, SETVAL, BUFFER_SIZE);
    semctl(sem_full, 0, SETVAL, 0);

    shared_data->count = 0;

    for (int i = 0; i < 10; ++i) {
        wait(sem_empty);
        wait(sem_mutex);
        std::cout << "Accuire mutex" << std::endl;

        int index = shared_data->count % BUFFER_SIZE;
        shared_data->buffer[index] = i;
        shared_data->count++;
        std::cout << "Produced: " << i << std::endl;

        std::cout << "Release mutex" << std::endl;
        signal(sem_full);
        signal(sem_mutex);

        sleep(1);
    }

    while(true);
    shmdt(shared_data);
    return 0;
}

