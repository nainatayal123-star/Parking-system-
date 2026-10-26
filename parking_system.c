/*
 * parking.h - Campus Parking Management System
 * Shared header file. All three members include this file.
 * Do NOT change anything here without telling the whole group.
 *
 * Slot layout (array index = slot number - 1):
 *   Slot   1 - 150  : normal slots for two-wheelers
 *   Slot 151 - 250  : normal slots for four-wheelers
 *   Slot 251 - 300  : reserved slots for monthly pass holders (both types)
 */

#ifndef PARKING_H
#define PARKING_H

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ---------------- Slot counts ---------------- */
#define NORMAL_TWO_SLOTS    150
#define NORMAL_FOUR_SLOTS   100
#define RESERVED_SLOTS       50
#define TOTAL_SLOTS         300     /* 150 + 100 + 50 */

/* ---------------- Vehicle types ---------------- */
#define TWO_WHEELER   0
#define FOUR_WHEELER  1

/* ---------------- Slot categories ---------------- */
#define SLOT_NORMAL_TWO   0
#define SLOT_NORMAL_FOUR  1
#define SLOT_RESERVED     2

/* ---------------- Fees (in rupees) ---------------- */
#define RATE_TWO_PER_HOUR    20
#define RATE_FOUR_PER_HOUR   40
/* Price of the first 30 minutes. ASSUMED as half of the hourly rate.
   Change these two lines if your group decides a different price. */
#define RATE_TWO_HALF_HOUR   10
#define RATE_FOUR_HALF_HOUR  20

#define PASS_PRICE_TWO    500
#define PASS_PRICE_FOUR   800
#define PASS_DAYS          30

/* ---------------- Other settings ---------------- */
#define PLATE_LEN    15
#define NAME_LEN     30
#define MAX_PASSES  500
#define ADMIN_PASSWORD "change@me"   /* fixed in code; choose your own */

#define SLOT_FILE   "slots.txt"
#define PASS_FILE   "passes.txt"
#define DAILY_FILE  "daily.txt"

/* Results of passStatus() */
#define PASS_NONE     0
#define PASS_VALID    1
#define PASS_EXPIRED  2

/* Optional, for testing only (Person 2 uses it inside the fee code):
 * remove the comment marks below to make 1 real minute count as 1 hour.
 * Put the comment marks back before the final submission.
 */
/* #define TEST_MODE */

/* ---------------- Structures ---------------- */
typedef struct {
    char   plate[PLATE_LEN];   /* capital letters, no spaces */
    int    type;               /* TWO_WHEELER or FOUR_WHEELER */
    time_t entryTime;          /* real time of entry */
    int    isPassHolder;       /* 1 if a valid pass was found at entry */
} Vehicle;

typedef struct {
    int     slotNo;            /* 1 to 300 */
    int     category;          /* SLOT_NORMAL_TWO / SLOT_NORMAL_FOUR / SLOT_RESERVED */
    int     isOccupied;        /* 0 = free, 1 = occupied */
    Vehicle vehicle;           /* valid only when isOccupied is 1 */
} Slot;

typedef struct {
    char   plate[PLATE_LEN];
    char   ownerName[NAME_LEN];
    int    type;
    time_t startDate;
    time_t expiryDate;         /* startDate + PASS_DAYS days */
    int    active;             /* 0 = cancelled */
} MonthlyPass;

typedef struct {
    char date[11];             /* "DD-MM-YYYY" */
    int  vehiclesToday;        /* vehicles parked today */
    int  hourlyAmount;         /* rupees collected from hourly fees today */
    int  passesSoldToday;
    int  passAmount;           /* rupees collected from pass sales today */
} DailyRecord;

/* ---------------- Shared data (defined in main.c by Person 3) ---------------- */
extern Slot        lot[TOTAL_SLOTS];
extern MonthlyPass passes[MAX_PASSES];
extern int         passCount;
extern DailyRecord today;

/* =====================================================
 *  PERSON 1 : core logic and real time   (entry_exit.c)
 * ===================================================== */
int  findFreeSlot(int type, int isPassHolder); /* returns array index, or -1 if none free.
                                                  Pass holder: reserved first, then normal.
                                                  Others: normal slots of own type only. */
int  minutesParked(time_t entry, time_t exitTime);
int  calcFee(int type, int minutes);           /* <=30 min: half-hour rate;
                                                  otherwise rate * hours rounded up */
void vehicleEntry(void);                       /* full entry process */
void vehicleExit(void);                        /* full exit process, bill, free slot */
void searchVehicle(void);
void showDailyReport(void);                    /* vehicles, hourly amount, pass amount, total */

/* =====================================================
 *  PERSON 2 : foundation, helpers and storage   (slots.c, files.c)
 * ===================================================== */
void initSlots(void);                          /* sets slotNo, category, isOccupied = 0 */
int  findParkedVehicle(const char *plate);     /* returns array index of the slot, or -1 */
void formatPlate(char *plate);                 /* capital letters, remove spaces */
int  readInt(const char *prompt, int min, int max);
void readString(const char *prompt, char *dest, int size);
void viewSlots(void);                          /* all / free / reserved */
int  loadData(void);                           /* read slots, passes, daily record.
                                                  Must restore entryTime and isPassHolder. */
int  saveData(void);                           /* write all of them back */
void resetDailyIfNewDay(void);                 /* if date changed, start a new daily record */

/* =====================================================
 *  PERSON 3 : passes, admin and main menu   (passes.c, main.c)
 * ===================================================== */
int  findPass(const char *plate);              /* index in passes[], or -1 */
int  passStatus(const char *plate);            /* PASS_NONE / PASS_VALID / PASS_EXPIRED */
void registerPass(void);
void renewPass(void);
void cancelPass(void);
void listPasses(void);
int  adminLogin(void);                         /* 1 if password correct, else 0 */
void adminMenu(void);                          /* report, pass options */
/* main() is also written by Person 3 */

#endif
