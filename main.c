#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "cjson/cJSON.h"

void adminMenu(){
    printf("admin");
}

void staffMenu(){
    printf("staff");
}

void customerMenu(){
    printf("customer");
}

cJSON* logIn(cJSON *json){
    //initalize variables
    char enteredUsername[16];
    char enteredPassword[16];
    int i = 0;
    char passmask;

    //prompt user credentials
    printf("Enter your username: ");
    scanf("%s", &enteredUsername);

    printf("Enter your password: ");
    // mask password
    while ((passmask = _getch()) != 13){
        if (passmask == 8){
            i--;
            printf("\b \b"); 
        }
        else {
            printf("*");
            enteredPassword[i] = passmask;
            i++;
        }
    }
    enteredPassword[i];

    //check user credentials
    cJSON *users = cJSON_GetObjectItem(json, "users");
    if (users) {
        int users_count = cJSON_GetArraySize(users);
        for (int i = 0; i < users_count; i++) {
            cJSON *user = cJSON_GetArrayItem(users, i);

            cJSON *userID = cJSON_GetObjectItem(user, "userID");
            cJSON *fullname = cJSON_GetObjectItem(user, "fullname");
            cJSON *username = cJSON_GetObjectItem(user, "username");
            cJSON *password = cJSON_GetObjectItem(user, "password");
            cJSON *role = cJSON_GetObjectItem(user, "role");
        

            if (strcmp(cJSON_GetStringValue(username), enteredUsername) == 0 &&
                strcmp(cJSON_GetStringValue(password), enteredPassword) == 0) {
                // printf("\nUser found!\n");
                return user;
                break;
            }
    }
    } else {
        printf("\nNo users found!\n");
    }

    free(users);
}

int main() {

    //file path
    const char *filename = "data.txt";

    //open file
    FILE *file = fopen(filename, "r");

    //find file length
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    //allocate memory (+1 for null)
    char *content = (char *)malloc(length + 1);

    //read file content
    fread(content, 1, length, file);
    fclose(file);
    content[length] = '\0'; // Null-terminate the string

    //parse json
    cJSON *json = cJSON_Parse(content);

    cJSON *user = logIn(json);
    if (user) {
        // Use the returned user object
        cJSON *fullname = cJSON_GetObjectItem(user, "fullname");
        cJSON *role = cJSON_GetObjectItem(user, "role");

        printf("Full name: %s\n", cJSON_GetStringValue(fullname));
        printf("Role: %s\n", cJSON_GetStringValue(role));
    } else {
        printf("\nInvalid username or password.\n");
    }
    
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
