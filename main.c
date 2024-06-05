#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdbool.h>
#include "cjson/cJSON.h"

void adminMenu(){
    printf("admin menu");
}

void staffMenu(){
    printf("staff menu");
}

void customerMenu(){
    printf("customer menu");
}

cJSON* logIn(cJSON *json){
    //initialize variables
    char enteredUsername[16];
    char enteredPassword[16];
    
    char passmask;

    int attempts = 3;
    bool isLoggedIn = false;

    //reading users
    cJSON *users = cJSON_GetObjectItem(json, "users");

    while (attempts + 1 > 0 && !isLoggedIn){
        //prompt user credentials
        printf("Enter your username: ");
        scanf("%15s", enteredUsername);

        printf("Enter your password: ");
        int i = 0;
        // mask password
        while ((passmask = _getch()) != 13){
            if (passmask == 8){
                printf("\b \b"); 
                i--;
            }
            else {
                printf("*");
                enteredPassword[i] = passmask;
                i++;
            }
        }
        enteredPassword[i] = '\0';

        if (users) {
            int users_count = cJSON_GetArraySize(users);
            for (int i = 0; i < users_count; i++) {
                cJSON *user = cJSON_GetArrayItem(users, i);

                cJSON *username = cJSON_GetObjectItem(user, "username");
                cJSON *password = cJSON_GetObjectItem(user, "password");
                cJSON *fullname = cJSON_GetObjectItem(user, "fullname");
                cJSON *role = cJSON_GetObjectItem(user, "role");

                char *firstname = strtok(cJSON_GetStringValue(fullname), " ");

                if (strcmp(cJSON_GetStringValue(username), enteredUsername) == 0 &&
                    strcmp(cJSON_GetStringValue(password), enteredPassword) == 0) {

                    system("cls");
                    printf("\nHello again %s!\n", firstname);
                    
                    if (strcmp(cJSON_GetStringValue(role), "1") == 0){
                        isLoggedIn = true;
                        adminMenu();
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "2") == 0){
                        isLoggedIn = true;
                        staffMenu();
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "3") == 0){
                        isLoggedIn = true;
                        customerMenu();
                    }
                    else {
                        printf("\nError reading user role!\n");
                    }
                }
            }
        } else {
            printf("\nError reading user data!\n");
        }

        if (!isLoggedIn){
            if (attempts == 0){
                printf("\nNo more attempts, try again later!");
                break;
            }
            else {
                printf("\n%s %s Invalid username or password! Try again (%d)\n", enteredPassword, enteredUsername, attempts);
                printf("Entered Password Length: %zu\n", strlen(enteredPassword));
                attempts--;
            }
        }
    }
    free(users);
    //return NULL; //if needed
}


int main() {

    //file path
    const char *filename = "data.txt";

    //open file
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file!\n");
        return 1;
    }

    //find file length
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    //allocate memory (+1 for null)
    char *content = (char *)malloc(length + 1);
    if (!content) {
        printf("Memory allocation failed!\n");
        fclose(file);
        return 1;
    }

    //read file content
    fread(content, 1, length, file);
    fclose(file);
    content[length] = '\0'; // Null-terminate the string

    //parse json
    cJSON *json = cJSON_Parse(content);
    if (!json) {
        printf("Error parsing JSON!\n");
        free(content);
        return 1;
    }

    logIn(json);

    // cJSON *user = logIn(json);

    // //printing parsed json
    // char *printed_json = cJSON_Print(json);
    // printf("%s\n", printed_json);

    // cJSON *rooms = cJSON_GetObjectItem(json, "rooms");
    // if (rooms){
    //     int rooms_count = cJSON_GetArraySize(rooms);
    //     for (int i =0; i < rooms_count; i++){
    //         cJSON *room = cJSON_GetArrayItem(rooms, i);
            
    //         cJSON *roomID = cJSON_GetObjectItem(room, "ID");
    //         cJSON *room_floor = cJSON_GetObjectItem(room, "room_floor");
    //         cJSON *room_type = cJSON_GetObjectItem(room, "type");
    //         cJSON *room_number = cJSON_GetObjectItem(room, "room_number");
    //         cJSON *room_price = cJSON_GetObjectItem(room, "price");
    //         cJSON *room_status = cJSON_GetObjectItem(room, "status");

    //         printf("RoomID: %s\n", roomID->valuestring);
    //         printf("Room: %s - %s\n", room_number->valuestring, room_floor->valuestring);
    //         printf("Type: %s\n", room_type->valuestring);
    //         printf("Price: %s\n", room_price->valuestring);
    //         printf("Status: %s\n\n", room_status->valuestring);
    //     }
    // } else {
    //     printf("No room data found!");
    // }

    //clean memory up
    cJSON_Delete(json);
    free(content);

    return 0;
}
