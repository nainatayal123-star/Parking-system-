#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#ifndef PARKING_H
#define PARKING_H

/* ---------------- Slot counts ---------------- */
#define NORMAL_TWO_SLOTS 150
#define NORMAL_FOUR_SLOTS 100
#define RESERVED_SLOTS 50
#define TOTAL_SLOTS 300 /* 150 + 100 + 50 */

/* ---------------- Vehicle types ---------------- */
#define TWO_WHEELER 2
#define FOUR_WHEELER 4

/* ---------------- Slot categories ---------------- */
#define SLOT_NORMAL_TWO 2
#define SLOT_NORMAL_FOUR 4
#define SLOT_RESERVED 3

/* ---------------- Fees (in rupees) ---------------- */
#define RATE_TWO_PER_HOUR 20
#define RATE_FOUR_PER_HOUR 40
#define RATE_TWO_HALF_HOUR 10
#define RATE_FOUR_HALF_HOUR 20

#define PASS_PRICE_TWO 500
#define PASS_PRICE_FOUR 800
#define PASS_DAYS 30

/* ---------------- Other settings ---------------- */
#define PLATE_LEN 15
#define NAME_LEN 30
#define MAX_PASSES 500
#define ADMIN_PASSWORD "Parking321@"

#define DATA_FILE "parking_data.txt"

/* Results of passStatus() */
#define PASS_NONE 0
#define PASS_VALID 1
#define PASS_EXPIRED 2

/* ---------------- Structures ---------------- */
typedef struct
{
    char plate[PLATE_LEN];
    int type;
    time_t entryTime;
    int isPassHolder;
} Vehicle;

typedef struct
{
    int slotNo;      /* 1 to 300 */
    int category;    /* SLOT_NORMAL_TWO / SLOT_NORMAL_FOUR / SLOT_RESERVED */
    int isOccupied;  /* 0 = free, 1 = occupied */
    Vehicle vehicle; /* valid only when isOccupied is 1 */
} Slot;

typedef struct
{
    char plate[PLATE_LEN];
    char ownerName[NAME_LEN];
    int type;
    time_t startDate;
    time_t expiryDate; /* startDate + PASS_DAYS days */
    int active;        /* 0 = cancelled */
} MonthlyPass;

typedef struct
{
    char date[11];     /* "DD-MM-YYYY" */
    int vehiclesToday; /* vehicles parked today */
    int hourlyAmount;  /* rupees collected from hourly fees today */
    int passesSoldToday;
    int passAmount;    /* rupees collected from pass sales today */
} DailyRecord;

/* ---------------- Global Variables ---------------- */
Slot lot[TOTAL_SLOTS];
MonthlyPass passes[MAX_PASSES];
int passCount = 0;
DailyRecord today = {0};

/* ---------------- Forward Declarations ---------------- */
void initSlots(void);
int findParkedVehicle(const char *plate);
void formatPlate(char *plate);
int readInt(const char *prompt, int min, int max);
void readString(const char *prompt, char *dest, int size);
void viewSlots(void);
int loadData(void);
int saveData(void);
void resetDailyIfNewDay(void);

int findFreeSlot(int type, int isPassHolder);
int minutesParked(time_t entry, time_t exitTime);
int calcFee(int type, int minutes);
void vehicleEntry(void);
void vehicleExit(void);
void searchVehicle(void);
void showDailyReport(void);

int findPass(const char *plate);
int passStatus(const char *plate);
void registerPass(void);
void renewPass(void);
void cancelPass(void);
void listPasses(void);
int adminLogin(void);
void adminMenu(void);

#endif

/* =====================================================
 *  PERSON 1 : Core Logic and Real Time
 * ===================================================== */

int findFreeSlot(int type, int isPassHolder)
{
    /* 1. Pass holders first try reserved slots (251-300: indices 250-299) */
    if (isPassHolder == 1)
    {
        for (int i = 250; i < 300; i++)
        {
            if (lot[i].isOccupied == 0)
                return i + 1;
        }
    }

    /* 2. Check regular category slots */
    if (type == TWO_WHEELER)
    {
        /* Slots 1-150: indices 0-149 */
        for (int i = 0; i < 150; i++)
        {
            if (lot[i].isOccupied == 0)
                return i + 1;
        }
    }
    else if (type == FOUR_WHEELER)
    {
        /* Slots 151-250: indices 150-249 */
        for (int i = 150; i < 250; i++)
        {
            if (lot[i].isOccupied == 0)
                return i + 1;
        }
    }

    /* 3. Pass holders overflow into any remaining regular slot if reserved is full */
    if (isPassHolder == 1)
    {
        for (int i = 0; i < 250; i++)
        {
            if (lot[i].isOccupied == 0)
                return i + 1;
        }
    }

    return -1;
}

int minutesParked(time_t entry, time_t exitTime)
{
    if (exitTime <= entry)
        return 0;
    return (int)((exitTime - entry) / 60);
}

int calcFee(int type, int minutes)
{
    if (minutes <= 0)
        return 0;

    int halfRate = (type == TWO_WHEELER) ? RATE_TWO_HALF_HOUR : RATE_FOUR_HALF_HOUR;
    int hourRate = (type == TWO_WHEELER) ? RATE_TWO_PER_HOUR : RATE_FOUR_PER_HOUR;

    if (minutes <= 30)
        return halfRate;

    int hours = (minutes + 59) / 60;
    return hours * hourRate;
}

void vehicleEntry(void)
{
    char plate_number[PLATE_LEN];
    int vehicle_type = 0;

    printf("\n--- Vehicle Entry ---\n");
    readString("Enter the plate number: ", plate_number, PLATE_LEN);
    formatPlate(plate_number);

    if (plate_number[0] == '\0')
    {
        printf("Plate number cannot be empty.\n");
        return;
    }

    int parked = findParkedVehicle(plate_number);
    if (parked != -1)
    {
        printf("The vehicle is already parked in slot %d\n", parked + 1);
        return;
    }

    int pStatus = passStatus(plate_number);
    int passholder = (pStatus == PASS_VALID) ? 1 : 0;

    if (pStatus == PASS_EXPIRED)
    {
        printf("Your pass has expired. Standard rates apply.\n");
    }

    if (passholder == 1)
    {
        int pIdx = findPass(plate_number);
        vehicle_type = passes[pIdx].type;
        printf("Valid pass found (%s).\n", vehicle_type == TWO_WHEELER ? "Two-wheeler" : "Four-wheeler");
    }
    else
    {
        vehicle_type = readInt("Enter the type of vehicle (2 = Two-Wheeler, 4 = Four-Wheeler): ", 2, 4);
        while (vehicle_type != TWO_WHEELER && vehicle_type != FOUR_WHEELER)
        {
            printf("Invalid vehicle type. Please enter 2 or 4.\n");
            vehicle_type = readInt("Enter the type of vehicle (2 = Two-Wheeler, 4 = Four-Wheeler): ", 2, 4);
        }
    }

    int slot = findFreeSlot(vehicle_type, passholder);
    if (slot == -1)
    {
        printf("No free slot available for your vehicle.\n");
        return;
    }

    int idx = slot - 1;
    lot[idx].isOccupied = 1;
    strncpy(lot[idx].vehicle.plate, plate_number, PLATE_LEN - 1);
    lot[idx].vehicle.plate[PLATE_LEN - 1] = '\0';
    lot[idx].vehicle.type = vehicle_type;
    lot[idx].vehicle.entryTime = time(NULL);
    lot[idx].vehicle.isPassHolder = passholder;

    today.vehiclesToday++;

    struct tm *t = localtime(&lot[idx].vehicle.entryTime);
    char time_string[30];
    strftime(time_string, sizeof(time_string), "%H:%M:%S", t);

    printf("Assigned Slot: #%d\n", slot);
    printf("Entry Time   : %s\n", time_string);
}

void vehicleExit(void)
{
    char plate_number[PLATE_LEN];
    printf("\n--- Vehicle Exit ---\n");
    readString("Enter the plate number: ", plate_number, PLATE_LEN);
    formatPlate(plate_number);

    int parked = findParkedVehicle(plate_number);
    if (parked == -1)
    {
        printf("The vehicle is not parked in this lot.\n");
        return;
    }

    time_t exit_time = time(NULL);
    int min = minutesParked(lot[parked].vehicle.entryTime, exit_time);
    int fee = 0;

    if (lot[parked].vehicle.isPassHolder == 1)
    {
        printf("Monthly pass holder verified. Parking fee: Rs. 0\n");
    }
    else
    {
        fee = calcFee(lot[parked].vehicle.type, min);
        today.hourlyAmount += fee;
        printf("Parked duration: %d minutes. Total fee: Rs. %d\n", min, fee);
    }

    lot[parked].isOccupied = 0;
    memset(&lot[parked].vehicle, 0, sizeof(Vehicle));
    printf("Vehicle exited. Slot %d is now free.\n", parked + 1);
}

void searchVehicle(void)
{
    char plate_number[PLATE_LEN];
    readString("Enter the plate number to search: ", plate_number, PLATE_LEN);
    formatPlate(plate_number);

    int parked = findParkedVehicle(plate_number);
    if (parked == -1)
    {
        printf("Vehicle %s is not currently parked.\n", plate_number);
        return;
    }

    struct tm *t = localtime(&lot[parked].vehicle.entryTime);
    char time_string[30];
    strftime(time_string, sizeof(time_string), "%d-%m-%Y %H:%M:%S", t);

    printf("\nVehicle found in slot #%d\n", parked + 1);
    printf("Type       : %s\n", lot[parked].vehicle.type == TWO_WHEELER ? "Two-wheeler" : "Four-wheeler");
    printf("Entry Time : %s\n", time_string);
    printf("Pass Holder: %s\n", lot[parked].vehicle.isPassHolder ? "Yes" : "No");
}

void showDailyReport(void)
{
    printf("\n--- DAILY REPORT (%s) ---\n", today.date);
    printf("Vehicles parked today  : %d\n", today.vehiclesToday);
    printf("Hourly fees collected  : Rs. %d\n", today.hourlyAmount);
    printf("Passes sold today      : %d\n", today.passesSoldToday);
    printf("Pass amount collected  : Rs. %d\n", today.passAmount);
    printf("Total revenue today    : Rs. %d\n", today.hourlyAmount + today.passAmount);
}

/* =====================================================
 *  PERSON 2 : Foundation, Helpers and Storage
 * ===================================================== */

void initSlots(void)
{
    for (int i = 0; i < TOTAL_SLOTS; i++)
    {
        lot[i].slotNo = i + 1;
        if (i < 150)
            lot[i].category = SLOT_NORMAL_TWO;
        else if (i < 250)
            lot[i].category = SLOT_NORMAL_FOUR;
        else
            lot[i].category = SLOT_RESERVED;

        lot[i].isOccupied = 0;
        memset(&lot[i].vehicle, 0, sizeof(Vehicle));
    }
}

int findParkedVehicle(const char *plate)
{
    for (int i = 0; i < TOTAL_SLOTS; i++)
    {
        if (lot[i].isOccupied && strcmp(lot[i].vehicle.plate, plate) == 0)
        {
            return i;
        }
    }
    return -1;
}

void formatPlate(char *plate)
{
    if (plate == NULL)
        return;

    int read_index = 0;
    int write_index = 0;

    while (plate[read_index] != '\0')
    {
        unsigned char c = (unsigned char)plate[read_index];
        if (!isspace(c))
            plate[write_index++] = (char)toupper(c);
        read_index++;
    }
    plate[write_index] = '\0';
}

int readInt(const char *prompt, int min, int max)
{
    int value = 0;
    while (1)
    {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1)
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
            if (value >= min && value <= max)
                return value;
        }
        else
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        printf("Invalid input. Please enter a number between %d and %d.\n", min, max);
    }
}

void readString(const char *prompt, char *dest, int size)
{
    if (dest == NULL || size <= 0)
        return;

    while (1)
    {
        printf("%s", prompt);
        if (fgets(dest, size, stdin) != NULL)
        {
            dest[strcspn(dest, "\r\n")] = '\0';
            if (dest[0] != '\0')
                return;
        }
        printf("Input cannot be empty. Please try again.\n");
    }
}

void viewSlots(void)
{
    int freeTwo = 0, freeFour = 0, freeReserved = 0;

    for (int i = 0; i < TOTAL_SLOTS; i++)
    {
        if (!lot[i].isOccupied)
        {
            if (lot[i].category == SLOT_NORMAL_TWO)
                freeTwo++;
            else if (lot[i].category == SLOT_NORMAL_FOUR)
                freeFour++;
            else if (lot[i].category == SLOT_RESERVED)
                freeReserved++;
        }
    }

    printf("\n--- SLOT SUMMARY ---\n");
    printf("Free Two-Wheeler Slots  : %d / %d\n", freeTwo, NORMAL_TWO_SLOTS);
    printf("Free Four-Wheeler Slots : %d / %d\n", freeFour, NORMAL_FOUR_SLOTS);
    printf("Free Reserved Slots     : %d / %d\n", freeReserved, RESERVED_SLOTS);
    printf("---------------------\n");

    printf("\nView Options:\n");
    printf("1. View All Slots\n");
    printf("2. View Free Slots Only\n");
    printf("3. View Reserved Slots Only\n");

    int choice = readInt("Enter choice (1-3): ", 1, 3);

    printf("\nSlot No\tCategory\tStatus\t\tPlate\n");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < TOTAL_SLOTS; i++)
    {
        int show = 0;

        if (choice == 1)
            show = 1;
        else if (choice == 2 && !lot[i].isOccupied)
            show = 1;
        else if (choice == 3 && lot[i].category == SLOT_RESERVED)
            show = 1;

        if (show)
        {
            const char *catStr = "Normal 2W";
            if (lot[i].category == SLOT_NORMAL_FOUR)
                catStr = "Normal 4W";
            else if (lot[i].category == SLOT_RESERVED)
                catStr = "Reserved";

            if (lot[i].isOccupied)
                printf("%-7d\t%-12s\tOccupied\t%s\n", lot[i].slotNo, catStr, lot[i].vehicle.plate);
            else
                printf("%-7d\t%-12s\tFree\t\t-\n", lot[i].slotNo, catStr);
        }
    }
}

int loadData(void)
{
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL)
        return 0;

    for (int i = 0; i < TOTAL_SLOTS; i++)
    {
        char tempPlate[PLATE_LEN];
        long tempTime = 0;

        if (fscanf(fp, "%d %d %d %14s %ld",
                   &lot[i].slotNo,
                   &lot[i].category,
                   &lot[i].isOccupied,
                   tempPlate,
                   &tempTime) != 5)
        {
            fclose(fp);
            return 0;
        }

        lot[i].vehicle.entryTime = (time_t)tempTime;

        if (strcmp(tempPlate, "NONE") != 0 && lot[i].isOccupied)
        {
            strncpy(lot[i].vehicle.plate, tempPlate, PLATE_LEN - 1);
            lot[i].vehicle.plate[PLATE_LEN - 1] = '\0';
        }
        else
        {
            lot[i].vehicle.plate[0] = '\0';
        }
    }

    fclose(fp);
    return 1;
}

int saveData(void)
{
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL)
        return 0;

    for (int i = 0; i < TOTAL_SLOTS; i++)
    {
        const char *plate = (lot[i].isOccupied && lot[i].vehicle.plate[0] != '\0') 
                            ? lot[i].vehicle.plate 
                            : "NONE";

        fprintf(fp, "%d %d %d %s %ld\n",
                lot[i].slotNo,
                lot[i].category,
                lot[i].isOccupied,
                plate,
                (long)lot[i].vehicle.entryTime);
    }

    fclose(fp);
    return 1;
}

void resetDailyIfNewDay(void)
{
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);

    static int last_day = -1;

    if (last_day == -1)
    {
        strftime(today.date, sizeof(today.date), "%d-%m-%Y", current_time);
        last_day = current_time->tm_mday;
        return;
    }

    if (current_time->tm_mday != last_day)
    {
        printf("\n[Info] New day detected. Resetting daily statistics...\n");
        strftime(today.date, sizeof(today.date), "%d-%m-%Y", current_time);
        today.vehiclesToday = 0;
        today.hourlyAmount = 0;
        today.passesSoldToday = 0;
        today.passAmount = 0;
        last_day = current_time->tm_mday;
    }
}

/* =====================================================
 *  PERSON 3 : Passes, Admin and Main Menu
 * ===================================================== */

int findPass(const char *plate)
{
    for (int i = 0; i < passCount; i++)
    {
        if (passes[i].active == 1 && strcmp(passes[i].plate, plate) == 0)
            return i;
    }
    return -1;
}

int passStatus(const char *plate)
{
    int index = findPass(plate);
    if (index == -1)
        return PASS_NONE;

    time_t currentTime = time(NULL);
    if (currentTime <= passes[index].expiryDate)
        return PASS_VALID;
    else
        return PASS_EXPIRED;
}

void registerPass(void)
{
    char plate[PLATE_LEN];
    char name[NAME_LEN];
    int typeChoice, type, price, index, confirm;
    time_t currentTime;

    printf("\n***** REGISTER MONTHLY PASS *****\n");

    if (passCount >= MAX_PASSES)
    {
        printf("Pass list is full.\n");
        return;
    }

    readString("Enter owner's name: ", name, NAME_LEN);
    readString("Enter vehicle number: ", plate, PLATE_LEN);
    formatPlate(plate);

    if (strlen(plate) == 0)
    {
        printf("Vehicle number cannot be empty.\n");
        return;
    }

    index = findPass(plate);
    if (index != -1)
    {
        printf("This vehicle already has a pass. Please renew the existing pass.\n");
        return;
    }

    printf("Select vehicle type:\n");
    printf("1. Two Wheeler\n");
    printf("2. Four Wheeler\n");

    typeChoice = readInt("Enter your choice of vehicle (1 or 2): ", 1, 2);
    if (typeChoice == 1)
    {
        type = TWO_WHEELER;
        price = PASS_PRICE_TWO;
    }
    else
    {
        type = FOUR_WHEELER;
        price = PASS_PRICE_FOUR;
    }

    printf("Pass price: Rs. %d\n", price);
    printf("Pass validity: %d days\n", PASS_DAYS);

    confirm = readInt("Confirm payment? (1=Yes, 0=No): ", 0, 1);
    if (confirm == 0)
    {
        printf("Pass registration cancelled.\n");
        return;
    }

    currentTime = time(NULL);
    strncpy(passes[passCount].plate, plate, PLATE_LEN - 1);
    passes[passCount].plate[PLATE_LEN - 1] = '\0';
    strncpy(passes[passCount].ownerName, name, NAME_LEN - 1);
    passes[passCount].ownerName[NAME_LEN - 1] = '\0';

    passes[passCount].type = type;
    passes[passCount].startDate = currentTime;
    passes[passCount].expiryDate = currentTime + (time_t)PASS_DAYS * 24 * 60 * 60;
    passes[passCount].active = 1;
    passCount++;

    today.passesSoldToday++;
    today.passAmount += price;

    printf("Pass registered successfully!\n");
    printf("Vehicle Number: %s\n", plate);
    printf("Expiry Date   : %s", ctime(&passes[passCount - 1].expiryDate));
}

void renewPass(void)
{
    char plate[PLATE_LEN];
    int index, price;
    time_t currentTime;
    time_t newStartDate;

    printf("\n***** RENEW MONTHLY PASS *****\n");
    readString("Enter the vehicle number: ", plate, PLATE_LEN);
    formatPlate(plate);

    if (strlen(plate) == 0)
    {
        printf("Vehicle number cannot be empty.\n");
        return;
    }

    index = findPass(plate);
    if (index == -1)
    {
        printf("Pass not found. Please register first.\n");
        return;
    }

    if (passes[index].type == TWO_WHEELER)
        price = PASS_PRICE_TWO;
    else
        price = PASS_PRICE_FOUR;

    currentTime = time(NULL);
    if (passes[index].expiryDate > currentTime)
        newStartDate = passes[index].expiryDate;
    else
        newStartDate = currentTime;

    passes[index].startDate = newStartDate;
    passes[index].expiryDate = newStartDate + (time_t)PASS_DAYS * 24 * 60 * 60;

    today.passesSoldToday++;
    today.passAmount += price;

    printf("Pass renewed successfully!\n");
    printf("Renewal amount : Rs. %d\n", price);
    printf("New expiry date: %s", ctime(&passes[index].expiryDate));
}

void cancelPass(void)
{
    char plate[PLATE_LEN];
    int index, confirm;

    printf("\n****** CANCEL MONTHLY PASS ******\n");
    readString("Enter vehicle number: ", plate, PLATE_LEN);
    formatPlate(plate);

    if (strlen(plate) == 0)
    {
        printf("Vehicle number cannot be empty.\n");
        return;
    }

    index = findPass(plate);
    if (index == -1)
    {
        printf("Pass not found.\n");
        return;
    }

    confirm = readInt("Are you sure you want to cancel? (1=Yes, 0=No): ", 0, 1);
    if (confirm == 0)
    {
        printf("Pass cancellation stopped.\n");
        return;
    }

    passes[index].active = 0;
    printf("Pass cancelled successfully. No refund will be given.\n");
}

void listPasses(void)
{
    int count = 0;

    printf("\n******* MONTHLY PASSES *******\n");
    for (int i = 0; i < passCount; i++)
    {
        if (passes[i].active == 1)
        {
            count++;
            printf("Owner name    : %s\n", passes[i].ownerName);
            printf("Vehicle number: %s\n", passes[i].plate);
            printf("Vehicle type  : %s\n", passes[i].type == TWO_WHEELER ? "Two wheeler" : "Four wheeler");
            printf("Expiry Date   : %s", ctime(&passes[i].expiryDate));
            printf("Pass Status   : %s\n", passStatus(passes[i].plate) == PASS_VALID ? "VALID" : "EXPIRED");
            printf("------------------------------\n");
        }
    }

    if (count == 0)
        printf("No active passes found.\n");
}

int adminLogin(void)
{
    char password[50];
    printf("\n******** ADMIN LOGIN *********\n");
    for (int attempts = 1; attempts <= 3; attempts++)
    {
        readString("Enter admin password: ", password, 50);

        if (strcmp(password, ADMIN_PASSWORD) == 0)
        {
            printf("Login successful.\n");
            return 1;
        }
        else
        {
            printf("Incorrect password (%d/3 attempts used).\n", attempts);
        }
    }
    printf("Three incorrect attempts. Access denied.\n");
    return 0;
}

void adminMenu(void)
{
    int choice;
    do
    {
        printf("\n********* ADMIN MENU *********\n");
        printf("1. Daily report\n");
        printf("2. Register monthly pass\n");
        printf("3. Renew monthly pass\n");
        printf("4. Cancel monthly pass\n");
        printf("5. List of monthly passes\n");
        printf("6. Back to Main Menu\n");

        choice = readInt("Enter your choice: ", 1, 6);

        switch (choice)
        {
        case 1:
            showDailyReport();
            break;
        case 2:
            registerPass();
            saveData();
            break;
        case 3:
            renewPass();
            saveData();
            break;
        case 4:
            cancelPass();
            saveData();
            break;
        case 5:
            listPasses();
            break;
        case 6:
            break;
        }
    } while (choice != 6);
}

/* =====================================================
 *  Main Program
 * ===================================================== */

int main(void)
{
    int choice;

    initSlots();
    loadData();
    resetDailyIfNewDay();

    printf("\n====================================\n");
    printf("   CAMPUS PARKING MANAGEMENT SYSTEM\n");
    printf("====================================\n");

    do
    {
        resetDailyIfNewDay();

        printf("\n--------- MAIN MENU ---------\n");
        printf("1. Vehicle entry\n");
        printf("2. Vehicle exit\n");
        printf("3. Search vehicle\n");
        printf("4. View parking slots\n");
        printf("5. Admin\n");
        printf("6. Quit\n");

        choice = readInt("Enter your choice: ", 1, 6);
        switch (choice)
        {
        case 1:
            vehicleEntry();
            saveData();
            break;

        case 2:
            vehicleExit();
            saveData();
            break;

        case 3:
            searchVehicle();
            break;

        case 4:
            viewSlots();
            break;

        case 5:
            if (adminLogin() == 1)
            {
                adminMenu();
                saveData();
            }
            break;

        case 6:
            saveData();
            printf("Data saved successfully. Thank you for using the system!\n");
            break;
        }
    } while (choice != 6);

    return 0;
}
