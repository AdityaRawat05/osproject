// fileops.c
#define _GNU_SOURCE
#include "dir_manage.h"
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

/* Copy file using read/write loop. Preserves file permissions. */
int copyFile(const char *src, const char *dst) {
    int in_fd = -1, out_fd = -1;
    ssize_t nread;
    char buf[65536]; // 64 KB buffer
    struct stat st;

    if (stat(src, &st) != 0) {
        perror("stat(src)");
        return -1;
    }

    in_fd = open(src, O_RDONLY);
    if (in_fd < 0) {
        perror("open(src)");
        return -1;
    }

    /* create dst with same mode */
    out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (out_fd < 0) {
        perror("open(dst)");
        close(in_fd);
        return -1;
    }

    while ((nread = read(in_fd, buf, sizeof(buf))) > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;
        do {
            nwritten = write(out_fd, out_ptr, nread);
            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else if (errno != EINTR) {
                perror("write");
                close(in_fd);
                close(out_fd);
                return -1;
            }
        } while (nread > 0);
    }
    if (nread < 0) {
        perror("read");
        close(in_fd);
        close(out_fd);
        return -1;
    }

    /* preserve times */
    struct timespec times[2];
#ifdef __APPLE__
    times[0].tv_sec = st.st_atime;
    times[0].tv_nsec = 0;
    times[1].tv_sec = st.st_mtime;
    times[1].tv_nsec = 0;
    /* macOS lacks futimens; skipping timestamp preservation on mac */
#else
    times[0] = (struct timespec){ .tv_sec = st.st_atime, .tv_nsec = 0 };
    times[1] = (struct timespec){ .tv_sec = st.st_mtime, .tv_nsec = 0 };
    futimens(out_fd, times);
#endif

    close(in_fd);
    close(out_fd);
    return 0;
}

/* Move file: try rename() first, otherwise copy+unlink */
int moveFile(const char *src, const char *dst) {
    if (rename(src, dst) == 0) return 0;

    /* rename failed — try copy then unlink */
    if (copyFile(src, dst) != 0) return -1;

    if (unlink(src) != 0) {
        perror("unlink(src) after copy");
        return -1;
    }
    return 0;
}

/* Rename (wrapper) */
int renameFile(const char *oldpath, const char *newpath) {
    if (rename(oldpath, newpath) == 0) return 0;
    perror("rename");
    return -1;
}

/* Thread-safe delete single file */
int deleteFile(const char *path) {
    int rc;
    lockFileOps();
    rc = remove(path);
    if (rc != 0) perror("remove");
    unlockFileOps();
    return rc;
}

/* Create directory (recursive not implemented here) */
int createDirectory(const char *path, mode_t mode) {
    if (mkdir(path, mode) == 0) return 0;
    if (errno == EEXIST) return 0;
    perror("mkdir");
    return -1;
}

/* Recursively remove directory contents then directory itself */
static int removeDirContents(const char *path) {
    struct dirent *de;
    struct stat st;
    char child[PATH_MAX];
    DIR *dr = opendir(path);
    if (!dr) {
        perror("opendir in removeDirContents");
        return -1;
    }

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;

        snprintf(child, sizeof(child), "%s/%s", path, de->d_name);
        if (lstat(child, &st) != 0) {
            perror("lstat");
            closedir(dr);
            return -1;
        }

        if (S_ISDIR(st.st_mode)) {
            if (removeDirContents(child) != 0) {
                closedir(dr);
                return -1;
            }
            /* remove the now-empty subdir */
            lockFileOps();
            if (rmdir(child) != 0) {
                perror("rmdir");
                unlockFileOps();
                closedir(dr);
                return -1;
            }
            unlockFileOps();
        } else {
            lockFileOps();
            if (remove(child) != 0) {
                perror("remove (file)");
                unlockFileOps();
                closedir(dr);
                return -1;
            }
            unlockFileOps();
        }
    }

    closedir(dr);
    return 0;
}

int removeDirectoryRecursive(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("stat");
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s is not a directory\n", path);
        return -1;
    }

    if (removeDirContents(path) != 0) return -1;

    /* now remove the top directory */
    lockFileOps();
    if (rmdir(path) != 0) {
        perror("rmdir top");
        unlockFileOps();
        return -1;
    }
    unlockFileOps();
    return 0;
}

/* ---------------------------
   Interactive fileops menu
   --------------------------- */
void fileOperationsMenu(const char *cwd) {
    int ch;
    char src[PATH_MAX], dst[PATH_MAX], name[PATH_MAX];

    while (1) {
        printf("\n----- File Operations Menu (cwd: %s) -----\n", cwd);
        printf("1. Copy file\n");
        printf("2. Move file\n");
        printf("3. Rename file\n");
        printf("4. Delete file\n");
        printf("5. Create directory\n");
        printf("6. Remove directory (recursive)\n");
        printf("7. Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &ch) != 1) { while (getchar()!='\n'); continue; }

        switch (ch) {
            case 1:
                printf("Source path: "); scanf("%s", src);
                printf("Destination path: "); scanf("%s", dst);
                if (copyFile(src, dst) == 0) printf("Copy successful\n");
                break;
            case 2:
                printf("Source path: "); scanf("%s", src);
                printf("Destination path: "); scanf("%s", dst);
                if (moveFile(src, dst) == 0) printf("Move successful\n");
                break;
            case 3:
                printf("Old path: "); scanf("%s", src);
                printf("New path: "); scanf("%s", dst);
                if (renameFile(src, dst) == 0) printf("Rename successful\n");
                break;
            case 4:
                printf("Path to delete: "); scanf("%s", src);
                if (deleteFile(src) == 0) printf("Delete successful\n");
                break;
            case 5:
                printf("Directory path to create: "); scanf("%s", name);
                if (createDirectory(name, 0755) == 0) printf("Directory created\n");
                break;
            case 6:
                printf("Directory path to remove recursively: "); scanf("%s", name);
                printf("Are you sure? This will remove all contents. [y/n]: ");
                {
                    char conf; scanf(" %c", &conf);
                    if (conf == 'y' || conf == 'Y') {
                        if (removeDirectoryRecursive(name) == 0) printf("Directory removed\n");
                    } else printf("Cancelled\n");
                }
                break;
            case 7:
                return;
            default:
                printf("Invalid option\n");
        }
    }
}
