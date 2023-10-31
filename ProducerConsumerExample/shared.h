#pragma once
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

const key_t SHM_KEY = 1234;
const key_t SEM_MUTEX_KEY = 5678;
const key_t SEM_EMPTY_KEY = 5679;
const key_t SEM_FULL_KEY = 5680;
const int BUFFER_SIZE = 3;

struct SharedData {
    int buffer[BUFFER_SIZE];
    int count;
};

