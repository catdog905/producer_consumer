#include "shared.h"
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

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

    pid_t pids[5];

    // Create 2 producer processes
    for (int i = 0; i < 2; ++i) {
        if ((pids[i] = fork()) < 0) {
            perror("fork");
            exit(1);
        } else if (pids[i] == 0) {
            execl("./producer", "producer", std::to_string(i+1).c_str(), nullptr);
            perror("execl");
            exit(1);
        }
    }

    // Create 3 consumer processes
    for (int i = 0; i < 3; ++i) {
        if ((pids[i + 2] = fork()) < 0) {
            perror("fork");
            exit(1);
        } else if (pids[i + 2] == 0) {
            execl("./consumer", "consumer", std::to_string(i+1).c_str(), nullptr);
            perror("execl");
            exit(1);
        }
    }

    // Wait for all child processes to finish
    for (auto &pid : pids) {
        waitpid(pid, nullptr, 0);
    }

    // Detach and remove shared memory
    shmdt(shared_data);
    shmctl(shm_id, IPC_RMID, nullptr);

    // Remove semaphores
    semctl(sem_mutex, 0, IPC_RMID);
    semctl(sem_empty, 0, IPC_RMID);
    semctl(sem_full, 0, IPC_RMID);

    std::cout << "Main process finished." << std::endl;
    return 0;
}

