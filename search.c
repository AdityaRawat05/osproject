#include "dir_manage.h"
void searchByNameOrExtension(const char *path, const char *pattern) {
    struct dirent *de;
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

        struct stat st;
        if (stat(fullPath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
              
                searchByNameOrExtension(fullPath, pattern);
            } else {
                if (strstr(de->d_name, pattern))
                    printf("Found: %s\n", fullPath);
            }
        }
    }
    closedir(dr);
}
