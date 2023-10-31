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
    // Get shared memory
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    auto *shared_data = static_cast<SharedData *>(shmat(shm_id, nullptr, 0));

    // Get semaphores
    int sem_mutex = semget(SEM_MUTEX_KEY, 1, 0666);
    int sem_empty = semget(SEM_EMPTY_KEY, 1, 0666);
    int sem_full = semget(SEM_FULL_KEY, 1, 0666);

    for (int i = 0; i < 100; ++i) {
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

        sleep(2);
    }

    shmdt(shared_data);
    return 0;
}

