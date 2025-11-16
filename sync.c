#include "dir_manage.h"

pthread_mutex_t fileLock;
sem_t fileSemaphore;

// Initialize mutex & semaphore
void initSyncMechanisms() {
    pthread_mutex_init(&fileLock, NULL);
    sem_init(&fileSemaphore, 0, 1);  // Binary semaphore
}

// Destroy them at exit
void destroySyncMechanisms() {
    pthread_mutex_destroy(&fileLock);
    sem_destroy(&fileSemaphore);
}

// Helper functions
void lockFileOps() {
    pthread_mutex_lock(&fileLock);
}

void unlockFileOps() {
    pthread_mutex_unlock(&fileLock);
}

void waitSemaphore() {
    sem_wait(&fileSemaphore);
}

void postSemaphore() {
    sem_post(&fileSemaphore);
}
