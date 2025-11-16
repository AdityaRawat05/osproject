#ifndef DIR_MANAGE_H
#define DIR_MANAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <limits.h>

// ===========================================================
// FILE INFO STRUCT
// ===========================================================
typedef struct FileInfo {
    char name[256];
    off_t size;
    char owner[64];
    char group[64];
    time_t modified;
} FileInfo;

// ===========================================================
// GLOBAL SYNC VARIABLES (DEFINED IN sync.c)
// ===========================================================
extern pthread_mutex_t fileLock;     // Mutex for critical sections
extern sem_t fileSemaphore;          // Semaphore for concurrency control

// ===========================================================
// SYNC FUNCTIONS (FROM sync.c)
// ===========================================================
void initSyncMechanisms();
void destroySyncMechanisms();

void lockFileOps();
void unlockFileOps();

void waitSemaphore();
void postSemaphore();

// ===========================================================
// DIRECTORY MANAGEMENT MODULE
// ===========================================================
void listAndSortDirectory(const char *path, int sortChoice);
void searchByNameOrExtension(const char *path, const char *pattern);
void deleteBySRUFilter(const char *path);

// Tree view module (we will add later)
// void printDirectoryTree(const char *path, int level, int isLast[]);

// ===========================================================
// REPORT MODULE (report.c)
// ===========================================================
void exportReportTXT(const char *path, const char *outfile);
void exportReportCSV(const char *path, const char *outfile);

// Append delete logs (optional)
void appendSRULog(const char *action, const char *filepath, long size, const char *owner);

// Export both TXT + CSV together
void exportAllReports(const char *path);

// ===========================================================
// FILE OPERATIONS MODULE (fileops.c)
// ===========================================================
int copyFile(const char *src, const char *dst);
int moveFile(const char *src, const char *dst);
int renameFile(const char *oldpath, const char *newpath);
int deleteFile(const char *path);
int createDirectory(const char *path, mode_t mode);
int removeDirectoryRecursive(const char *path);

// Interactive file operations utility
void fileOperationsMenu(const char *cwd);

// ===========================================================
// MAIN MENU
// ===========================================================
void menu();

#endif
