#include "shared.h"
#include <iostream>
#include <unistd.h>
#include <random>
#include <thread>
#include <chrono>

void wait(int sem_id) {
    struct sembuf p = {0, -1, SEM_UNDO};
    if (semop(sem_id, &p, 1) == -1) {
      perror("semop - P");
      exit(EXIT_FAILURE);
    }
}

void signal(int sem_id) {
    struct sembuf v = {0, 1, SEM_UNDO};
    if (semop(sem_id, &v, 1) == -1) {
      perror("semop - P");
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
    // Get shared memory
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    auto *shared_data = static_cast<SharedData *>(shmat(shm_id, nullptr, 0));

    // Get semaphores
    int sem_mutex = semget(SEM_MUTEX_KEY, 1, 0666);
    int sem_empty = semget(SEM_EMPTY_KEY, 1, 0666);
    int sem_full = semget(SEM_FULL_KEY, 1, 0666);

    for (int i = 0; i < 4; ++i) {
        random_wait(500, 3000);
        wait(sem_full);
        std::cout << "Decrease full sem: " << std::endl;
        wait(sem_mutex);
        std::cout << "Accuire mutex:" << std::endl;
  
        std::cout << shared_data->count << std::endl;
        int index = shared_data->count - 1;
        int item = shared_data->buffer[index];
        shared_data->count--;
        std::cout << "Consumed: " << item << std::endl;

        signal(sem_empty);
        signal(sem_mutex);
        std::cout << "Increase empty sem: " << std::endl;
    }

    shmdt(shared_data);
    return 0;
}

