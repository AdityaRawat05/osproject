#include "dir_manage.h"

int main() {
    char path[512];
    int choice;
    char pattern[128];

    printf("Enter directory path: ");
    scanf("%s", path);

    do {
        printf("\nChoose an option:\n");
        printf("1. List & Sort Directory\n");
        printf("2. Search by Name/Extension\n");
        printf("3. Delete using SRU Filtering\n");
        printf("4. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                int sortChoice;
                printf("Select sorting option:\n");
                printf("1. Sort by Name\n");
                printf("2. Sort by Size\n");
                printf("3. Sort by Last Modified Date\n");
                printf("Enter choice: ");
                scanf("%d", &sortChoice);

                listAndSortDirectory(path, sortChoice);
                break;
            }

            case 2:
                printf("Enter name or extension to search (e.g., .c or file): ");
                scanf("%s", pattern);
                searchByNameOrExtension(path, pattern);
                break;

            case 3:
                deleteBySRUFilter(path);
                break;

            case 4:
                printf("Exiting program.\n");
                break;

            default:
                printf("Invalid choice! Try again.\n");
        }

    } while (choice != 4);

    return 0;
}
