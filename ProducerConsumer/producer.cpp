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

void print_buffer(SharedData* shared_data) {
  std::cout << "Current buffer state: ";
  for (int i = 0; i < shared_data->count; i++) {
    std::cout << shared_data->buffer[i] << " ";
  }
}


int main() {
    // Get shared memory
    int shm_id = shmget(SHM_KEY, sizeof(SharedData), 0666);
    auto *shared_data = static_cast<SharedData *>(shmat(shm_id, nullptr, 0));

    // Get semaphores
    int sem_mutex = semget(SEM_MUTEX_KEY, 1, 0666);
    int sem_empty = semget(SEM_EMPTY_KEY, 1, 0666);
    int sem_full = semget(SEM_FULL_KEY, 1, 0666);

    for (int i = 0; i < 6; ++i) {  // Producing 6 values
        wait(sem_empty);
        wait(sem_mutex);

        int index = shared_data->count % BUFFER_SIZE;
        shared_data->buffer[index] = i;
        shared_data->count++;
        std::cout << "Produced: " << i << "; ";
        print_buffer(shared_data);
        std::cout << std::endl;
      
        signal(sem_mutex);
        signal(sem_full);

        sleep(1);
    }

    shmdt(shared_data);
    return 0;
}

