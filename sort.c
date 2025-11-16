#include "dir_manage.h"

// -----------------------------------------------------
// Comparison Functions for qsort()
// -----------------------------------------------------

int compareByName(const void *a, const void *b) {
    const FileInfo *f1 = (const FileInfo *)a;
    const FileInfo *f2 = (const FileInfo *)b;
    return strcmp(f1->name, f2->name);
}

int compareBySize(const void *a, const void *b) {
    const FileInfo *f1 = (const FileInfo *)a;
    const FileInfo *f2 = (const FileInfo *)b;

    if (f1->size < f2->size) return -1;
    if (f1->size > f2->size) return 1;
    return 0;
}

int compareByDate(const void *a, const void *b) {
    const FileInfo *f1 = (const FileInfo *)a;
    const FileInfo *f2 = (const FileInfo *)b;

    if (f1->modified < f2->modified) return -1;
    if (f1->modified > f2->modified) return 1;
    return 0;
}

// -----------------------------------------------------
// Main Directory Listing + Sorting Function
// -----------------------------------------------------

void listAndSortDirectory(const char *path, int sortChoice) {
    struct dirent *de;
    struct stat st;
    FileInfo files[1000];
    int count = 0;
    char fullPath[1024];

    DIR *dr = opendir(path);
    if (!dr) {
        perror("Unable to open directory");
        return;
    }

    // -------------------------------------------------
    // Read all directory entries
    // -------------------------------------------------
    while ((de = readdir(dr)) != NULL) {

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, de->d_name);

        if (stat(fullPath, &st) == 0 && S_ISREG(st.st_mode)) {

            strcpy(files[count].name, de->d_name);
            files[count].size = st.st_size;

            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);

            strcpy(files[count].owner, pw ? pw->pw_name : "unknown");
            strcpy(files[count].group, gr ? gr->gr_name : "unknown");
            files[count].modified = st.st_mtime;

            count++;
        }
    }

    closedir(dr);

    // -------------------------------------------------
    // Sorting based on user choice
    // -------------------------------------------------
    switch (sortChoice) {
        case 1:
            qsort(files, count, sizeof(FileInfo), compareByName);
            break;

        case 2:
            qsort(files, count, sizeof(FileInfo), compareBySize);
            break;

        case 3:
            qsort(files, count, sizeof(FileInfo), compareByDate);
            break;

        default:
            qsort(files, count, sizeof(FileInfo), compareByName);
            break;
    }

    // -------------------------------------------------
    // Print Sorted Output
    // Thread-Safe Printing
    // -------------------------------------------------

    printf("\nListing of Directory: %s\n", path);
    printf("-----------------------------------------------------------------------------------------------------\n");
    printf("%-25s %-12s %-12s %-12s %-25s\n", "File Name", "Size(B)", "Owner", "Group", "Last Modified");
    printf("-----------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {

        lockFileOps();  // Thread-safe printing

        printf("%-25s %-12ld %-12s %-12s %-25s",
               files[i].name,
               (long)files[i].size,
               files[i].owner,
               files[i].group,
               ctime(&files[i].modified));

        unlockFileOps();
    }

    printf("-----------------------------------------------------------------------------------------------------\n");
}
