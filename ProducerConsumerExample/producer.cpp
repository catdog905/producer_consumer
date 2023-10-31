#include "shared.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <random>


void wait(int sem_id) {
    struct sembuf p = {0, -1, SEM_UNDO};
    if(semop(sem_id, &p, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

void signal(int sem_id) {
    struct sembuf v = {0, 1, SEM_UNDO};
    if(semop(sem_id, &v, 1) == -1) {
        perror("semop wait");
        exit(EXIT_FAILURE);
    }
}

void random_wait(int start, int stop) {
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution(start, stop);

    int wait_time = distribution(generator);

    std::cout << "Waiting for " << wait_time << " milliseconds..." << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
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

    for (int i = 0; i < 6; ++i) {
        random_wait(500, 3000);
        wait(sem_empty);
        wait(sem_mutex);
        std::cout << "Accuire mutex" << std::endl;

        int index = shared_data->count % BUFFER_SIZE;
        shared_data->buffer[index] = i;
        shared_data->count++;
        std::cout << "Produced: " << i << std::endl;

        signal(sem_full);
        signal(sem_mutex);
        std::cout << "Released mutex" << std::endl;
    }

    while(true);
    shmdt(shared_data);
    return 0;
}

