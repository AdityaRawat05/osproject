#include "dir_manage.h"

// Structure for storing suggested cleanup files
typedef struct {
    char fullPath[1024];
    long size;
    double age;
} Suggestion;

// ===============================================================
// AUTO-GENERATED CLEANUP SUGGESTIONS
// ===============================================================
int generateSuggestions(const char *path, long sizeLimit, int daysOld, Suggestion suggestions[]) {
    struct dirent *de;
    struct stat st;
    char fullPath[1024];
    DIR *dr = opendir(path);

    if (!dr) {
        perror("Unable to open directory");
        return 0;
    }

    int count = 0;
    time_t now = time(NULL);

    while ((de = readdir(dr)) != NULL) {

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, de->d_name);

        if (stat(fullPath, &st) == 0 && S_ISREG(st.st_mode)) {
            double ageInDays = difftime(now, st.st_mtime) / (60 * 60 * 24);

            if (st.st_size > sizeLimit && ageInDays > daysOld) {
                strcpy(suggestions[count].fullPath, fullPath);
                suggestions[count].size = st.st_size;
                suggestions[count].age = ageInDays;
                count++;
            }
        }
    }

    closedir(dr);
    return count;
}

// ===============================================================
// SRU FILTER + MANUAL DELETE BASED ON SUGGESTION LIST
// ===============================================================
void deleteBySRUFilter(const char *path) {

    long sizeLimit;
    int daysOld;

    printf("\nEnter size limit in bytes (suggest files larger than this): ");
    scanf("%ld", &sizeLimit);

    printf("Enter age limit in days (suggest files older than this): ");
    scanf("%d", &daysOld);

    Suggestion suggestions[500];
    int count = generateSuggestions(path, sizeLimit, daysOld, suggestions);

    printf("\n=================== Cleanup Suggestions ===================\n");

    if (count == 0) {
        printf("No files match the cleanup criteria.\n");
        printf("===========================================================\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("%d) %s\n", i + 1, suggestions[i].fullPath);
        printf("   Size: %ld bytes | Age: %.1f days\n", suggestions[i].size, suggestions[i].age);
    }

    printf("===========================================================\n");

    printf("\nDo you want to delete any of these files? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);

    if (confirm != 'y' && confirm != 'Y') {
        printf("Cleanup cancelled.\n");
        return;
    }

    // Manual selection
    int fileIndex;
    printf("Enter the file number to delete (1-%d): ", count);
    scanf("%d", &fileIndex);

    if (fileIndex < 1 || fileIndex > count) {
        printf("Invalid file number.\n");
        return;
    }

    // File to delete
    char *targetFile = suggestions[fileIndex - 1].fullPath;

    // THREAD SAFE DELETE
    lockFileOps();

    if (remove(targetFile) == 0) {
        printf("\nSuccessfully deleted: %s\n", targetFile);
    } else {
        perror("Error deleting file");
    }

    unlockFileOps();

    printf("\nCleanup complete.\n");
}
