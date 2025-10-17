#ifndef DIR_MANAGE_H
#define DIR_MANAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

struct FileInfo {
    char name[256];
    off_t size;
    char owner[64];
    char group[64];
    time_t modified;
};

// Function prototypes
void listAndSortDirectory(const char *path, int sortChoice);
void searchByNameOrExtension(const char *path, const char *pattern);
void deleteBySRUFilter(const char *path);
//void printDirectoryTree(const char *path, int level, int isLast[]);
void menu();

#endif
