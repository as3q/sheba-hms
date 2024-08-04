#include <stdio.h>
#include <time.h>
#include <stdlib.h> //free malloc exit
#include <conio.h> //getch
#include <string.h> //strcmp strlen
#include <stdbool.h> //for booleans
#include "cjson/cJSON.h" //reading json

cJSON* writeFile(cJSON *json){
    FILE *fp = fopen("data.txt", "w+"); 
    
    // write into file
   char *json_str = cJSON_Print(json);
   fputs(json_str, fp); 
   fclose(fp);
   cJSON_free(json_str); 
}

//clears input buffer (for fgets)
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

//to parse string time
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

//CANCEL RESERVATIONS AFTER 3 DAYS
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

//OK
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

//OK
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

//OK
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

//OK
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

//OK
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
            break;

        case 5:
            return false;
            break;
        
        default:
            printf("Invalid option!\n");
        };
    }
}

void customerMenu(){
    printf("customer menu");   
}

void guestMenu(cJSON *json){
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
            
            
            clearInputBuffer();
            fgets(guestViewChoice, sizeof(guestViewChoice), stdin);

            if (strcmp(guestViewChoice, "0") == 0){
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
        scanf("%15s", enteredUsername); //scanf

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
                cJSON *role = cJSON_GetObjectItem(user, "role");
                cJSON *fullname = cJSON_GetObjectItem(user, "fullname");

                char *firstname = strtok(cJSON_GetStringValue(fullname), " ");
                if (strcmp(cJSON_GetStringValue(username), enteredUsername) == 0 &&
                    strcmp(cJSON_GetStringValue(password), enteredPassword) == 0) {

                    system("cls");
                    printf("Hello %s!\n", firstname);

                    
                    if (strcmp(cJSON_GetStringValue(role), "1") == 0){
                        isLoggedIn = true;
                        printf("Admin\n");
                    }
                    else if (strcmp(cJSON_GetStringValue(role), "2") == 0){
                        staffMenu(json, user, users);
                        return false;
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
    char mainMenuChoice[2];

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

    //handle over limit reservation
    //if reservedOn time + 3 days is greater than today then delete reservation

    printf("╔════════════════════════════╗\n");
    printf("║ Welcome to Sheba Hotel App ║\n");
    printf("╚════════════════════════════╝\n");
        
    while (1){    
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
            printf("Exiting...\n");
            return false;
            break;
        
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
