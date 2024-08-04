#include <stdio.h>
#include <stdlib.h> //free malloc exit
#include <conio.h> //getch
#include <time.h>
#include <string.h> //strcmp strlen
#include <stdbool.h> //for booleans
#include "cjson/cJSON.h" //reading json
#include <ctype.h>

int generalBookingIDCounter;
int generalCounterBooking(cJSON *json)
{
    cJSON* bookings = cJSON_GetObjectItem(json, "bookings");
    int bookingsNumber = cJSON_GetArraySize(bookings);

    for (int i = 0; i < bookingsNumber; i++) {
        cJSON* booking = cJSON_GetArrayItem(bookings, i);
        generalBookingIDCounter = i + 2;
    }


    return generalBookingIDCounter;
}


cJSON* viewCurrentBooking(cJSON *bookings, int bookingsNumber, int userID) {
    cJSON* currentBooking = NULL;
    int currentBookingFind = -1;

    for (int i = 0; i < bookingsNumber; i++) {
        cJSON* booking = cJSON_GetArrayItem(bookings, i);
        if (cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(booking, "customerID")) == userID) {
            currentBookingFind = i;
            currentBooking = booking;
        }   
    }

    if (currentBooking != NULL)
    {
        printf("Your Current Bookings are:\n");
        printf("-------------------------------------------------------------\n");
        printf("|%10s|%15s|%10s|%10s|%10s|\n", "Booking ID", "Room ID", "Check-In", "Check-Out", "Status");
        printf("-------------------------------------------------------------\n");
        printf("|%10d|%15s|%10s|%10s|%10s|\n", 
            (int)cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(currentBooking, "bookingID")),
            cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(currentBooking, "roomID")),
            cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(currentBooking, "checkIn")),
            cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(currentBooking, "checkOut")),
            cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(currentBooking, "status")));
        printf("-------------------------------------------------------------\n");
    } else
    {
        printf("There are no current bookings.\n");
    }
}

cJSON* writeFile(cJSON *json){
    FILE *fp = fopen("data.txt", "w+"); 
    
    // write into file
   char *json_str = cJSON_Print(json);
   fputs(json_str, fp); 
   fclose(fp);
   cJSON_free(json_str); 
}

cJSON* editProfile(cJSON *json, cJSON *user, cJSON *users){
    printf("Editing profile...\n");

    char newPassword[16];

    cJSON *old_password = cJSON_GetObjectItem(user, "password");

    while (1) {    
        printf("Enter your new password: ");
        scanf("%s", &newPassword);

        cJSON* new_password = cJSON_CreateString(newPassword);
    
        if (strlen(newPassword) > 16){
            printf("Password should not be more than 16 characters!\n");
        }
        else
        {
            if (strlen(newPassword) < 8){ 
                printf("Password should be at least 8 characters!\n");
            }
            else
            {
                if (strcmp(newPassword, cJSON_GetStringValue(old_password)) == 0){ 
                    printf("password similar to the old one!\n");
                    
                }
                else
                {
                    cJSON_ReplaceItemInObject(user, "password", new_password);
                    writeFile(json);
                    printf("Password successfully changed!\n");
                    break;
                }
            }
        }
        
    }

    int users_count = cJSON_GetArraySize(users);
    for (int i =0; i < users_count; i++){
        cJSON *finding_user = cJSON_GetArrayItem(user, i);

        cJSON *username = cJSON_GetObjectItem(user, "username");
        
        cJSON *finding_username = cJSON_GetObjectItem(finding_user, "username"); 
    }
}

cJSON* adminMenu(cJSON *json, cJSON *users){
    char room_ID[50];
    int choice;
    char  fullname_new [100] , username_new[100] , password_new[100], role_new [100] ; 
    char userID_existing [50]; 
    int manage_hotel_rooms_choice;
    char log_out_request [50];
    char status_new;
    char new_room_type[100] , new_room_floor[100];
    int new_room_no , new_room_price;

    printf("admin menu\n");
    printf("1 - Register new users\n");
    printf("2 - Manage existing users\n");
    printf("3 - Manage hotel rooms\n");
    printf("4 - Log out\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
   
    cJSON* rooms = cJSON_GetObjectItem(json, "rooms");
   switch (choice)
   {
   case 1:
        printf("You selected 'Register new usersss'\n");
        printf("Enter the full name of the user: ");
        scanf("%s", fullname_new);
        printf("Enter a username of the user: ");
        scanf("%s", username_new);
        printf("Enter a password of the user: ");
        scanf("%s", password_new);
        printf("Enter the role of the user: ");
        scanf("%s", role_new);
        

        cJSON *new_user = cJSON_CreateObject();
        cJSON_AddStringToObject(new_user, "fullname", fullname_new);
        cJSON_AddStringToObject(new_user, "username", username_new);
        cJSON_AddStringToObject(new_user, "password", password_new);
        cJSON_AddStringToObject(new_user, "role", role_new);

        cJSON_ReplaceItemInArray(users ,0 , new_user );

        writeFile(json);
       


               //update data//
               // generate userID// 
               \

               
//                char* generateUserID(cJSON *users) {
//             int userCount = cJSON_GetArraySize(users);
//             char *userID = (char *)malloc(8 * sizeof(char)); // Assuming "UIDXXX" format
//             sprintf(userID, "UID%03d", userCount + 1); // Incrementing count for new user
//             return userID;
            
// }
//             char *newUserID = generateUserID(users);
//             printf("%s\n", newUserID);

               
               break;
               

        

     case 2:
        printf(" you selected manage exisitng users\n");
        
        // printf ("enter tha userID of the user:");
        //     scanf ("%s" , &userID_existing);
        // // read userID from 4edata //
        // if (users) {
        //     int users_count = cJSON_GetArraySize(users);
        //     for (int i = 0; i < users_count; i++) {
        //         cJSON *user = cJSON_GetArrayItem(users, i);
        //         cJSON *userID = cJSON_GetObjectItem(user, "userID");
                
        //         if (strcmp(cJSON_GetStringValue(userID), userID_existing) == 0){
                    
        //         printf ("this user exists");
        //         }
        //         //edit status to be reserved or checked in or null//
        //          printf("Enter new status (reserved/checked in/null): ");
        //     scanf ("%s", &status_new);

        //     if (strcmp(status_new, "null") == 0) {
        //         cJSON_ReplaceItemInObject(users, "status", cJSON_CreateNull());
        //     } else {
        //         cJSON_ReplaceItemInObject(users, "status", cJSON_CreateString(status_new));
        //     }
        //     //make sure it update//
        //     break;
        //     }
        // }


    break;

     case 3:
            printf ("you slected manage hotel rooms\n");
            
            printf ("1-add hotel room details\n");
            printf ("2-update hotel room details\n");
            printf ("3-remove hotel room details");
            scanf ("%d" , &manage_hotel_rooms_choice);
            switch (manage_hotel_rooms_choice) {
                
                
                case 1:
                
                    printf("Enter the ID for the room\n");
                    scanf ("%s", &room_ID);

                    int room_exists = 0;
                    
                   if (rooms) {
                    int rooms_count = cJSON_GetArraySize(rooms);

                    // Check if room with given ID already exists
                    for (int i = 0; i < rooms_count; i++) {
                        cJSON *room = cJSON_GetArrayItem(rooms, i);
                        cJSON *ID = cJSON_GetObjectItem(room, "ID");

                        if (strcmp(cJSON_GetStringValue(ID), room_ID) == 0) {
                            room_exists = 1;
                            printf("Room with ID '%s' already exists.\n", room_ID);
                            break;
                                }   
                    }
                        }
                            if (!room_exists) {
                            
                            //add hotel room details//
                            
                            printf ("enter new room number:\n");
                            scanf ("%d", &new_room_no);
                            printf ("enter new room floor:\n");
                            scanf ("%s", &new_room_floor);
                            printf ("enter new room type:\n");
                            scanf ("%s", &new_room_type);
                            printf ("enter new room price:\n");
                            scanf ("%d", &new_room_price);
                            
                            cJSON *new_room = cJSON_CreateObject();
                            cJSON_AddNumberToObject(new_room, "room_number", new_room_no);
                            cJSON_AddStringToObject(new_room, "room_floor", new_room_floor);
                            cJSON_AddStringToObject(new_room, "type", new_room_type);
                            cJSON_AddNumberToObject(new_room, "price", new_room_price);
                            char *json_new_room = cJSON_Print(new_room);
                            printf("%s\n", json_new_room);
                            cJSON *users = cJSON_CreateArray();
                            cJSON_AddItemToArray(rooms, new_room);


                            char *updated_rooms_json = cJSON_Print(rooms);
                            printf("%s\n", updated_rooms_json);

                            break;
                            }

                            //generate ID based on type and number//         
        
                case 2:
                    printf("Enter the ID for the room\n");
                    scanf ("%s", &room_ID);
                    
                    if (rooms) {
                        int rooms_count = cJSON_GetArraySize(rooms);
                        for (int i = 0; i < rooms_count; i++) {
                            cJSON *room = cJSON_GetArrayItem(rooms, i);
                            cJSON *ID = cJSON_GetObjectItem(room, "ID");
                            
                            if (strcmp(cJSON_GetStringValue(ID), room_ID) == 0){

                            printf ("yay");
                            //update hotel room details
                            }
                        }
        }
                    
                case 3:
                    printf("Enter the ID for the room\n");
                    scanf ("%s", &room_ID);
                    
                    if (rooms) {
                        int rooms_count = cJSON_GetArraySize(rooms);
                        for (int i = 0; i < rooms_count; i++) {
                            cJSON *room = cJSON_GetArrayItem(rooms, i);
                            cJSON *ID = cJSON_GetObjectItem(room, "ID");
                            
                            if (strcmp(cJSON_GetStringValue(ID), room_ID) == 0){

                            printf ("yay");
                            //remove hotel room details
                            }
                        }
        }
            }
    break;

     case 4:
        printf("Are you sure you want to log out? (yes/no)\n");
        scanf("%s", log_out_request);  

        if (strcmp(log_out_request, "yes") == 0) {
            printf("Logged out\n");
            break;
        } else if (strcmp(log_out_request, "no") == 0) {
            printf("Returning to main menu\n");
            // Add code to return to main menu
        } else {
            printf("Invalid input\n");
        }

    
        default:
            printf ("invalid choice");
        
        
    }        
}

cJSON* staffMenu(cJSON *json, cJSON *user, cJSON *users){
    int staffMenuChoice;
    
    printf("Sheba Staff Menu\n");

    while (1){
        printf("1.View Available Rooms\n2.Manage Bookings\n3.Booking History\n4.Change Password\n5.Logout\n");
        scanf("%d", &staffMenuChoice);
        
        switch (staffMenuChoice)
        {
        case 1:
            printf("Available Rooms");
            break;

        case 2:
            printf("Bookings");
            break;

        case 3:
            printf("History");
            break;

        case 4:
            editProfile(json, user, users);
            system("cls");
            printf("logged out!\n");
            return false; 

        case 5:
            printf("Logging out...\n");
            return false;
        
        default:
            system("cls");
            printf("Invalid option!\n");
        }
    }
}

cJSON* customerMenu(cJSON *json, cJSON *user){
    writeFile(json);
    cJSON* userIDItem = cJSON_GetObjectItemCaseSensitive(user, "userID");
    int userID = (int)cJSON_GetNumberValue(userIDItem);

    cJSON* rooms = cJSON_GetObjectItem(json, "rooms");
    cJSON* bookings = cJSON_GetObjectItem(json, "bookings");
    
    char roomID[20];
    char roomType[20];
    int choice;
    int roomsNumber = cJSON_GetArraySize(rooms);
    int bookingsNumber = cJSON_GetArraySize(bookings);
    
    do
    {
    
    printf("\nSheba Customer menu\n");
    printf("1 - Search for available rooms\n");
    printf("2 - Make a reservation\n");
    printf("3 - Cancel reservation\n");
    printf("4 - View reservation history\n");
    printf("5 - View current booking\n");
    printf("0 - Exit\n");
    printf("Please select a service: ");
    scanf("%d", &choice);
    

    switch (choice)
    {
    case 1: // Search the available rooms incase choice 1
    { 
        printf("Select preferred room type (Standard, Deluxe, or Suite): ");
        scanf("%s", roomType); 
        for (int i = 0; i < roomsNumber; i++) {
            cJSON* room = cJSON_GetArrayItem(rooms, i);
            cJSON* type = cJSON_GetObjectItem(room, "type");
            cJSON* status = cJSON_GetObjectItem(room, "status");

            if (strcmp(cJSON_GetStringValue(type), roomType) == 0 && strcmp(cJSON_GetStringValue(status), "Available") == 0)
            {
                cJSON* room_number = cJSON_GetObjectItem(room, "room_number");
                cJSON* room_floor = cJSON_GetObjectItem(room, "room_floor");
                cJSON* price = cJSON_GetObjectItem(room, "price");
                printf("|%15s|%7d|%15s|%10s|%7d|\n", cJSON_GetStringValue(cJSON_GetObjectItem(room, "ID")), 
                   (*room_number).valueint, cJSON_GetStringValue(room_floor),
                   roomType, (*price).valueint);
            }
        }
        break;
    }
        
    case 2: // Make A Reservation incase choice 2
    {
        char reservedOn[20];
        char checkInDate[20];
        char checkOutDate[20];
        // time with date
        time_t now = time(NULL);
        struct tm *timeinfo = localtime(&now);
        strftime(reservedOn, sizeof(reservedOn), "%Y-%m-%dT%H:%M:%S", timeinfo);

        printf("Please fill in the following:\n");
        printf("Your preferred RoomID: ");
        scanf("%s", roomID);

        // see room exists and availability
        bool roomFound; // boolean to check if the room is available, default 0(false)
        bool roomReserved; // boolean to cancel if the room is reserved

        if (rooms) {
            for (int i = 0; i < roomsNumber; i++)
            {
                cJSON* room = cJSON_GetArrayItem(rooms, i);
                if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(room, "ID")), roomID) == 0 &&
                    strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(room, "status")), "Available") == 0)
                    {
                    roomFound = true;
                    roomReserved = false;
                    }
                if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(room, "ID")), roomID) == 0 &&
                    strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(room, "status")), "Reserved") == 0)
                {
                    printf("Sorry, this room is not available.\n");
                    roomFound = true;
                    roomReserved = true;
                }
            }
        }

        if(!roomFound)
        {
            printf("Sorry, this room does not exist.\n");
            break;
        }

        if (roomReserved && roomFound)
        {
            break;
        }


        printf("When are you checking-in (YYYY-MM-DD): ");
        scanf("%s", checkInDate);
        
        printf("When are you checking-out (YYYY-MM-DD): ");
        scanf("%s", checkOutDate);

        generalCounterBooking(json);


        // Creating the booking object
        cJSON* newCustomerBooking = cJSON_CreateObject();
        cJSON_AddNumberToObject(newCustomerBooking, "bookingID", generalBookingIDCounter);
        cJSON_AddNumberToObject(newCustomerBooking, "customerID", userID);
        cJSON_AddStringToObject(newCustomerBooking, "roomID", roomID);
        cJSON_AddStringToObject(newCustomerBooking, "reservedOn", reservedOn); 
        cJSON_AddStringToObject(newCustomerBooking, "checkIn", checkInDate);
        cJSON_AddStringToObject(newCustomerBooking, "checkOut", checkOutDate);
        cJSON_AddStringToObject(newCustomerBooking, "status", "Confirmed");


        cJSON_AddItemToArray(bookings, newCustomerBooking); //Adding the new booking (the new customer's booking object) to data
        bookingsNumber++;

        // To change room Availability to Reserved
        for (int i = 0; i < roomsNumber; i++)
        {
            cJSON* room = cJSON_GetArrayItem(rooms, i);
            if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(room, "ID")), roomID) == 0)
            {
                cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("Reserved"));
                break;
            }
        }
        writeFile(json);

        printf("Your reservation has been confirmed!\nYour Booking ID is: %d\nWe are glad to serve you.\n", generalBookingIDCounter);
        break;
    }

    case 3: //Cancel reservation incase choice 3
    {
        int inputBookingID;
        cJSON* cancel = cJSON_CreateString("Canceled");

        viewCurrentBooking(bookings, bookingsNumber, userID);
        printf("To cancel, fill in the following:\n");

        printf("Your BookingID: ");
        scanf("%d", &inputBookingID);

        printf("Your RoomID: ");
        scanf("%s", roomID);


        bool bookingFound = false; // Flag to track if booking was found
        for (int i = 0; i < bookingsNumber; i++) {
            cJSON* booking = cJSON_GetArrayItem(bookings, i); 
            cJSON* BookingIDData = cJSON_GetObjectItemCaseSensitive(booking, "bookingID");

            if (cJSON_IsNumber(BookingIDData) && (int)cJSON_GetNumberValue(BookingIDData) == inputBookingID) {
                // Update room status to "Canceled"
                cJSON_ReplaceItemInObject(booking, "status", cancel);

                bookingFound = true;
                printf("\nYour Booking was canceled Successfully.\n");
                break;
            }
        }
        
    
        if (!bookingFound) {
        printf("Sorry, the provided booking ID cannot be found.\nPlease make sure to type in the correct ID.\n");
        } 

        // Switch room back to available
        for (int i = 0; i < roomsNumber; i++)
        {
            cJSON* room = cJSON_GetArrayItem(rooms, i);
            if (strcmp(cJSON_GetStringValue(cJSON_GetObjectItem(room, "ID")), roomID) == 0)
            {
                cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("Available"));
                break;
            }
        }
        
        writeFile(json);
        break;
    }
   
    case 4: // View reservation history incase choice 4
    {
        printf("Your Booking History:\n");
        printf("-------------------------------------------------------------\n");
        printf("|%10s|%15s|%10s|%10s|%10s|\n", "Booking ID", "Room ID", "Check-In", "Check-Out", "Status");
        printf("-------------------------------------------------------------\n");

        for (int i = 0; i < bookingsNumber; i++)
        {
            cJSON* booking = cJSON_GetArrayItem(bookings, i);
            if (cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(booking, "customerID")) == userID)
            {
                printf("|%10d|%15s|%10s|%10s|%10s|\n", 
                (int)cJSON_GetNumberValue(cJSON_GetObjectItem(booking, "bookingID")),
                cJSON_GetStringValue(cJSON_GetObjectItem(booking, "roomID")),
                cJSON_GetStringValue(cJSON_GetObjectItem(booking, "checkIn")),
                cJSON_GetStringValue(cJSON_GetObjectItem(booking, "checkOut")),
                cJSON_GetStringValue(cJSON_GetObjectItem(booking, "status")));
                
            }
        }
        printf("-------------------------------------------------------------\n");
            
        break;
    }

    case 5: // View current booking incase choice 5
    {
        viewCurrentBooking(bookings, bookingsNumber, userID);
        break;
    }
    }
    } while (choice >= 1 && choice <= 5);
}



void guestMenu(){
    printf("guest menu");
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
            if (passmask == 8 && i!=0){
                printf("\b \b"); 
                i--;
            }
            else if (passmask == 3) {
                exit(0);
            }
            else if (i==0 && passmask == 8){
                printf("");
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
                        adminMenu(json, users);
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "2") == 0){
                        isLoggedIn = true;
                        staffMenu(json, user, users);
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "3") == 0){
                        isLoggedIn = true;
                        customerMenu(json, user);
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
                printf("\nInvalid username or password! Try again (%d)\n", attempts);
                attempts--;
            }
        }
    }
    free(users);
}

int main() {

    //initialize variables
    int mainMenuChoice;

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

    printf("Hi! Welcome to Sheba Hotel App\n");
        
    while (1){    
        printf("1.log in\n2.guest\n3.Exit\n");
        scanf("%d", &mainMenuChoice);

        switch (mainMenuChoice)
        {
        case 1:
            logIn(json);
            break;
        
        case 2:
            guestMenu();
            break;

        case 3:
            printf("Exiting...\n");
            return false;
        
        default:
            system("cls");
            printf("Invalid option!\n");
        }
    }

    //clean memory up
    cJSON_Delete(json);
    free(content);

    return 0;
}
