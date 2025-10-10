#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct User {
    int id;
    char name[50];
    int age;
};

void createUser() {
    FILE *fp = fopen("users.txt", "a");
    if (fp == NULL) {
        printf("Sorry !! This file could not be opened \n");
        return;
    }
    struct User u;
    printf("Enter ID: ");
    scanf("%d", &u.id);
    printf("Enter Name: ");
    scanf("%s", u.name);
    printf("Enter Age: ");
    scanf("%d", &u.age);
    fprintf(fp, "%d %s %d\n", u.id, u.name, u.age);
    fclose(fp);
    printf("User added \n");
}

void readUsers() {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) {
        printf("There is No user , File doesn't exist yet  \n");
        return;
    }
    struct User u;
    printf("\n *** User Records ***\n");
    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        printf("ID: %d | Name: %s | Age: %d\n", u.id, u.name, u.age);
    }
    fclose(fp);
}

void updateUser() {
    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL) {
        printf("Sorry !! This file could not be opened \n");
        return;
    }
    int id;
    printf("Enter ID to update: ");
    scanf("%d", &id);

    struct User u;
    int found = 0;
    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        if (u.id == id) {
            printf("Enter new Name: ");
            scanf("%s", u.name);
            printf("Enter new Age: ");
            scanf("%d", &u.age);
            found = 1;
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found) {
        printf("User updated \n");
    } else {
        printf("User with given ID is Not there .\n");
    }
}

void deleteUser() {
    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL) {
        printf("Sorry !! This file could not be opened\n");
        return;
    }
    int id;
    printf("Enter ID to delete: ");
    scanf("%d", &id);
    struct User u;
    int found = 0;

    while (fscanf(fp, "%d %s %d", &u.id, u.name, &u.age) != EOF) {
        if (u.id == id) {
            found = 1;
            continue;
        }
        fprintf(temp, "%d %s %d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found) {
        printf("User deleted \n");
    } else {
        printf("User with given ID is not there \n");
    }
}

int main() {
    int choice;
    while (1) {
        printf("\n    *** User Management System ***  \n");
        printf("1. Create User\n");
        printf("2. Read Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                createUser();
                break;
            case 2:
                readUsers();
                break;
            case 3:
                updateUser();
                break;
            case 4:
                deleteUser();
                break;
            case 5:
                printf("Thank You , Exiting you the Program !!!");
                return 0;
            default:
                printf("Wrong Choice, could you pls Try again.\n");
                break;
        }
    }
    return 0;
}
