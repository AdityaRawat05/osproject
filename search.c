#include "dir_manage.h"

// -------------------------------------------------------------
// Recursive Search Function
// -------------------------------------------------------------
void searchByNameOrExtension(const char *path, const char *pattern) {
    struct dirent *de;
    char fullPath[1024];

    DIR *dr = opendir(path);
    if (!dr) {
        perror("Unable to open directory");
        return;
    }

    while ((de = readdir(dr)) != NULL) {
        
        // Skip "." and ".."
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        // Build full path
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, de->d_name);

        struct stat st;
        if (stat(fullPath, &st) == 0) {

            // ---------------------------------------------
            // If it is a directory → recursive call
            // ---------------------------------------------
            if (S_ISDIR(st.st_mode)) {
                searchByNameOrExtension(fullPath, pattern);
            } 
            // ---------------------------------------------
            // If it is a file → check match
            // ---------------------------------------------
            else if (S_ISREG(st.st_mode)) {

                // If filename contains pattern (case-sensitive)
                if (strstr(de->d_name, pattern) != NULL) {

                    // Thread-safe output
                    lockFileOps();

                    printf("Found: %s\n", fullPath);

                    unlockFileOps();
                }
            }
        }
    }

    closedir(dr);
}
