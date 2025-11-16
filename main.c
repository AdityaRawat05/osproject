#include "dir_manage.h"

int main() {
    char path[512];
    int choice;
    char pattern[128];

    // ---------------------------------------------------------
    // Initialize Synchronization (mutex + semaphore)
    // ---------------------------------------------------------
    initSyncMechanisms();

    printf("Enter directory path: ");
    scanf("%s", path);

    do {
        printf("\n=================== Directory Management System ===================\n");
        printf("1. List & Sort Directory\n");
        printf("2. Search by Name/Extension\n");
        printf("3. Delete using SRU Filtering\n");
        printf("4. Generate Report (TXT + CSV)\n");
        printf("5. File Operations (Copy/Move/Rename/Delete)\n");
        printf("6. Exit\n");
        printf("===================================================================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {

            case 1: {
                int sortChoice;
                printf("\nSelect sorting option:\n");
                printf("1. Sort by Name\n");
                printf("2. Sort by Size\n");
                printf("3. Sort by Last Modified Date\n");
                printf("Enter choice: ");
                scanf("%d", &sortChoice);

                listAndSortDirectory(path, sortChoice);
                break;
            }

            case 2:
                printf("\nEnter name or extension to search (e.g., .c or file): ");
                scanf("%s", pattern);
                searchByNameOrExtension(path, pattern);
                break;

            case 3:
                deleteBySRUFilter(path);
                break;

            case 4:
                exportAllReports(path);
                break;

            case 5:
                fileOperationsMenu(path);
                break;

            case 6:
                printf("\nExiting program...\n");
                break;

            default:
                printf("\nInvalid choice! Try again.\n");
        }

    } while (choice != 6);

    // ---------------------------------------------------------
    // Cleanup Synchronization Before Exit
    // ---------------------------------------------------------
    destroySyncMechanisms();

    return 0;
}
