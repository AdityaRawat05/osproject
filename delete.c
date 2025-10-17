#include "dir_manage.h"
void deleteBySRUFilter(const char *path) {
    struct dirent *de;
    struct stat st;
    char fullPath[1024];
    DIR *dr = opendir(path);

    if (!dr) {
        perror("Unable to open directory");
        return;
    }

    long sizeLimit;
    int daysOld;
    char userFilter[64];

    printf("\nEnter size limit in bytes (files larger than this will be deleted): ");
    scanf("%ld", &sizeLimit);

    printf("Enter age limit in days (files older than this will be deleted): ");
    scanf("%d", &daysOld);

    printf("Enter username to filter (or 'all' for every user): ");
    scanf("%s", userFilter);

    time_t now = time(NULL);
    int deleted = 0;

    while ((de = readdir(dr)) != NULL) {
        
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, de->d_name);

        if (stat(fullPath, &st) == 0 && S_ISREG(st.st_mode)) {
            struct passwd *pw = getpwuid(st.st_uid);
            if (!pw) continue;

            // Calculate file age
            double ageInDays = difftime(now, st.st_mtime) / (60 * 60 * 24);

            // Apply SRU filters (Size, Recentness, User)
            if ((st.st_size > sizeLimit) &&
                (ageInDays > daysOld) &&
                (strcmp(userFilter, "all") == 0 || strcmp(pw->pw_name, userFilter) == 0)) {

                printf("\nDelete file: %s (Size: %ld bytes, Owner: %s, Age: %.1f days)? [y/n]: ",
                       fullPath, st.st_size, pw->pw_name, ageInDays);
                char confirm;
                scanf(" %c", &confirm);

                if (confirm == 'y' || confirm == 'Y') {
                    if (remove(fullPath) == 0) {
                        printf("Deleted: %s\n", fullPath);
                        deleted++;
                    } else {
                        perror("Error deleting file");
                    }
                } else {
                    printf("Skipped: %s\n", fullPath);
                }
            }
        }
    }

    closedir(dr);
    printf("\nTotal files deleted: %d\n", deleted);
}
