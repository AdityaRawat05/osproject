// report.c
#include "dir_manage.h"

#define MAX_FILES 20000

// Helper: recursively collect files under 'path' into files[] (up to max_files).
// Returns number of files collected.
static int collectFilesRecursive(const char *path, FileInfo files[], int max_files, int *index) {
    struct dirent *de;
    struct stat st;
    char fullPath[1024];

    DIR *dr = opendir(path);
    if (!dr) {
        // perror("Unable to open directory (collectFilesRecursive)");
        return *index;
    }

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, de->d_name);

        if (stat(fullPath, &st) != 0)
            continue;

        if (S_ISDIR(st.st_mode)) {
            // recurse into subdirectory
            collectFilesRecursive(fullPath, files, max_files, index);
        } else if (S_ISREG(st.st_mode)) {
            if (*index >= max_files) {
                closedir(dr);
                return *index;
            }
            // fill FileInfo
            strncpy(files[*index].name, fullPath, sizeof(files[*index].name)-1);
            files[*index].name[sizeof(files[*index].name)-1] = '\0';
            files[*index].size = st.st_size;

            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);
            strncpy(files[*index].owner, pw ? pw->pw_name : "unknown", sizeof(files[*index].owner)-1);
            files[*index].owner[sizeof(files[*index].owner)-1] = '\0';
            strncpy(files[*index].group, gr ? gr->gr_name : "unknown", sizeof(files[*index].group)-1);
            files[*index].group[sizeof(files[*index].group)-1] = '\0';

            files[*index].modified = st.st_mtime;
            (*index)++;
        }
    }

    closedir(dr);
    return *index;
}

// Public: export TXT report (human readable)
void exportReportTXT(const char *path, const char *outfile) {
    FileInfo *files = malloc(sizeof(FileInfo) * MAX_FILES);
    if (!files) {
        fprintf(stderr, "Memory allocation failed for files array\n");
        return;
    }
    int count = 0;
    collectFilesRecursive(path, files, MAX_FILES, &count);

    lockFileOps();
    FILE *fp = fopen(outfile, "w");
    if (!fp) {
        perror("Unable to create TXT report");
        unlockFileOps();
        free(files);
        return;
    }

    fprintf(fp, "Directory Snapshot Report for: %s\n", path);
    fprintf(fp, "Generated on: %s", ctime(&(time_t){time(NULL)}));
    fprintf(fp, "Total files: %d\n", count);
    fprintf(fp, "--------------------------------------------------------------------------------\n");
    fprintf(fp, "%-6s %-80s %-12s %-12s %-24s\n", "Index", "Path", "Size(B)", "Owner", "Last Modified");
    fprintf(fp, "--------------------------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        char timestr[64];
        struct tm tm;
        localtime_r(&files[i].modified, &tm);
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm);

        fprintf(fp, "%-6d %-80s %-12ld %-12s %-24s\n",
                i + 1,
                files[i].name,
                (long)files[i].size,
                files[i].owner,
                timestr);
    }

    fprintf(fp, "--------------------------------------------------------------------------------\n");
    fclose(fp);
    unlockFileOps();

    printf("TXT report generated: %s (files: %d)\n", outfile, count);
    free(files);
}

// Public: export CSV report (path, size, owner, group, modified_epoch)
void exportReportCSV(const char *path, const char *outfile) {
    FileInfo *files = malloc(sizeof(FileInfo) * MAX_FILES);
    if (!files) {
        fprintf(stderr, "Memory allocation failed for files array\n");
        return;
    }
    int count = 0;
    collectFilesRecursive(path, files, MAX_FILES, &count);

    lockFileOps();
    FILE *fp = fopen(outfile, "w");
    if (!fp) {
        perror("Unable to create CSV report");
        unlockFileOps();
        free(files);
        return;
    }

    // CSV header
    fprintf(fp, "path,size_bytes,owner,group,last_modified_epoch\n");

    for (int i = 0; i < count; i++) {
        fprintf(fp, "\"%s\",%ld,%s,%s,%ld\n",
                files[i].name,
                (long)files[i].size,
                files[i].owner,
                files[i].group,
                (long)files[i].modified);
    }

    fclose(fp);
    unlockFileOps();

    printf("CSV report generated: %s (files: %d)\n", outfile, count);
    free(files);
}

// Append a single SRU activity line to sru_log.txt (thread-safe).
// Example logline format: "2025-10-13 12:34:56,DELETED,/path/to/file,12345,owner"
void appendSRULog(const char *action, const char *filepath, long size, const char *owner) {
    time_t now = time(NULL);
    char timestr[64];
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm);

    lockFileOps();
    FILE *fp = fopen("sru_log.txt", "a");
    if (!fp) {
        perror("Unable to open sru_log.txt for appending");
        unlockFileOps();
        return;
    }
    // action could be "DELETED" or "SKIPPED" etc.
    fprintf(fp, "%s,%s,%s,%ld,%s\n", timestr, action, filepath, size, owner);
    fclose(fp);
    unlockFileOps();
}

// Convenience wrapper: generate both txt and csv with default filenames
void exportAllReports(const char *path) {
    exportReportTXT(path, "report.txt");
    exportReportCSV(path, "report.csv");
}
