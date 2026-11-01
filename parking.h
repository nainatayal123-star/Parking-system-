
#ifndef PARKING_H
#define PARKING_H

#include <stdio.h>
#include <string.h>
#include <time.h>

/* Slot counts */
#define NORMAL_TWO_SLOTS 150
#define NORMAL_FOUR_SLOTS 100
#define RESERVED_SLOTS 50
#define TOTAL_SLOTS 300

/* Vehicle types */
#define TWO_WHEELER 0
#define FOUR_WHEELER 1

/* Slot categories */
#define SLOT_NORMAL_TWO 0
#define SLOT_NORMAL_FOUR 1
#define SLOT_RESERVED 2

/* Parking fees */
#define RATE_TWO_PER_HOUR 20
#define RATE_FOUR_PER_HOUR 40
#define RATE_TWO_HALF_HOUR 10
#define RATE_FOUR_HALF_HOUR 20

/* Monthly pass prices */
#define PASS_PRICE_TWO 500
#define PASS_PRICE_FOUR 800
#define PASS_DAYS 30

/* Other settings */
#define PLATE_LEN 15
#define NAME_LEN 30
#define MAX_PASSES 500
#define ADMIN_PASSWORD "change@me"

#define SLOT_FILE "slots.txt"
#define PASS_FILE "passes.txt"
#define DAILY_FILE "daily.txt"

/* Pass status */
#define PASS_NONE 0
#define PASS_VALID 1
#define PASS_EXPIRED 2

/* Structures */
typedef struct
{
    char plate[PLATE_LEN];
    int type;
    time_t entryTime;
    int isPassHolder;
} Vehicle;

typedef struct
{
    int slotNo;
    int category;
    int isOccupied;
    Vehicle vehicle;
} Slot;

typedef struct
{
    char plate[PLATE_LEN];
    char ownerName[NAME_LEN];
    int type;
    time_t startDate;
    time_t expiryDate;
    int active;
} MonthlyPass;

typedef struct
{
    char date[11];
    int vehiclesToday;
    int hourlyAmount;
    int passesSoldToday;
    int passAmount;
} DailyRecord;

/* Shared variables */
extern Slot lot[TOTAL_SLOTS];
extern MonthlyPass passes[MAX_PASSES];
extern int passCount;
extern DailyRecord today;

/* Person 1 functions */
int findFreeSlot(int type, int isPassHolder);
int minutesParked(time_t entry, time_t exitTime);
int calcFee(int type, int minutes);
void vehicleEntry(void);
void vehicleExit(void);
void searchVehicle(void);
void showDailyReport(void);

/* Person 2 functions */
void initSlots(void);
int findParkedVehicle(const char *plate);
void formatPlate(char *plate);
int readInt(const char *prompt, int min, int max);
void readString(const char *prompt, char *dest, int size);
void viewSlots(void);
int loadData(void);
int saveData(void);
void resetDailyIfNewDay(void);

/* Person 3 functions */
int findPass(const char *plate);
int passStatus(const char *plate);
void registerPass(void);
void renewPass(void);
void cancelPass(void);
void listPasses(void);
int adminLogin(void);
void adminMenu(void);

#endif
