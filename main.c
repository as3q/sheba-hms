#include <stdio.h>
#include <stdlib.h> //free malloc exit
#include <conio.h> //getch
#include <time.h>
#include <string.h> //strcmp strlen
#include <stdbool.h> //for booleans
#include "cjson/cJSON.h" //reading json
#include <ctype.h>

int generalBookingIDCounter;
cJSON *adminMenu(cJSON *json, cJSON *user, cJSON *users);
cJSON *registerNewUser(cJSON *users, cJSON *json);
cJSON *manageExistingUsers(cJSON *json, cJSON *users);
cJSON *manageHotelRooms(cJSON *rooms, cJSON *json);
cJSON *addHotelRoom(cJSON *rooms, cJSON *json);
cJSON *updateHotelRoom(cJSON *rooms, cJSON *json);
cJSON *removeHotelRoom(cJSON *rooms, cJSON *json);
cJSON *logOut(int *is_logged_in, cJSON *json);

//Mahmood Qaid
//Helper Functions + Staff
////////////////
cJSON* writeFile(cJSON *json){
    FILE *fp = fopen("data.txt", "w+"); 
    
    // write into file
   char *json_str = cJSON_Print(json);
   fputs(json_str, fp); 
   fclose(fp);
   cJSON_free(json_str); 
}

void clearInputBuffer(){
    int c;
        
    //clears input buffer
    while ((c = getchar()) != '\n' && c != EOF) {}
}

char* getCurrentTime() {
    time_t now;
    struct tm *local;
    char *currentTime = (char*)malloc(20);

    now = time(NULL);
    local = gmtime(&now);
    local->tm_hour += 8;  //adjust for UTC+8

    //handle UTC+8 hour rollover
    if (local->tm_hour >= 24) {
        local->tm_hour -= 24;
    }

    //format current time into ISO 8601 format
    strftime(currentTime, 20, "%Y-%m-%dT%H:%M:%S", local);
    return strcat(currentTime, "Z");
}

time_t parseTime(const char *timeStr) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    // Manually parse the date string
    sscanf(timeStr, "%d-%d-%dT%d:%d:%dZ",
           &tm.tm_year,
           &tm.tm_mon,
           &tm.tm_mday,
           &tm.tm_hour,
           &tm.tm_min,
           &tm.tm_sec);

    // Adjust year and month values
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    return mktime(&tm);
}

void printUsersByStatus(cJSON *users, const char *status) {
    int found = 0;
    int userCount = cJSON_GetArraySize(users);
    for (int i = 0; i < userCount; i++) {
        cJSON *user = cJSON_GetArrayItem(users, i);
        cJSON *userID = cJSON_GetObjectItem(user, "userID");
        cJSON *userStatus = cJSON_GetObjectItem(user, "status");
        
        if (userStatus && strcmp(userStatus->valuestring, status) == 0) {
            printf("\nUser ID: %d\n", userID->valueint);
            found = 1;
        }
    }
    if (found == 0){
        printf("No users with status %s found!\n", status);
    }
}

cJSON* editProfile(cJSON *json, cJSON *user, cJSON *users){
    printf("Editing profile...\n");

    char newPassword[16];

    cJSON *old_password = cJSON_GetObjectItem(user, "password");

    while (1) {    
        printf("Enter your new password: ");
        scanf("%s", &newPassword); //scanf

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

////////////////

cJSON *staffAutoCancel(cJSON *json){
     int available = 0;
    cJSON *rooms = cJSON_GetObjectItem(json, "rooms");
    cJSON *bookings = cJSON_GetObjectItem(json, "bookings");
    cJSON *users = cJSON_GetObjectItem(json, "users");

    if (!rooms || !bookings || !users) {
        printf("No room, booking, or user data found!\n");
        return NULL;
    }

    int bookings_count = cJSON_GetArraySize(bookings);
    int users_count = cJSON_GetArraySize(users);
    int rooms_count = cJSON_GetArraySize(rooms);

    time_t now = time(NULL);

    //check all bookings for cancellations
    for (int j = 0; j < bookings_count; j++) {
        cJSON *booking = cJSON_GetArrayItem(bookings, j);
        cJSON *bookingID = cJSON_GetObjectItem(booking, "bookingID");
        cJSON *customerID = cJSON_GetObjectItem(booking, "customerID");
        cJSON *roomIDInBooking = cJSON_GetObjectItem(booking, "roomID");
        cJSON *reservedOn = cJSON_GetObjectItem(booking, "reservedOn");
        cJSON *bookingStatus = cJSON_GetObjectItem(booking, "status");

        if (!bookingID || !customerID || !roomIDInBooking || !reservedOn || !bookingStatus) {
            printf("Error: Missing booking details\n");
            continue;
        }

        time_t reservedTime = parseTime(reservedOn->valuestring);
        int daysElapsed = difftime(now, reservedTime) / (60 * 60 * 24);

        if (daysElapsed > 3 && strcmp(bookingStatus->valuestring, "active") == 0) {
            cJSON_SetValuestring(bookingStatus, "Canceled");
            available = 1;

            //update user status
            for (int k = 0; k < users_count; k++) {
                cJSON *user = cJSON_GetArrayItem(users, k);
                cJSON *userID = cJSON_GetObjectItem(user, "userID");
                cJSON *userStatus = cJSON_GetObjectItem(user, "status");

                if (userID->valueint == customerID->valueint) {
                    cJSON_SetValuestring(userStatus, "N/A");
                    break;
                }
            }

            //update room status
            for (int l = 0; l < rooms_count; l++) {
                cJSON *room = cJSON_GetArrayItem(rooms, l);
                cJSON *roomID = cJSON_GetObjectItem(room, "ID");
                cJSON *roomStatus = cJSON_GetObjectItem(room, "status");

                if (strcmp(roomID->valuestring, roomIDInBooking->valuestring) == 0) {
                    cJSON_SetValuestring(roomStatus, "Available");
                    break;
                }
            }

            printf("Reservation ID %d has been cancelled due to exceeding 3 days.\n\n", bookingID->valueint);
        }
    }

    if (available == 0) {
        printf("No reservations exceeding 3 days were found!\n\n");
    } else {
        writeFile(json);
    }
}

cJSON* staffMakeReservation(cJSON *json, cJSON *users){
    char staffViewBack[16];
    char enteredUsername[16];
    int customerID = -1;

    int users_count = cJSON_GetArraySize(users);

    cJSON* bookings = cJSON_GetObjectItem(json, "bookings");
    int bookings_count = cJSON_GetArraySize(bookings);

    cJSON* rooms = cJSON_GetObjectItem(json, "rooms");
    if (rooms) {
        int rooms_count = cJSON_GetArraySize(rooms);
        for (int i = 0; i < rooms_count; i++) {
            cJSON *room = cJSON_GetArrayItem(rooms, i);

            cJSON *roomID = cJSON_GetObjectItem(room, "ID");
            cJSON *room_floor = cJSON_GetObjectItem(room, "room_floor");
            cJSON *room_type = cJSON_GetObjectItem(room, "type");
            cJSON *room_number = cJSON_GetObjectItem(room, "room_number");
            cJSON *room_price = cJSON_GetObjectItem(room, "price");
            cJSON *room_status = cJSON_GetObjectItem(room, "status");

            if (strcmp(cJSON_GetStringValue(room_status), "Available") == 0){
                printf("RoomID: %s\n", roomID->valuestring);
                printf("Room: %d - %s\n", room_number->valueint, room_floor->valuestring);
                printf("Type: %s\n", room_type->valuestring);
                printf("Price: %d\n\n", room_price->valueint);
            }
        }

        while (1){
            printf("Enter room ID (0 to exit): ");
            
            clearInputBuffer();
            scanf("%s", staffViewBack);

            if (strcmp(staffViewBack, "0") == 0){
                system("cls");
                return NULL;
            }

            int roomIndex = -1;
            for (int i = 0; i < rooms_count; i++) {
                cJSON *room = cJSON_GetArrayItem(rooms, i);
                cJSON *roomID = cJSON_GetObjectItem(room, "ID");

                if (strcmp(roomID->valuestring, staffViewBack) == 0) {
                    roomIndex = i;
                    break;
                }
            }

            if (roomIndex == -1) {
                printf("Invalid room ID!\n");
                continue;
            }

            clearInputBuffer();
            printf("Enter customer username: ");
            scanf("%s", enteredUsername); //scanf

            for (int i = 0; i < users_count; i++) {
                cJSON *user = cJSON_GetArrayItem(users, i);
                cJSON *userID = cJSON_GetObjectItem(user, "userID");
                cJSON *username = cJSON_GetObjectItem(user, "username");
                cJSON *userRole = cJSON_GetObjectItem(user, "role");
                cJSON *userStatus = cJSON_GetObjectItem(user, "status");

                if (strcmp(cJSON_GetStringValue(userStatus), "N/A") == 0 && strcmp(cJSON_GetStringValue(username), enteredUsername) == 0 && strcmp(cJSON_GetStringValue(userRole), "3") == 0){
                    customerID = userID->valueint;
                    break;
                }
            }

            if (customerID == -1) {
                printf("Invalid customer username or customer already has a reservation!\n");
                continue;
            }

            if (customerID == -1) {
                continue;
            }

            cJSON *room = cJSON_GetArrayItem(rooms, roomIndex);
            cJSON *roomID = cJSON_GetObjectItem(room, "ID");
            cJSON *roomStatus = cJSON_GetObjectItem(room, "status");

            if (strcmp(cJSON_GetStringValue(roomStatus), "Available") == 0){
                int maxID = 0;
                for (int i = 0; i < bookings_count; i++) {
                    cJSON *booking = cJSON_GetArrayItem(bookings, i);
                    cJSON *bookingID = cJSON_GetObjectItemCaseSensitive(booking, "bookingID");
                    if (cJSON_IsNumber(bookingID) && (bookingID->valueint > maxID)) {
                        maxID = bookingID->valueint;
                    }
                }
                int newID = maxID + 1;

                char* reservedOn = getCurrentTime();

                cJSON *newBooking = cJSON_CreateObject();
                cJSON_AddNumberToObject(newBooking, "bookingID", newID);
                cJSON_AddNumberToObject(newBooking, "customerID", customerID);
                cJSON_AddStringToObject(newBooking, "roomID", roomID->valuestring);
                cJSON_AddStringToObject(newBooking, "reservedOn", reservedOn);
                cJSON_AddStringToObject(newBooking, "checkedIn", "N/A");
                cJSON_AddStringToObject(newBooking, "checkedOut", "N/A");
                cJSON_AddStringToObject(newBooking, "status", "Confirmed");

                cJSON_AddItemToArray(bookings, newBooking);

                cJSON* new_status = cJSON_CreateString("Reserved");
                cJSON_ReplaceItemInObject(room, "status", new_status);

                for (int i = 0; i < users_count; i++) {
                    cJSON *user = cJSON_GetArrayItem(users, i);
                    cJSON *userID = cJSON_GetObjectItem(user, "userID");

                    if (userID->valueint == customerID) {
                        cJSON_ReplaceItemInObject(user, "status", cJSON_CreateString("reservee"));
                        break;
                    }
                }

                writeFile(json);
                printf("Reservation Successfully made!\n");

                free(reservedOn);
                free(bookings);
                free(rooms);

            } else {
                printf("Room not Available!\n");
            }
        }
    } else {
        printf("No room data found!\n");
    }
}

cJSON* staffCancelReservation(cJSON *json, cJSON *users){
    int available = 0;
    cJSON *rooms = cJSON_GetObjectItem(json, "rooms");
    cJSON *bookings = cJSON_GetObjectItem(json, "bookings");

    if (!rooms || !bookings || !users) {
        printf("No room, booking or user data found!\n");
        return NULL;
    }

    int bookings_count = cJSON_GetArraySize(bookings);
    int users_count = cJSON_GetArraySize(users);
    int rooms_count = cJSON_GetArraySize(rooms);

    //display all reserved rooms
    for (int i = 0; i < rooms_count; i++) {
        cJSON *room = cJSON_GetArrayItem(rooms, i);
        cJSON *roomID = cJSON_GetObjectItem(room, "ID");
        cJSON *roomStatus = cJSON_GetObjectItem(room, "status");

        if (strcmp(roomStatus->valuestring, "Reserved") == 0) {

            for (int j = 0; j < bookings_count; j++) {
                cJSON *booking = cJSON_GetArrayItem(bookings, j);
                cJSON *bookingID = cJSON_GetObjectItem(booking, "bookingID");
                cJSON *customerID = cJSON_GetObjectItem(booking, "customerID");
                cJSON *roomIDInBooking = cJSON_GetObjectItem(booking, "roomID");
                cJSON *bookingStatus = cJSON_GetObjectItem(booking, "status");
                cJSON *checkedIn = cJSON_GetObjectItem(booking, "checkedIn");

                if (strcmp(roomID->valuestring, roomIDInBooking->valuestring) == 0 && strcmp(checkedIn->valuestring, "N/A") == 0) {
                    for (int k = 0; k < users_count; k++) {
                        cJSON *user = cJSON_GetArrayItem(users, k);
                        cJSON *userID = cJSON_GetObjectItem(user, "userID");
                        cJSON *userStatus = cJSON_GetObjectItem(user, "status");
                        cJSON *username = cJSON_GetObjectItem(user, "username");

                        if (userID->valueint == customerID->valueint) {
                            printf("Reservation ID: %d | Room ID: %s | Customer: %s\n", bookingID->valueint, roomID->valuestring, username->valuestring);   
                            available = 1;
                        }
                    }
                }
            }
        }
    }

    if (available == 0){
        printf("No current reservations found!\n");
    }

    //prompt for cancellation
    while (available == 1) {
        char input[16];
        printf("Enter reservation ID (0 to exit): ");
        
        clearInputBuffer();
        fgets(input, sizeof(input), stdin);

        int reservationID = atoi(input);
        if (reservationID > 0) {
            int found = 0;
            for (int j = 0; j < bookings_count; j++) {
                cJSON *booking = cJSON_GetArrayItem(bookings, j);
                cJSON *bookingID = cJSON_GetObjectItem(booking, "bookingID");
                cJSON *customerID = cJSON_GetObjectItem(booking, "customerID");
                cJSON *roomIDInBooking = cJSON_GetObjectItem(booking, "roomID");
                cJSON *bookingStatus = cJSON_GetObjectItem(booking, "status");

                if (bookingID->valueint == reservationID) {
                    cJSON_SetValuestring(bookingStatus, "Canceled");
                    found = 1;

                    for (int k = 0; k < users_count; k++) {
                        cJSON *user = cJSON_GetArrayItem(users, k);
                        cJSON *userID = cJSON_GetObjectItem(user, "userID");
                        cJSON *userStatus = cJSON_GetObjectItem(user, "status");

                        if (userID->valueint == customerID->valueint) {
                            cJSON_SetValuestring(userStatus, "N/A");

                            // Update room status
                            for (int l = 0; l < rooms_count; l++) {
                                cJSON *room = cJSON_GetArrayItem(rooms, l);
                                cJSON *roomID = cJSON_GetObjectItem(room, "ID");
                                cJSON *roomStatus = cJSON_GetObjectItem(room, "status");

                                if (strcmp(roomID->valuestring, roomIDInBooking->valuestring) == 0) {
                                    cJSON_SetValuestring(roomStatus, "Available");
                                    break;
                                }
                            }

                            //change reservation ststus to Canceled

                            writeFile(json);
                            printf("Reservation ID %d has been cancelled.\n", reservationID);
                            return NULL;
                        }
                    }
                }
            }

            if (!found) {
                printf("Reservation ID not found.\n");
            }
        } else {
            printf("Invalid input!\n");
        }
    }
}

cJSON*  staffReservationMenu(cJSON *json,cJSON *users){
    
    while (1){
        char staffReservationChoice[2];

        printf("1.Make Reservation\n2.Cancel Reservation\n3.Go back\n");

        clearInputBuffer();
        fgets(staffReservationChoice, sizeof(staffReservationChoice), stdin);
        
        system("cls");
        switch (atoi(staffReservationChoice))
        {
        case 1:
            staffMakeReservation(json, users);
            break;

        case 2:
            staffCancelReservation(json, users);
            break;

        case 3:
            return false;
            break;

        default:
            printf("Invalid option!\n");
            break;
        }
    }
}

cJSON*  staffCheckInOut(cJSON *json, cJSON *users){
    int choice;

    cJSON *rooms = cJSON_GetObjectItem(json, "rooms");
    int rooms_count = cJSON_GetArraySize(rooms);

    cJSON *bookings = cJSON_GetObjectItem(json, "bookings");
    int bookingCount = cJSON_GetArraySize(bookings);

    cJSON *invoices = cJSON_GetObjectItem(json, "invoices");
    int invoices_count = cJSON_GetArraySize(invoices);

    printf("1. Check-in\n");
    printf("2. Check-out\n");
    printf("3. Back\n");
    printf("Enter your choice: ");
    scanf("%d", &choice); //scanf

    if (choice == 1) { // Check-in Reservees
        printUsersByStatus(users, "reservee");

        int userIDInput;
        printf("Enter user ID to check-in (0 to exit): ");
        scanf("%d", &userIDInput); //scanf

        //check if user ID is a reservee
        cJSON *userToCheckIn = NULL;
        int userCount = cJSON_GetArraySize(users);
        for (int i = 0; i < userCount; i++) {
            cJSON *user = cJSON_GetArrayItem(users, i);
            cJSON *userID = cJSON_GetObjectItem(user, "userID");
            cJSON *userStatus = cJSON_GetObjectItem(user, "status");

            if (userID->valueint == userIDInput && userStatus && strcmp(userStatus->valuestring, "reservee") == 0) {
                userToCheckIn = user;
                break;
            }
        }

        if (userToCheckIn != 0) {
            cJSON* currentTime = cJSON_CreateString(getCurrentTime());

            //find the user's booking and update it
            for (int i = 0; i < bookingCount; i++) {
                cJSON *booking = cJSON_GetArrayItem(bookings, i);
                cJSON *bookingID = cJSON_GetObjectItem(booking, "bookingID");
                cJSON *customerID = cJSON_GetObjectItem(booking, "customerID");
                cJSON *roomIDInBooking = cJSON_GetObjectItem(booking, "roomID");
                cJSON *checkedIn = cJSON_GetObjectItem(booking, "checkedIn");
                cJSON *bookingStatus = cJSON_GetObjectItem(booking, "status");


                if (customerID->valueint == userIDInput && strcmp(checkedIn->valuestring, "N/A") == 0 && strcmp(bookingStatus->valuestring, "Confirmed") == 0) {
                    //add check in time to booking
                    cJSON_ReplaceItemInObject(booking, "checkedIn", currentTime);
                    printf("Updated booking ID: %d.\n", bookingID->valueint);

                    //room status to Occupied
                    for (int l = 0; l < rooms_count; l++) {
                        cJSON *room = cJSON_GetArrayItem(rooms, l);
                        cJSON *roomID = cJSON_GetObjectItem(room, "ID");
                        cJSON *roomStatus = cJSON_GetObjectItem(room, "status");

                        if (strcmp(roomID->valuestring, roomIDInBooking->valuestring) == 0) {
                            cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("Occupied"));
                        }
                    }

                    //user status to checked in
                    cJSON_ReplaceItemInObject(userToCheckIn, "status", cJSON_CreateString("checked in"));
                    writeFile(json);
                    printf("User Successfully Checked In!\n");
                }
            }

        } else {
            printf("User not found or not a reservee.\n");
        }

    } else if (choice == 2) { // Check-out Checked-in Users
        printUsersByStatus(users, "checked in");

        int userIDInput;
        printf("Enter user ID to check-out (0 to exit): ");
        scanf("%d", &userIDInput); //scanf

        // Check if user ID is checked in
        cJSON *userToCheckOut = NULL;
        int userCount = cJSON_GetArraySize(users);
        for (int i = 0; i < userCount; i++) {
            cJSON *user = cJSON_GetArrayItem(users, i);
            cJSON *userID = cJSON_GetObjectItem(user, "userID");
            cJSON *userStatus = cJSON_GetObjectItem(user, "status");

            if (userID->valueint == userIDInput && userStatus && strcmp(userStatus->valuestring, "checked in") == 0) {
                userToCheckOut = user;
                break;
            }
        }

        if (userToCheckOut != 0) {
            // Update booking and user status
            char *currentTime = getCurrentTime();
            double total;

            // Find the user's booking and update it
            for (int i = 0; i < bookingCount; i++) {
                cJSON *booking = cJSON_GetArrayItem(bookings, i);
                cJSON *customerID = cJSON_GetObjectItem(booking, "customerID");
                cJSON *roomIDInBooking = cJSON_GetObjectItem(booking, "roomID");
                cJSON *checkedIn = cJSON_GetObjectItem(booking, "checkedIn");
                cJSON *checkedOut = cJSON_GetObjectItem(booking, "checkedOut");

                //from invoices count increment ID
                if (customerID->valueint == userIDInput && strcmp(checkedOut->valuestring, "N/A") == 0) {
                    
                    cJSON_ReplaceItemInObject(booking, "checkedOut", cJSON_CreateString(currentTime));
                    printf("Updated booking ID: %d with checked-out time.\n", cJSON_GetObjectItem(booking, "bookingID")->valueint);
                    
                    //new ID
                    int maxID = 0;
                    for (int i = 0; i < invoices_count; i++) {
                        cJSON *invoice = cJSON_GetArrayItem(invoices, i);
                        cJSON *invoiceID = cJSON_GetObjectItemCaseSensitive(invoice, "invoiceID");
                        if (cJSON_IsNumber(invoiceID) && (invoiceID->valueint > maxID)) {
                            maxID = invoiceID->valueint;
                        }
                    }
                    int newID = maxID + 1;

                    //find room price and update room status
                    for (int l = 0; l < rooms_count; l++) {
                        cJSON *room = cJSON_GetArrayItem(rooms, l);
                        cJSON *roomID = cJSON_GetObjectItem(room, "ID");
                        cJSON *roomPrice = cJSON_GetObjectItem(room, "price");

                        if (strcmp(roomID->valuestring, roomIDInBooking->valuestring) == 0) {
                            time_t now = time(NULL);
                            time_t checkInTime = parseTime(checkedIn->valuestring);
                            int daysSpent = difftime(now, checkInTime) / (60 * 60 * 24);
                            total = daysSpent * roomPrice->valueint;
                            if (total == 0){
                                total = roomPrice->valueint;
                            }

                            cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("Available"));
                        }

                    }

                    //create an invoice
                    cJSON *newInvoice = cJSON_CreateObject();
                    cJSON_AddNumberToObject(newInvoice, "invoiceID", newID); 
                    cJSON_AddNumberToObject(newInvoice, "customerID", userIDInput);
                    cJSON_AddNumberToObject(newInvoice, "bookingID", cJSON_GetObjectItem(booking, "bookingID")->valueint);
                    cJSON_AddNumberToObject(newInvoice, "amountPaid", total);
                    cJSON_AddStringToObject(newInvoice, "datePaid", currentTime);
                    cJSON_AddItemToArray(invoices, newInvoice);
                    printf("Created invoice for booking ID: %d.\n", cJSON_GetObjectItem(booking, "bookingID")->valueint);
                    break;
                }
            }

            cJSON_ReplaceItemInObject(userToCheckOut, "status", cJSON_CreateString("N/A"));
            writeFile(json);
            printf("User Successfully Checked out!\n");

        } else if (userToCheckOut == 0) {
            printf("");
        } 
        else {
            printf("User not found or not checked in.\n");
        }

    } else if (choice == 3) { // Exit
        printf("");
    } else {
        printf("Invalid choice. Please try again.\n");
    }

    free(bookings);
    free(invoices);
    free(rooms);
}

cJSON*  staffBookingsMenu(cJSON *json, cJSON *users){
    while (1){
        char staffBookingChoice[2];

        printf("1.Reservations\n2.CheckIn/CheckOut\n3.Go back\n");

        clearInputBuffer();
        fgets(staffBookingChoice, sizeof(staffBookingChoice), stdin);
        
        system("cls");
        switch (atoi(staffBookingChoice))
        {
        case 1:
            staffReservationMenu(json, users);
            break;

        case 2:
            staffCheckInOut(json, users);
            system("cls");
            break;

        case 3:
            return false;
            break;

        default:
            printf("Invalid option!\n");
            break;
        }
    }
}

cJSON*  staffViewRooms(cJSON *json){
    int count = 1;
    char staffViewBack[2];

    cJSON* rooms = cJSON_GetObjectItem(json, "rooms");
    if (rooms) {
        int rooms_count = cJSON_GetArraySize(rooms);
        for (int i = 0; i < rooms_count; i++) {
            cJSON *room = cJSON_GetArrayItem(rooms, i);

            cJSON *roomID = cJSON_GetObjectItem(room, "ID");
            cJSON *room_floor = cJSON_GetObjectItem(room, "room_floor");
            cJSON *room_type = cJSON_GetObjectItem(room, "type");
            cJSON *room_number = cJSON_GetObjectItem(room, "room_number");
            cJSON *room_price = cJSON_GetObjectItem(room, "price");
            cJSON *room_status = cJSON_GetObjectItem(room, "status");

            if (strcmp(cJSON_GetStringValue(room_status), "Available") == 0){
                printf("%d.\n", count);
                printf("Room ID: %s\n", roomID->valuestring);
                printf("Room: %d - %s\n", room_number->valueint, room_floor->valuestring);
                printf("Type: %s\n", room_type->valuestring);
                printf("Price: %d\n\n", room_price->valueint);
                count++;
            }

            }
        while (1){
            printf("Enter 0 to exit: ");
            
            
            clearInputBuffer();
            fgets(staffViewBack, sizeof(staffViewBack), stdin);

            if (strcmp(staffViewBack, "0") == 0){
                system("cls");
                return NULL;
            }
        }
    } else {
        printf("No room data found!");
    }
    free(rooms);
    return NULL;
}

cJSON*  staffHistoryView(cJSON *json, cJSON *users){
    char *customerStatus;
    char *customerName;
    char staffHistoryBack[2];

    cJSON* bookings = cJSON_GetObjectItem(json, "bookings");
    int bookingsCount = cJSON_GetArraySize(bookings);

    cJSON *invoices = cJSON_GetObjectItem(json, "invoices");
    int invoicesCount = cJSON_GetArraySize(invoices);
    
    int usersCount = cJSON_GetArraySize(users);

    printf("ID |    CUSTOMER    |       RESERVATION       |       CHECK IN         |        CHECK OUT       |            PAYMENT             | STATUS\n");
    printf("-----------------------------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < bookingsCount; i++) {
        cJSON *booking = cJSON_GetArrayItem(bookings, i);
        cJSON *bookingID = cJSON_GetObjectItem(booking, "bookingID");
        cJSON *customerID = cJSON_GetObjectItem(booking, "customerID");
        cJSON *roomIDInBooking = cJSON_GetObjectItem(booking, "roomID");
        cJSON *reservedOn = cJSON_GetObjectItem(booking, "reservedOn");
        cJSON *checkedIn = cJSON_GetObjectItem(booking, "checkedIn");
        cJSON *checkedOut = cJSON_GetObjectItem(booking, "checkedOut");
        cJSON *bookingStatus = cJSON_GetObjectItem(booking, "status");

        for (int j = 0; j < usersCount; j++) {
            cJSON *user = cJSON_GetArrayItem(users, j);
            cJSON *userID = cJSON_GetObjectItem(user, "userID");
            cJSON *username = cJSON_GetObjectItem(user, "username");
            cJSON *userStatus = cJSON_GetObjectItem(user, "status");

            if (customerID->valueint == userID->valueint){
                customerStatus = userStatus->valuestring;
                strcpy(customerName, username->valuestring);
            }
        
        }

        if (strcmp(cJSON_GetStringValue(checkedOut), "N/A") != 0) {
            for (int i = 0; i < invoicesCount; i++) {
                cJSON *invoice = cJSON_GetArrayItem(invoices, i);
                cJSON *invoiceID = cJSON_GetObjectItem(invoice, "invoiceID");
                cJSON *bookingIDInInvoice = cJSON_GetObjectItem(invoice, "bookingID");
                cJSON *paidAmount = cJSON_GetObjectItem(invoice, "amountPaid");
                cJSON *invoiceDate = cJSON_GetObjectItem(invoice, "datePaid");

                if (bookingID->valueint == bookingIDInInvoice->valueint) {
                    printf(" ");
                    printf("%d", bookingID->valueint);
                    printf(" |   ");
                    printf(" %s ", customerName);
                    printf("  | ");
                    printf(" %s ", reservedOn->valuestring);
                    printf("  | ");
                    printf(" %s ", checkedIn->valuestring);
                    printf(" | ");
                    printf(" %s ", checkedOut->valuestring);
                    printf(" |   ");
                    printf("%d on ", paidAmount->valueint);
                    printf("%s ", invoiceDate->valuestring);
                    printf(" | ");
                    printf("COMPLETE\n");
                }
                }
        }

        if (strcmp(cJSON_GetStringValue(reservedOn), "N/A") != 0 && strcmp(cJSON_GetStringValue(checkedIn), "N/A") == 0 && strcmp(customerStatus, "reservee") == 0 && strcmp(bookingStatus->valuestring, "confirmede") == 0) {
            printf(" ");
            printf("%d", bookingID->valueint);
            printf(" |   ");
            printf(" %s ", customerName);
            printf("  | ");
            printf(" %s  ", reservedOn->valuestring);
            printf(" | ");
            printf("          N/A         ");
            printf(" | ");
            printf("          N/A         ");
            printf(" |  ");
            printf("           N/A         ");
            printf("       | ");
            printf("RESERVATION\n");
        }

        if (strcmp(cJSON_GetStringValue(checkedIn), "N/A") != 0 && strcmp(cJSON_GetStringValue(checkedOut), "N/A") == 0) {
            printf(" ");
            printf("%d", bookingID->valueint);
            printf(" |   ");
            printf(" %s ", customerName);
            printf("  | ");
            printf(" %s ", reservedOn->valuestring);
            printf("  | ");
            printf(" %s ", checkedIn->valuestring);
            printf(" | ");
            printf("          N/A         ");
            printf(" |  ");
            printf("          N/A          ");
            printf("       | ");
            printf("CHECKED IN\n");
        }


        if (strcmp(bookingStatus->valuestring, "Canceled") == 0) {
            printf(" ");
            printf("%d", bookingID->valueint);
            printf(" |   ");
            printf(" %s ", customerName);
            printf("  | ");
            printf(" %s ", reservedOn->valuestring);
            printf("  | ");
            printf("          N/A         ");
            printf(" | ");
            printf("          N/A         ");
            printf(" |  ");
            printf("          N/A         ");
            printf("        | ");
            printf("CANELED RESERVATION\n");
        }
    }

    while (1){
        printf("\nEnter 0 to exit: ");
        
        
        clearInputBuffer();
        fgets(staffHistoryBack, sizeof(staffHistoryBack), stdin);

        if (strcmp(staffHistoryBack, "0") == 0){
            system("cls");
            return NULL;
        }
    }

    free(bookings);
    free(invoices);
}

cJSON* staffMenu(cJSON *json, cJSON *user, cJSON *users){
    char staffMenuChoice[2];

    while (1){

        staffAutoCancel(json);

        printf("[Sheba Staff Menu]\n");
        printf("1.View Available Rooms\n2.Manage Bookings\n3.Booking History\n4.Change Password\n5.Logout\n");
        

        clearInputBuffer();
        fgets(staffMenuChoice, sizeof(staffMenuChoice), stdin);

        system("cls");
        switch (atoi(staffMenuChoice))
        {
        case 1:
            staffViewRooms(json);
            break;

        case 2:
            staffBookingsMenu(json, users);
            break;

        case 3:
            staffHistoryView(json, users);
            break;

        case 4:
            editProfile(json, user, users);
            system("cls");
            printf("logged out!\n");
            return false; 

        case 5:
            return false;
        
        default:
            printf("Invalid option!\n");
        };
    }
}
////////////////


//Ibrahim Saeed
//Customer
////////////////
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

cJSON* customerMenu(cJSON *json, cJSON *user, cJSON *users){
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
    printf("6 - Change password\n");
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
    case 6:
    {
        editProfile(json, user, users);
        break;
    }
    }
    } while (choice >= 1 && choice <= 5);
}
////////////////


//Sohaib Aboosbua
//Admin
////////////////
// Implementing adminMenu function
cJSON *adminMenu(cJSON *json, cJSON *user, cJSON *users) {
    char room_ID[50];
    int choice, is_logged_in = 1;
    cJSON *rooms = cJSON_GetObjectItem(json, "rooms");
    
    //admin main menu
    while (is_logged_in) {
        printf("admin menu\n");
        printf("1 - Register new users\n");
        printf("2 - Manage existing users\n");
        printf("3 - Manage hotel rooms\n");
        printf("4 - Change password\n");
        printf("5 - Log out\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                registerNewUser(users, json);
                break;
            case 2:
                manageExistingUsers(json, users);
                break;
            case 3:
                manageHotelRooms(rooms, json);
                break;
            case 4:
                editProfile(json, user, users);
                break;
            case 5:
                logOut(&is_logged_in, json);
                break;
            default:
                printf("Invalid choice\n");
                break;
        }
    }
    return NULL;
}

// Generate ID code for new user
int getNextUserID(cJSON *users) {
    int maxID = 0;
    if (users) {
        int users_count = cJSON_GetArraySize(users);
        for (int i = 0; i < users_count; i++) {
            cJSON *user = cJSON_GetArrayItem(users, i);
            cJSON *userID = cJSON_GetObjectItem(user, "userID");
            if (userID && cJSON_IsNumber(userID)) {
                int currentID = userID->valueint;
                if (currentID > maxID) {
                    maxID = currentID;
                }
            }
        }
    }
    return maxID + 1;
}

//is username taken or no for new user
int isUsernameTaken(cJSON *users,  char *username) {
    if (users) {
        int users_count = cJSON_GetArraySize(users);
        for (int i = 0; i < users_count; i++) {
            cJSON *user = cJSON_GetArrayItem(users, i);
            cJSON *existingUsername = cJSON_GetObjectItem(user, "username");
            if (existingUsername && strcmp(cJSON_GetStringValue(existingUsername), username) == 0) {
                return 1; // Username is taken
            }
        }
    }
    return 0; // Username is not taken
}

//registering new user
cJSON *registerNewUser(cJSON *users, cJSON *json) {
    
    char fullname_new[100], username_new[100], password_new[100], role_new[100];
    int userID_new;
    printf("You selected 'Register new users'\n");

    printf("Enter the full name of the user: ");
    // Clear input buffer (chatgpt)
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);


    fgets(fullname_new, sizeof(fullname_new), stdin); // stdin means the input
    fullname_new[strcspn(fullname_new, "\n")] = '\0';  // Remove newline character

    while (1) {
        printf("Enter a username for the user: ");
        fgets(username_new, sizeof(username_new), stdin);
        username_new[strcspn(username_new, "\n")] = '\0';  // Remove newline character

        if (isUsernameTaken(users, username_new)) {
            printf("Username '%s' is already taken. Please choose a different username.\n", username_new);
            continue;
        } else {
            break; // Username is new
        }
    }

    printf("Enter a password for the user: ");
    fgets(password_new, sizeof(password_new), stdin);
    password_new[strcspn(password_new, "\n")] = '\0';  // Remove newline character

    printf("Enter the role of the user: ");
    fgets(role_new, sizeof(role_new), stdin);
    role_new[strcspn(role_new, "\n")] = '\0';  // Remove newline character

    int nextID = getNextUserID(users);// generate ID
    
    

    cJSON *new_user = cJSON_CreateObject();// adding the info to the file
    cJSON_AddNumberToObject(new_user, "userID", nextID);
    cJSON_AddStringToObject(new_user, "fullname", fullname_new);
    cJSON_AddStringToObject(new_user, "username", username_new);
    cJSON_AddStringToObject(new_user, "password", password_new);
    cJSON_AddStringToObject(new_user, "role", role_new);
    cJSON_AddItemToObject(new_user, "status", cJSON_CreateNull());

    printf("New userID: %s\n", userID_new);

    cJSON_AddItemToArray(users, new_user);
    writeFile(json);

    
    
    return new_user;
}
// managing the users 
cJSON *manageExistingUsers(cJSON *json, cJSON *users) {
    char userStatuschange, input[10];
    int userID_existing;
    printf("You selected 'Manage existing users'\n");

    while (1) {
        
        printf("Enter the userID of the user or press R to go back to main menu: ");
        
        scanf("%s", input);

        
        if (input[0] == 'R' || input[0] == 'r') {
            printf("Returning to main menu\n");
            return NULL;
        }

        // Convert input to integer 
        if (sscanf(input, "%d", &userID_existing) != 1) {
            printf("Invalid input. Please enter a valid userID.\n");
            // Clear input buffer to handle invalid input
            while (getchar() != '\n');
            continue;
        }

        // Check if the user exists
        int user_found = 0;
        int user_count = cJSON_GetArraySize(users);
        for (int i = 0; i < user_count; i++) {
            cJSON *user = cJSON_GetArrayItem(users, i);
            cJSON *userID = cJSON_GetObjectItem(user, "userID");

            if (cJSON_IsNumber(userID) && userID->valueint == userID_existing) {
                printf("This user exists\n");
                user_found = 1;

                // Change user status
                while (1) {
                    printf("Change user status to: enter a number (1 - null, 2 - reservee, 3 - checked-in) or press R to go back to main menu: ");
                    if (scanf(" %c", &userStatuschange) != 1) {
                        printf("Invalid input. Please enter a valid option.\n");
                        
                        while (getchar() != '\n');
                        continue;
                    }

                    if (userStatuschange == 'R' || userStatuschange == 'r') {
                        printf("Returning to main menu\n");
                        return NULL;
                    }
                    //status choosing
                    switch (userStatuschange) {
                        case '1':
                            cJSON_ReplaceItemInObject(user, "status", cJSON_CreateNull());
                            writeFile(json);
                            printf("User status changed to null\n");
                            break;
                        case '2':
                            cJSON_ReplaceItemInObject(user, "status", cJSON_CreateString("reservee"));
                            writeFile(json);
                            printf("User status changed to reservee\n");
                            break;
                        case '3':
                            cJSON_ReplaceItemInObject(user, "status", cJSON_CreateString("checked-in"));
                            writeFile(json);
                            printf("User status changed to checked-in\n");
                            break;
                        default:
                            printf("Invalid choice. Please select a valid status.\n");
                            continue;
                    }
                    break;  
                }
                break;  
            }
        }

        if (!user_found) {
            printf("User doesn't exist\n");
        }

        // Clear input buffer to avoid issues with scanf
        while (getchar() != '\n');
    }

    return NULL;
}

//managing hotel rooms
cJSON *manageHotelRooms(cJSON *rooms, cJSON *json) {
    int manage_hotel_rooms_choice;
    printf("You selected manage hotel rooms choose a number:\n ");
    //managing hotel room menu
    while (1) {
        printf("1 - Add hotel room details\n");
        printf("2 - Update hotel room details\n");
        printf("3 - Remove hotel room details\n");
        printf("4 - main menu\n");
        printf("Enter your choice: ");
        scanf("%d", &manage_hotel_rooms_choice);

        switch (manage_hotel_rooms_choice) {
            case 1:
                addHotelRoom(rooms, json);
                break;
            case 2:
                updateHotelRoom(rooms, json);
                break;
            case 3:
                removeHotelRoom(rooms, json);
                break;
            case 4:
                printf("returning to main menu...\n");
                return NULL;
                
            default:
                printf("Invalid choice\n");
                break;
        }
    }
    return NULL;
}
// adding new hotle room
cJSON *addHotelRoom(cJSON *rooms, cJSON *json) {
    char room_ID[50], new_room_type[100], new_room_floor[100], input[10];
    int new_room_no, new_room_price;

    while (1) {
        printf("Enter A to continue or press R to return to the main menu: ");
        scanf("%s", input);

        
        if (input[0] == 'R' || input[0] == 'r') {
            printf("Returning to the main menu\n");
            return NULL;
        }

        // Check if input is 'A' or 'a'
        if (input[0] == 'A' || input[0] == 'a') {
            

            // Check for existing room number
            printf("Enter new room number: ");
            scanf("%d", &new_room_no);

            int room_exists = 0;
            int rooms_count = cJSON_GetArraySize(rooms);
            for (int i = 0; i < rooms_count; i++) {
                cJSON *room = cJSON_GetArrayItem(rooms, i);
                cJSON *room_number = cJSON_GetObjectItem(room, "room_number");
                if (room_number && room_number->valueint == new_room_no) {
                    room_exists = 1;
                    printf("Room with number '%d' already exists.\n", new_room_no);
                    break;
                }
            }
            if (room_exists) {
                continue;
            }

            printf("Enter new room floor (Ground floor, First floor, Second floor): ");
            scanf(" %[^\n]", new_room_floor); // reads the space
            //checks the floor is written correctly
            if (strcasecmp(new_room_floor, "Ground floor") != 0 &&
                strcasecmp(new_room_floor, "First floor") != 0 &&
                strcasecmp(new_room_floor, "Second floor") != 0) {
                printf("Invalid floor entered. Please enter 'Ground floor', 'First floor', or 'Second floor'.\n");
                continue;
            }

            printf("Enter new room type: ");
            scanf("%s", new_room_type);
            //checks the type is wrriten correctly
            // Generate room ID based on the type and number
            if (strcasecmp(new_room_type, "standard") == 0) {
                snprintf(room_ID, sizeof(room_ID), "S%d-Standard", new_room_no);
            } else if (strcasecmp(new_room_type, "deluxe") == 0) {
                snprintf(room_ID, sizeof(room_ID), "D%d-Deluxe", new_room_no);
            } else if (strcasecmp(new_room_type, "suite") == 0) {
                snprintf(room_ID, sizeof(room_ID), "SU%d-Suite", new_room_no);
            } else {
                printf("Invalid room type entered.\n");
                continue;
            }

            printf("Enter new room price: ");
            scanf("%d", &new_room_price);

            // Create new room object
            cJSON *new_room = cJSON_CreateObject();
            cJSON_AddStringToObject(new_room, "ID", room_ID);
            cJSON_AddNumberToObject(new_room, "room_number", new_room_no);
            cJSON_AddStringToObject(new_room, "room_floor", new_room_floor);
            cJSON_AddStringToObject(new_room, "type", new_room_type);
            cJSON_AddNumberToObject(new_room, "price", new_room_price);
            cJSON_AddStringToObject(new_room, "status", "Available");

            // Insert the new room in the correct position based on room number
            int inserted = 0;
            for (int i = 0; i < rooms_count; i++) {
                cJSON *room = cJSON_GetArrayItem(rooms, i);
                cJSON *room_number = cJSON_GetObjectItem(room, "room_number");
                if (room_number && new_room_no < room_number->valueint) {
                    cJSON_InsertItemInArray(rooms, i, new_room);
                    inserted = 1;
                    break;
                }
            }
            if (!inserted) {
                cJSON_AddItemToArray(rooms, new_room);
            }
            
            writeFile(json);
            printf("Room added successfully.\n");
            break;
        } else {
            printf("Invalid input. Please enter A to continue or R to return to the main menu.\n");
        }
    }
    return NULL;
}

//update hotel room status or price
cJSON *updateHotelRoom(cJSON *rooms, cJSON *json) {
    char room_ID[50], room_update_new[50], room_status_new[10];
    int room_new_price;

    printf("Enter the ID for the room or press R to go back to the main menu: ");
    scanf("%s", room_ID);

    
    if (strcasecmp(room_ID, "R") == 0) {
        printf("Returning to the main menu\n\n");
        return NULL;
    }

    // Check if the room ID exists
    int room_exists = 0;
    if (rooms) {
        int rooms_count = cJSON_GetArraySize(rooms);
        for (int i = 0; i < rooms_count; i++) {
            cJSON *room = cJSON_GetArrayItem(rooms, i);
            cJSON *ID = cJSON_GetObjectItem(room, "ID");
            if (ID && strcmp(cJSON_GetStringValue(ID), room_ID) == 0) {
                room_exists = 1;
                printf("Room with ID '%s' exists.\n", room_ID);

                while (1) { 
                    printf("What do you want to change? (price or status) or press R to go back to the main menu: ");
                    scanf("%s", room_update_new);

                    
                    if (strcasecmp(room_update_new, "R") == 0) {
                        printf("Returning to the main menu\n\n");
                        return NULL;
                    }

                    //  updating price
                    if (strcasecmp(room_update_new, "price") == 0) {
                        printf("Enter the new price or press R to go back: ");
                        scanf("%d", &room_new_price);
                        if (room_new_price > 0) {
                            cJSON_ReplaceItemInObject(room, "price", cJSON_CreateNumber(room_new_price));
                            writeFile(json);
                            printf("Price updated successfully.\n");
                        } else {
                            printf("Invalid price entered.\n");
                        }
                    }
                    //  updating status
                    else if (strcasecmp(room_update_new, "status") == 0) {
                        while (1) {
                            printf("Enter the number for the new status:\n");
                            printf("1 - Available\n");
                            printf("2 - Occupied\n");
                            printf("3 - Reserved\n");
                            printf("4 - Under Maintenance\n");
                            printf("Or press R to go back to the main menu: ");
                            scanf("%s", room_status_new);

                            
                            if (strcasecmp(room_status_new, "R") == 0) {
                                printf("Returning to the main menu\n\n");
                                return NULL;
                            }

                            // Convert room_status_new to an integer
                            int status_choice = atoi(room_status_new);

                            // Check for valid status choice
                            if (status_choice == 1) {
                                cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("available"));
                                writeFile(json);
                                printf("Status updated successfully to 'available'.\n");
                            } else if (status_choice == 2) {
                                cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("occupied"));
                                writeFile(json);
                                printf("Status updated successfully to 'occupied'.\n");
                            } else if (status_choice == 3) {
                                cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("reserved"));
                                writeFile(json);
                                printf("Status updated successfully to 'reserved'.\n");
                            } else if (status_choice == 4) {
                                cJSON_ReplaceItemInObject(room, "status", cJSON_CreateString("Under Maintenance"));
                                writeFile(json);
                                printf("Status updated successfully to 'Under Maintenance'.\n");
                            } else {
                                printf("Invalid choice. Please enter a valid number (1-4).\n");
                                continue;
                            }
                            break;
                        }
                    } else {
                        printf("Invalid choice. Please enter 'price' or 'status'.\n");
                        continue;
                    }
                    break;
                }
            }
        }
    }

    if (!room_exists) {
        printf("Room with ID '%s' does not exist.\n", room_ID);
    }

    return NULL;
}

// remove hotel room
cJSON *removeHotelRoom(cJSON *rooms, cJSON *json) {
    char room_ID[50], yesNo_delete_room[10];
    int found = 0;

    while (1) {
        printf("Enter the ID for the room or press R to go back to main menu: ");
        scanf("%s", room_ID);
        
        // Check if user wants to return to the main menu
        if (strcmp(room_ID, "R") == 0 || strcmp(room_ID, "r") == 0) {
            printf("Returning to main menu\n\n");
            return NULL;
        }
        //  Check if the room ID exists
        if (rooms) {
            int rooms_count = cJSON_GetArraySize(rooms);
            for (int i = 0; i < rooms_count; i++) {
                cJSON *room = cJSON_GetArrayItem(rooms, i);
                cJSON *ID = cJSON_GetObjectItem(room, "ID");
                if (ID && strcmp(cJSON_GetStringValue(ID), room_ID) == 0) {
                    found = 1;
                    printf("Are you sure you want to delete this room (YES/NO): ");
                    scanf("%s", yesNo_delete_room);
                    
                    if (strcasecmp(yesNo_delete_room, "NO") == 0) {
                        printf("Returning\n");
                        break;
                    } else if (strcasecmp(yesNo_delete_room, "YES") == 0) {
                        cJSON_DeleteItemFromArray(rooms, i);
                        writeFile(json); 
                        printf("Room has been deleted successfully\n");
                        return NULL; // Exit after successful deletion
                    }
                }
            }
            if (!found) {
                printf("Room ID is not available\n");
            }
        }
    }
    return NULL;
}
// logging out
cJSON *logOut(int *is_logged_in, cJSON *json) {
    char log_out_request[50];
    printf("Are you sure you want to log out? (yes/no): ");
    scanf("%s", log_out_request);

    if (strcmp(log_out_request, "yes") == 0) {
        *is_logged_in = 0;
        printf("Logged out\n");
    } else if (strcmp(log_out_request, "no") == 0) {
        printf("Returning to main menu\n");
    } else {
        printf("Invalid input\n");
    }
    return NULL;
}
////////////////


//Osamah Ahmed
//Guest
////////////////
cJSON *guestMenu(cJSON *json){
    int count = 1;
    char guestViewChoice[2];

    cJSON* rooms = cJSON_GetObjectItem(json, "rooms");
    if (rooms) {
        int rooms_count = cJSON_GetArraySize(rooms);
        for (int i = 0; i < rooms_count; i++) {
            cJSON *room = cJSON_GetArrayItem(rooms, i);

            cJSON *roomID = cJSON_GetObjectItem(room, "ID");
            cJSON *room_floor = cJSON_GetObjectItem(room, "room_floor");
            cJSON *room_type = cJSON_GetObjectItem(room, "type");
            cJSON *room_number = cJSON_GetObjectItem(room, "room_number");
            cJSON *room_price = cJSON_GetObjectItem(room, "price");
            cJSON *room_status = cJSON_GetObjectItem(room, "status");

            if (strcmp(cJSON_GetStringValue(room_status), "Available") == 0){
                printf("%d.\n", count);
                printf("Room ID: %s\n", roomID->valuestring);
                printf("Room: %d - %s\n", room_number->valueint, room_floor->valuestring);
                printf("Type: %s\n", room_type->valuestring);
                printf("Price: %d\n\n", room_price->valueint);
                count++;
            }

            }
        while (1){
            printf("Enter 0 to exit: ");
            
            fgets(guestViewChoice, sizeof(guestViewChoice), stdin);

            if (atoi(guestViewChoice) == 0){
                system("cls");
                return NULL;
            }
        }
    } else {
        printf("No room data found!");
    }
    free(rooms);
}
////////////////


//Mahmood Qaid
//Main
////////////////
cJSON* logIn(cJSON *json){
    //initialize variables
    char enteredUsername[16];
    char enteredPassword[16];
    
    char passmask;

    int attempts = 3;
    bool isLoggedIn = false;

    //reading users
    cJSON *users = cJSON_GetObjectItem(json, "users");

    while (attempts + 1 > 0){
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
                    printf("Hello again %s!\n", firstname);
                    
                    if (strcmp(cJSON_GetStringValue(role), "1") == 0){
                        adminMenu(json, user, users);
                        return NULL;
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "2") == 0){
                        staffMenu(json, user, users);
                        return NULL;
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "3") == 0){
                        customerMenu(json, user, users);
                        return NULL;
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
                system("cls");
                printf("No more attempts, try again later!");
                break;
            }
            else {
                system("cls");
                printf("Invalid username or password! Try again (%d)\n", attempts);
                attempts--;
            }
        }
    }
    free(users);
}

int main() {

    //initialize variables
    char mainMenuChoice[10];

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

        
    while (1){    
        printf("╔════════════════════════════╗\n");
        printf("║ Welcome to Sheba Hotel App ║\n");
        printf("╚════════════════════════════╝\n");
        
        printf("1.log in\n2.guest\n3.Exit\n");
        fgets(mainMenuChoice, sizeof(mainMenuChoice), stdin);

        system("cls");
        switch (atoi(mainMenuChoice))
        {
        case 1:
            logIn(json);
            break;
        
        case 2:
            guestMenu(json);
            break;

        case 3:
            printf("Goodbye...\n");
            return false;
            break;
        
        default:
            system("cls");
            printf("Invalid option!\n");
        }
        clearInputBuffer();
    }

    //clean memory up
    cJSON_Delete(json);
    free(content);

    return 0;
}
////////////////