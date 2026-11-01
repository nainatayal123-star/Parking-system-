v/*
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
    strcpy(lot[idx].vehicle.plate, plate_number);
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

    if (lot[parked].vehicle.isPassHolder == 1)
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


  void initSlots(void)
{
     for(int i=0;i<TOTAL_SLOTS;i++)
     {
       lot.slotNo[i]+=1;
       if(lot.slotNo<150)
       strcpy (lot.category[i],"TWO WHEELER");
       else if(lot.slotNo[i]>150 && lot.slotNo[i]<250)
      strcpy (lot.category[i]," FOUR WHEELER");
       else
       strcpy(lot.category[i],"RESERVED");

       lot.isOccupied=0;
       lot.isplate[0]='\0';
        
     }

}
int findParkedVehicle(const char *plate)
    { 
      for(int i=0;i<TOTAL_SLOTS;i++)
      {
        if(lot.isOccupied && strcmp(lot[i].vehicle.plate , plate )==0)
        {
            return i;
        }
    }
    return -1;
}

void formatPlate(char *plate)
{
    if(plate==NULL)
    return ;

    int read_index=0;
    int write_index=0;

    while (plate[read_index]!=\0)
    {
        unsigned char c = plate[read_index];
        if(!isspace(c))
        plate[write_index++]= (char)toupper(c);
    read_index++;
    } 
    plate[write_index]=\0;
}
            
int readInt(const char *prompt, int min, int max)
{ 
    int value;

    while(1)
    {
        printf("%s",prompt);
        if(scanf("%d",&value)==1)
        {
            if( value>=min && value<= max)
            {
                while(getchar() !='\n');
                return value;
            }
        }
        else{
            int ch;
            while((ch=getchar())!='\n' && ch !=EOF);
        }
        printf("Invalid Input. Please enter a number between %d and %d .\n",min,max);
        }
    }




void readString(const char *prompt, char *dest, int size)
{
    while(1)
    {
        printf("%s",prompt);
        if(fgets(dest,size,stdin)!=NULL)
        {
            int i=0;
            while(dest[i]!=='\0')
            {
                if(dest[i]!=='\n')
                dest[i]==0;
                break;
            }
            i++;
        }
        if(dest[0]!='\0')
        break;
    }
    printf("Input cannot be empty. Please try again.\n");
}
void viewSlots(void)   
    {
        #include <stdio.h>

void viewSlots(void)
{
    int freeTwo = 0, freeFour = 0, freeReserved = 0;

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (!lot[i].isOccupied) {
            if (lot[i].category == SLOT_NORMAL_TWO) freeTwo++;
            else if (lot[i].category == SLOT_NORMAL_FOUR) freeFour++;
            else if (lot[i].category == SLOT_RESERVED) freeReserved++;
        }
    }
    printf("\n--- SLOT SUMMARY ---\n");
    printf("Free Two-Wheeler Slots  : %d / %d\n", freeTwo, NORMAL_TWO_SLOTS);
    printf("Free Four-Wheeler Slots : %d / %d\n", freeFour, NORMAL_FOUR_SLOTS);
    printf("Free Reserved Slots     : %d / %d\n", freeReserved, RESERVED_SLOTS);
    printf("---------------------\n");

    // 3. Ask user choice using readInt helper
    printf("\nView Options:\n");
    printf("1. View All Slots\n");
    printf("2. View Free Slots Only\n");
    printf("3. View Reserved Slots Only\n");
    int choice = readInt("Enter choice (1-3): ", 1, 3);

    
    printf("\nSlot No\tCategory\t\tStatus\t\tPlate\n");
    printf("----------------------------------------------------------\n");

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        int show = 0;

        if (choice == 1) {
            show = 1; 
        } else if (choice == 2 && !lot[i].isOccupied) {
            show = 1; 
        } else if (choice == 3 && lot[i].category == SLOT_RESERVED) {
            show = 1; 
        }

        if (show)
         {
            const char *catStr = "Normal 2W";
            if (lot[i].category == SLOT_NORMAL_FOUR) catStr = "Normal 4W";
            else if (lot[i].category == SLOT_RESERVED) catStr = "Reserved ";

            if (lot[i].isOccupied) {
                printf("%-7d\t%-15s\tOccupied\t%s\n", lot[i].slotNo, catStr, lot[i].vehicle.plate);
            } else {
                printf("%-7d\t%-15s\tFree\t\t-\n", lot[i].slotNo, catStr);
            }
        }
    }
}

    }

    int loadData()
     {

    FILE *fp = fopen("parking_data.txt", "r");
    

    if (fp == NULL) {
        return 0;
    }
    
    int i;
    for (i = 0; i < TOTAL_SLOTS; i++) {
        
        fscanf(fp, "%d %d %d %s", &lot[i].slotNo, 
                                 (int*)&lot[i].category, 
                                 &lot[i].isOccupied, 
                                 lot[i].vehicle.plate);
    }
    
    fclose(fp);
    return 1;
}
                                  

    int saveData()
     {
    FILE *fp = fopen("parking_data.txt", "w");
    
    
    if (fp == NULL) {
        return 0; 
    }
    
    int i;
    for (i = 0; i < TOTAL_SLOTS; i++)
     {

        fprintf(fp, "%d %d %d %s\n", lot[i].slotNo, 
                                     lot[i].category, 
                                     lot[i].isOccupied, 
                                     lot[i].vehicle.plate);
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
        last_day = current_time->tm_mday;
        return;
    }
    
    if (current_time->tm_mday != last_day) {

        printf("\n[Info] New day detected. Resetting daily statistics...\n");
        

        last_day = current_time->tm_mday;
    }
}

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

//****************(passes.c)*******************

int findPass(const char *plate)
{
int i;
for (i = 0; i < passCount; i++)
{
   if (passes[i].active == 1 &&
   strcmp(passes[i].plate, plate) == 0)
  {
    return i;
   }
}
    return -1;
}
int passStatus(const char *plate)           /*check whether a pass is valid or not */
{
    int index;
    time_t currentTime;
    index = findPass(plate);
    if (index == -1)
    {
       return PASS_NONE;
    }
    currentTime = time(NULL);

    if (currentTime <= passes[index].expiryDate)
    {
        return PASS_VALID;
    }
    else
    {
        return PASS_EXPIRED;
    }
}
void registerPass(void)      /*for a new month pass (registration)*/
{
   char plate[PLATE_LEN];
   char name[NAME_LEN];
   int typeChoice,type,price,index,confirm;
   time_t currentTime;
   printf("\n***** REGISTER MONTHLY PASS *****\n");

    if (passCount >= MAX_PASSES)
    {
        printf("Pass list is full \n");
        return;
    }

    readString("Enter owner's name: ", name, NAME_LEN);
    readString("Enter vehicle number: ", plate, PLATE_LEN);
    formatPlate(plate);

    if (strlen(plate)==0)
    {
        printf("Vehicle number cannot be empty \n");
        return;
    }
    index = findPass(plate);
    if (index!=-1)
    {
        printf("This vehicle already has a pass\n");
        printf("Please... renew the existing pass\n");
        return;
    }

    printf("Select vehicle type:\n");
    printf("1. Two Wheeler\n");
    printf("2. Four Wheeler\n");

    typeChoice = readInt("Enter your choice of vehicle:",1,2);
    if (typeChoice == 1)
    {
       type=TWO_WHEELER;
       price=PASS_PRICE_TWO;
    }
    else
    {
       type=FOUR_WHEELER;
       price=PASS_PRICE_FOUR;
    }

    printf("pass price: Rs. %d\n", price);
    printf("pass validity: %d days\n", PASS_DAYS);

    confirm = readInt("Confirm payment? (1=Yes,0=No):",0,1);
    if (confirm == 0)
    {
        printf("Pass registration is cancelled\n");
        return;
    }

    currentTime=time(NULL);
    strcpy(passes[passCount].plate,plate);
    strcpy(passes[passCount].ownerName,name);

    passes[passCount].type=type;
    passes[passCount].startDate=currentTime;
    passes[passCount].expiryDate =currentTime+PASS_DAYS*24*60*60;

    passes[passCount].active=1;
    passCount++;

    today.passesSoldToday++;
    today.passAmount=today.passAmount+price;
    printf("Pass registered successfully!!\n");
    printf("Vehicle Number: %s\n",plate);
    printf("Expiry Date: %s",ctime(&passes[passCount-1].expiryDate));
}
void renewPass(void)                   /*renew of the existing pass*/
{
    char plate[PLATE_LEN];
    int index,price;
    time_t currentTime;
    time_t newStartDate;

    printf("*****RENEW MONTHLY PASS******\n");
    readString("Enter the vehicle number:",plate,PLATE_LEN);
    formatPlate(plate);

    if (strlen(plate)==0)
    {
        printf("\nVehicle number cannot be empty\n");
        return;
    }
    index=findPass(plate);

    if (index==-1)
    {
        printf("Pass not found....Please register yourself first\n");
        return;
    }
    if (passes[index].type==TWO_WHEELER)
    {
        price=PASS_PRICE_TWO;
    }
    else
    {
        price=PASS_PRICE_FOUR;
    }
    currentTime=time(NULL);
    if (passes[index].expiryDate>currentTime)
    {
        newStartDate = passes[index].expiryDate;
    }
    else
    {
        newStartDate = currentTime;
    }

    passes[index].startDate=newStartDate;
    passes[index].expiryDate=newStartDate+PASS_DAYS*24*60*60;

    today.passesSoldToday++;
    today.passAmount=today.passAmount+price;

    printf("Pass renewed successfully\n");
    printf("Renewal amount: Rs. %d\n",price);
    printf("New expiry date: %s",ctime(&passes[index].expiryDate));
}

void cancelPass(void)
{
    char plate[PLATE_LEN];
    int index,confirm;

    printf("******CANCEL MONTHLY PASS*******\n");

    readString("Enter vehicle number: ",plate,PLATE_LEN);
    formatPlate(plate);

    if (strlen(plate)==0)
    {
        printf("\nVehicle number cannot be empty\n");
        return;
    }

    index=findPass(plate);

    if (index==-1)
    {
        printf("Pass is not found\n");
        return;
    }

    confirm=readInt("Are you sure you want to cancel? (1=Yes,0=No):",0,1);
    if (confirm==0)
    {
        printf("Pass cancellation stopped\n");
        return;
    }
    passes[index].active=0;

    printf("Pass cancelled successfully\n");
    printf("No refund will be given\n");
}

void listPasses(void)             /*displaying of all active montly pass*/
{
    int i,count=0;

    printf("*******MONTHLY PASSES********\n");
    for(i=0;i<passCount;i++)
    {
        if (passes[i].active==1)
        {
            count++;

            printf("Owner name:%s\n",passes[i].ownerName);
            printf("Vehicle number:%s\n",passes[i].plate);

            if (passes[i].type==TWO_WHEELER)
            {
                printf("Vehicle Type:Two wheeler\n");
            }
            else
            {
                printf("Vehicle type:Four wheeler\n");
            }
            printf("Expiry Date:%s",ctime(&passes[i].expiryDate));

            if (passStatus(passes[i].plate)==PASS_VALID)
            {
                printf("Pass Status:VALID\n");
            }
            else
            {
                printf("Pass Status:EXPIRED\n");
            }
        }
    }

    if (count==0)
    {
        printf("No active passes found\n");
    }
}
int adminLogin(void)
{
    char password[50];
    int attempts;
    printf("********ADMIN LOGIN*********\n");
    for(attempts=1;attempts<=3;attempts++)
    {
        readString("Enter admin password:",password,50);

        if(strcmp(password,ADMIN_PASSWORD)==0)
        {
            printf("login successful\n");
            return 1;
        }
        else
        {
            printf("incorrect password\n");
        }
    }
    printf("three incorrect attempts....denied\n");
    return 0;
}

void adminMenu(void)
{
    int choice;
    do
    {
        printf("*********ADMIN MENU********\n");
        printf("1.Daily report\n");
        printf("2.Register monthly pass\n");
        printf("3.Renew monthly pass\n");
        printf("4.Cancel monthly pass\n");
        printf("5.List of monthly passes\n");
        printf("0.Back to Main Menu\n");

        choice=readInt("Enter your choice:",0,5);
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

            case 0:
                printf("back to the main menu\n");
                break;
        }
    } while (choice!=0);
}

int main(void)
{
    int choice;

    initSlots();
    loadData();
    resetDailyIfNewDay();

    printf("\n====================================\n");
    printf(" CAMPUS PARKING MANAGEMENT SYSTEM\n");
    printf("====================================\n");
    do
    {
        resetDailyIfNewDay();
        printf("-------- MAIN MENU ---------\n");
        printf("1. Vehicle Entry\n");
        printf("2. Vehicle Exit\n");
        printf("3. Search Vehicle\n");
        printf("4. View Parking Slots\n");
        printf("5. Admin\n");
        printf("0. Quit\n");

        choice = readInt("Enter your choice:",0,5);
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
                if (adminLogin()==1)
                {
                    adminMenu();
                    saveData();
                }
                break;

            case 0:
                saveData();
                printf("Data is saved successfully\n");
                printf("Thank you !!\n");
                break;
        }
    } while (choice!=0);
    return 0;
}

