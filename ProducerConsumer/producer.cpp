#include "shared.h"
#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <chrono>
#include <thread>
#include <random>

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

void random_wait(int start, int stop) {
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> distribution(start, stop);

    int wait_time = distribution(generator);

    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
}

int main(int argc, char* argv[]) {
    int producer_id = argc > 1 ? std::stoi(argv[1]) : 1;

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
        auto now = std::chrono::system_clock::now();
        time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::cout << std::put_time(std::localtime(&now_c), "%F %T") << "; ";
        std::cout << "Producer{" << producer_id  << "}; Produced: " << i << "; ";
        print_buffer(shared_data);
        std::cout << std::endl;
      
        signal(sem_mutex);
        signal(sem_full);

        random_wait(1000, 1500);
    }

    shmdt(shared_data);
    return 0;
}

