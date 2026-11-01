/*
 * parking.h - Campus Parking Management System
 * Shared header file. All three members include this file.
 * Do NOT change anything here without telling the whole group.
 *
 *   Slot layout (array index = slot number - 1):
 *   Slot   1 - 150  : normal slots for two-wheelers
 *   Slot 151 - 250  : normal slots for four-wheelers
 *   Slot 251 - 300  : reserved slots for monthly pass holders (both types)
 */

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
/* Price of the first 30 minutes. ASSUMED as half of the hourly rate.
   Change these two lines if your group decides a different price. */
#define RATE_TWO_HALF_HOUR 10
#define RATE_FOUR_HALF_HOUR 20

#define PASS_PRICE_TWO 500
#define PASS_PRICE_FOUR 800
#define PASS_DAYS 30

/* ---------------- Other settings ---------------- */
#define PLATE_LEN 15
#define NAME_LEN 30
#define MAX_PASSES 500
#define ADMIN_PASSWORD "Parking321@" /* fixed in code; choose your own */

#define SLOT_FILE "slots.txt"
#define PASS_FILE "passes.txt"
#define DAILY_FILE "daily.txt"

/* Results of passStatus() */
#define PASS_NONE 0
#define PASS_VALID 1
#define PASS_EXPIRED 2

/* Optional, for testing only (Person 2 uses it inside the fee code):
 * remove the comment marks below to make 1 real minute count as 1 hour.
 * Put the comment marks back before the final submission.
 */
/* #define TEST_MODE */

/* ---------------- Structures ---------------- */
typedef struct
{
    char plate[PLATE_LEN]; /* capital letters, no spaces */
    int type;              /* TWO_WHEELER or FOUR_WHEELER */
    time_t entryTime;      /* real time of entry */
    int isPassHolder;      /* 1 if a valid pass was found at entry */
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
    int passAmount; /* rupees collected from pass sales today */
} DailyRecord;

/* ---------------- Shared data (defined in main.c by Person 3) ---------------- */
extern Slot lot[TOTAL_SLOTS];
extern MonthlyPass passes[MAX_PASSES];
extern int passCount;
extern DailyRecord today;
extern time_t entry_time;

/* =====================================================
 *  PERSON 1 : core logic and real time   (entry_exit.c)
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

    /* 3. Pass holders can overflow into any regular slot if reserved is full */
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
    if (exitTime <= entry) return 0;
    return (int)((exitTime - entry) / 60);
}

int calcFee(int type, int minutes)
{
    if (minutes <= 0) return 0;

    int halfRate = (type == TWO_WHEELER) ? RATE_TWO_HALF_HOUR : RATE_FOUR_HALF_HOUR;
    int hourRate = (type == TWO_WHEELER) ? RATE_TWO_PER_HOUR : RATE_FOUR_PER_HOUR;

    if (minutes <= 30)
        return halfRate;

    int hours = (minutes + 59) / 60; /* Round up to next full hour */
    return hours * hourRate;
}

void vehicleEntry(void)
{
    char plate_number[PLATE_LEN];
    int vehicle_type = 0;

    printf("Enter the plate number: ");
    scanf("%14s", plate_number);
    formatPlate(plate_number);

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
        printf("Enter the type of vehicle (2 = two wheeler, 4 = four wheeler): ");
        scanf("%d", &vehicle_type);
        if (vehicle_type != TWO_WHEELER && vehicle_type != FOUR_WHEELER)
        {
            printf("Invalid vehicle type.\n");
            return;
        }
    }

    int slot = findFreeSlot(vehicle_type, passholder);
    if (slot == -1)
    {
        printf("No free slot for your vehicle.\n");
        return;
    }

    /* Assign and fill slot */
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

    printf("The slot number is %d\n", slot);
    printf("Time: %s\n", time_string);
}

void vehicleExit(void)
{
    char plate_number[PLATE_LEN];
    printf("Enter the plate number: ");
    scanf("%14s", plate_number);
    formatPlate(plate_number);

    int parked = findParkedVehicle(plate_number);
    if (parked == -1)
    {
        printf("The vehicle is not parked.\n");
        return;
    }

    time_t exit_time = time(NULL);
    int min = minutesParked(lot[parked].vehicle.entryTime, exit_time);
    int fee = 0;

    if (lot[parked].vehicle.isPassHolder)
    {
        printf("Monthly pass holder. No parking fee.\n");
    }
    else
    {
        fee = calcFee(lot[parked].vehicle.type, min);
        today.hourlyAmount += fee;
        printf("Parked duration: %d minutes. Total fee: Rs. %d\n", min, fee);
    }

    /* Free slot */
    lot[parked].isOccupied = 0;
    memset(&lot[parked].vehicle, 0, sizeof(Vehicle));
    printf("Vehicle exited. Slot %d is now free.\n", parked + 1);
}

void searchVehicle(void)
{
    char plate_number[PLATE_LEN];
    printf("Enter the plate number to search: ");
    scanf("%14s", plate_number);
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

    printf("Vehicle %s found in slot %d\n", plate_number, parked + 1);
    printf("Type: %s | Entry Time: %s | Pass Holder: %s\n",
           lot[parked].vehicle.type == TWO_WHEELER ? "Two-wheeler" : "Four-wheeler",
           time_string,
           lot[parked].vehicle.isPassHolder ? "Yes" : "No");
}

void showDailyReport(void)
{
    printf("\n--- DAILY REPORT (%s) ---\n", today.date);
    printf("Vehicles parked today : %d\n", today.vehiclesToday);
    printf("Hourly amount collected: Rs. %d\n", today.hourlyAmount);
    printf("Passes sold today      : %d\n", today.passesSoldToday);
    printf("Pass amount collected  : Rs. %d\n", today.passAmount);
    printf("Total revenue today    : Rs. %d\n", today.hourlyAmount + today.passAmount);
}

/* =====================================================
 *  PERSON 2 : foundation, helpers and storage   (slots.c, files.c)
 * ===================================================== */
void initSlots(void);                     /* sets slotNo, category, isOccupied = 0 */
int findParkedVehicle(const char *plate); /* returns array index of the slot, or -1 */
void formatPlate(char *plate);            /* capital letters, remove spaces */
int readInt(const char *prompt, int min, int max);
void readString(const char *prompt, char *dest, int size);
void viewSlots(void);          /* all / free / reserved */
int loadData(void);            /* read slots, passes, daily record.
                                  Must restore entryTime and isPassHolder. */
int saveData(void);            /* write all of them back */
void resetDailyIfNewDay(void); /* if date changed, start a new daily record */

/* =====================================================
 *  PERSON 3 : passes, admin and main menu   (passes.c, main.c)
 * ===================================================== */
int findPass(const char *plate);   /* index in passes[], or -1 */
int passStatus(const char *plate); /* PASS_NONE / PASS_VALID / PASS_EXPIRED */
void registerPass(void);
void renewPass(void);
void cancelPass(void);
void listPasses(void);
int adminLogin(void); /* 1 if password correct, else 0 */
void adminMenu(void); /* report, pass options */
/* main() is also written by Person 3 */

#endif

//(PASSES.C)(MAIN.C)
#include <stdlib.h>
#include <ctype.h>
#include "parking.h"
 
#define SECONDS_PER_DAY(24L*60L*60L)
 
/* ----Small private helpers ---- */
 /* Writes a time_t as "DD-MM-YYYY" into buf (buf must hold at least 11 chars). */
static void dateText(time_t t, char *buf, int size)
{
    struct tm *info = localtime(&t);
    if (info == NULL) {
        strncpy(buf, "00-00-0000",size-1);
        buf[size-1] = '\0';
        return;
    }
    strftime(buf,size, "%d-%m-%Y", info);
}
 /* Price of a pass for the given vehicle type. */
static int passPrice(int type)
{
    return (type == TWO_WHEELER) ? PASS_PRICE_TWO : PASS_PRICE_FOUR;
}
 /* Name of the vehicle type, for printing. */
static const char *typeName(int type)
{
    return (type == TWO_WHEELER) ? "Two-wheeler" : "Four-wheeler";
}
 
/* Asks a yes/no question. Returns 1 for yes, 0 for no. */
static int confirm(const char *prompt)
{
    return readInt(prompt,0,1) == 1;
}
 
/* ----Pass lookup ----*/
 /* Returns the index in passes[] of the ACTIVE pass for this plate, or -1. */
int findPass(const char *plate)
{
    int i;
    for (i = 0; i < passCount; i++) {
        if (passes[i].active && strcmp(passes[i].plate, plate) == 0)
            return i;
    }
    return -1;
}
 /* PASS_NONE if there is no active pass, PASS_VALID if not yet expired,
   PASS_EXPIRED if the expiry date has passed. */
int passStatus(const char *plate)
{
    int idx = findPass(plate);
    if (idx == -1)
        return PASS_NONE;
 
    if (time(NULL) > passes[idx].expiryDate)
        return PASS_EXPIRED;
    return PASS_VALID;
}
 //----Pass operations ---- 
 Void registerPass(void)
{
    char name[NAME_LEN];
    char plate[PLATE_LEN];
    char expiryText[11];
    int  type, price, choice;
    time_t now;
    MonthlyPass *p;
 
    printf("\n--- Register Monthly Pass ---\n");
 
    if (passCount >= MAX_PASSES) {
        printf("Pass list full. No more passes can be registered.\n");
        return;
    }
    readString("Enter owner name: ", name, NAME_LEN);
    readString("Enter vehicle plate number: ", plate, PLATE_LEN);
    formatPlate(plate);
 
    if (plate[0] == '\0') {
        printf("Plate number cannot be empty.\n");
        return;
    }
    if (findPass(plate) != -1) {
        printf("Vehicle %s already has a pass. Please use Renew instead.\n", plate);
        return;
    }
 
    choice = readInt("Vehicle type (1 = two-wheeler, 2 = four-wheeler): ", 1, 2);
    type = (choice == 1) ? TWO_WHEELER : FOUR_WHEELER;
    price = passPrice(type);
 
    printf("Pass price for a %s: Rs. %d (valid for %d days)\n",
           typeName(type), price, PASS_DAYS);
    if (!confirm("Confirm payment and register? (1 = yes, 0 = no): ")) {
        printf("Registration cancelled.\n");
        return;
    }
    now = time(NULL);
    p = &passes[passCount];
    strncpy(p->plate, plate, PLATE_LEN - 1);
    p->plate[PLATE_LEN - 1] = '\0';
    strncpy(p->ownerName, name, NAME_LEN - 1);
    p->ownerName[NAME_LEN - 1] = '\0';
    p->type       = type;
    p->startDate  = now;
    p->expiryDate = now + (time_t)PASS_DAYS * SECONDS_PER_DAY;
    p->active     = 1;
    passCount++;
 
    today.passesSoldToday++;
    today.passAmount += price;
 
    dateText(p->expiryDate, expiryText, sizeof(expiryText));
    printf("Pass registered for %s (%s). Rs. %d received.\n", plate, name, price);
    printf("Pass is valid until %s.\n", expiryText);
}
void renewPass(void)
{
    char plate[PLATE_LEN];
    char expiryText[11];
    int  idx, price;
    time_t now, base;
 
    printf("\n--- Renew Monthly Pass ---\n");
    readString("Enter vehicle plate number: ", plate, PLATE_LEN);
    formatPlate(plate);
 
    idx = findPass(plate);
    if (idx == -1) {
        printf("No pass found for %s. Please register first.\n", plate);
        return;
    }
 
    price = passPrice(passes[idx].type);
    dateText(passes[idx].expiryDate, expiryText, sizeof(expiryText));
    printf("Owner: %s | Current expiry: %s (%s)\n",
           passes[idx].ownerName, expiryText,
           passStatus(plate) == PASS_VALID ? "VALID" : "EXPIRED");
    printf("Renewal price: Rs. %d for %d more days\n", price, PASS_DAYS);
 
    if (!confirm("Confirm payment and renew? (1 = yes, 0 = no): ")) {
        printf("Renewal cancelled.\n");
        return;
    }
 
    /* New expiry = later of (now, old expiry) + 30 days */
    now  = time(NULL);
    base = (passes[idx].expiryDate > now) ? passes[idx].expiryDate : now;
    passes[idx].expiryDate = base + (time_t)PASS_DAYS * SECONDS_PER_DAY;
 
    today.passAmount += price;
 
     dateText(passes[idx].expiryDate, expiryText, sizeof(expiryText));
    printf("Pass renewed. Rs. %d received. New expiry: %s.\n", price, expiryText);
}
 
void cancelPass(void)
{
    char plate[PLATE_LEN];
    int  idx;
 
    printf("\n--- Cancel Monthly Pass ---\n");
    readString("Enter vehicle plate number: ", plate, PLATE_LEN);
    formatPlate(plate);
 
    idx = findPass(plate);
    if (idx == -1) {
        printf("No active pass found for %s.\n", plate);
        return;
    }
 
    printf("Pass owner: %s. Cancelling gives NO refund.\n", passes[idx].ownerName);
    if (!confirm("Really cancel this pass? (1 = yes, 0 = no): ")) {
        printf("Cancellation aborted.\n");
        return;
    }
 
    passes[idx].active = 0;
    printf("Pass for %s has been cancelled.\n", plate);
}
 void listPasses(void)
{
    int i, shown = 0;
    char expiryText[11];
 
    printf("\n--- Active Monthly Passes ---\n");
    printf("%-20s %-12s %-14s %-11s %s\n",
           "Owner", "Plate", "Type", "Expiry", "Status");
    printf("-------------------------------------------------------------------\n");
 
    for (i = 0; i < passCount; i++) {
        if (!passes[i].active)
            continue;
 
        dateText(passes[i].expiryDate, expiryText, sizeof(expiryText));
        printf("%-20s %-12s %-14s %-11s %s\n",
               passes[i].ownerName,
               passes[i].plate,
               typeName(passes[i].type),
               expiryText,
               passStatus(passes[i].plate) == PASS_VALID ? "VALID" : "EXPIRED");
        shown++;
    }
 
    if (shown == 0)
        printf("No active passes.\n");
    else
        printf("Total active passes: %d\n", shown);
}
 
/* ----Admin ---- */
 
/* Returns 1 if the correct password is entered within 3 attempts, else 0. */
int adminLogin(void)
{
    char entered[50];
    int  attempt;
 
    for (attempt = 1; attempt <= 3; attempt++) {
        readString("Enter admin password: ", entered, sizeof(entered));
        if (strcmp(entered, ADMIN_PASSWORD) == 0) {
            printf("Login successful.\n");
            return 1;
        }
        printf("Wrong password (%d of 3 attempts used).\n", attempt);
    }
 
    printf("Too many wrong attempts. Access denied.\n");
    return 0;
}
 
void adminMenu(void)
{
    int choice;
 
    do {
        printf("\n===== ADMIN MENU =====\n");
        printf("1. Daily report\n");
        printf("2. Register pass\n");
        printf("3. Renew pass\n");
        printf("4. Cancel pass\n");
        printf("5. List passes\n");
        printf("0. Back to main menu\n");
 
        choice = readInt("Enter your choice: ", 0, 5);
 
        switch (choice) {
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
            case 0:
                break;
        }
    } while (choice != 0);
}


#include "parking.h"
 
/* --- Shared data (declared extern in parking.h) --- */
Slot        lot[TOTAL_SLOTS];
MonthlyPass passes[MAX_PASSES];
int         passCount = 0;
DailyRecord today;
time_t      entry_time;   /* declared extern in Person 1's header */
 
int main(void)
{
    int choice;
 
    /* Start-up: set up slots, load saved data, check the date */
    initSlots();
    loadData();              /* a missing file on the first run is fine */
    resetDailyIfNewDay();
 
    printf("=====================================\n");
    printf("   CAMPUS PARKING MANAGEMENT SYSTEM\n");
    printf("=====================================\n");
 
    do {
        /* Also handles the program running past midnight */
        resetDailyIfNewDay();
 
        printf("\n========== MAIN MENU ==========\n");
        printf("1. Vehicle entry\n");
        printf("2. Vehicle exit\n");
        printf("3. Search vehicle\n");
        printf("4. View slots\n");
        printf("5. Admin\n");
        printf("0. Quit\n");
 
        choice = readInt("Enter your choice: ", 0, 5);
 
        switch (choice) {
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
                if (adminLogin()) {
                    adminMenu();
                    saveData();
                }
                break;
            case 0:
                saveData();
                printf("\nData saved. Thank you for using the Campus Parking System. Goodbye!\n");
                break;
        }
    } while (choice != 0);
 
    return 0;
}
