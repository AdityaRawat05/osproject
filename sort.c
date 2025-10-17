#include "dir_manage.h"

// Comparison functions
int compareByName(const void *a, const void *b) {
    const struct FileInfo *f1 = (const struct FileInfo *)a;
    const struct FileInfo *f2 = (const struct FileInfo *)b;
    return strcmp(f1->name, f2->name);
}

int compareBySize(const void *a, const void *b) {
    const struct FileInfo *f1 = (const struct FileInfo *)a;
    const struct FileInfo *f2 = (const struct FileInfo *)b;
    return (f1->size > f2->size) - (f1->size < f2->size);
}

int compareByDate(const void *a, const void *b) {
    const struct FileInfo *f1 = (const struct FileInfo *)a;
    const struct FileInfo *f2 = (const struct FileInfo *)b;
    return (f1->modified > f2->modified) - (f1->modified < f2->modified);
}

void listAndSortDirectory(const char *path, int sortChoice) {
    struct dirent *de;
    struct stat st;
    struct FileInfo files[1000];
    int count = 0;
    char fullPath[1024];

    DIR *dr = opendir(path);
    if (!dr) {
        perror("Unable to open directory");
        return;
    }

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, de->d_name);
        if (stat(fullPath, &st) == 0) {
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

    // Sorting choice
    switch (sortChoice) {
        case 1: qsort(files, count, sizeof(struct FileInfo), compareByName); break;
        case 2: qsort(files, count, sizeof(struct FileInfo), compareBySize); break;
        case 3: qsort(files, count, sizeof(struct FileInfo), compareByDate); break;
        default: qsort(files, count, sizeof(struct FileInfo), compareByName); break;
    }

    // Header
    printf("\nListing of Directory: %s\n", path);
    printf("--------------------------------------------------------------------------------------------\n");
    printf("%-25s %-10s %-10s %-10s %-25s\n", "File Name", "Size(B)", "Owner", "Group", "Last Modified");
    printf("--------------------------------------------------------------------------------------------\n");

    // Print details
    for (int i = 0; i < count; i++) {
        printf("%-25s %-10ld %-10s %-10s %-25s",
               files[i].name,
               (long)files[i].size,
               files[i].owner,
               files[i].group,
               ctime(&files[i].modified));
    }
    printf("--------------------------------------------------------------------------------------------\n");
}

// Menu function
void menu() {
    char path[512];
    int choice;

    printf("Enter directory path: ");
    scanf("%s", path);

    do {
        printf("\n==================== Directory Sort Menu ====================\n");
        printf("1. Sort by Name\n");
        printf("2. Sort by Size\n");
        printf("3. Sort by Last Modified Date\n");
        printf("4. Change Directory\n");
        printf("5. Exit\n");
        printf("=============================================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
            case 2:
            case 3:
                listAndSortDirectory(path, choice);
                break;
            case 4:
                printf("Enter new directory path: ");
                scanf("%s", path);
                break;
            case 5:
                printf("Exiting program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 5);
}
