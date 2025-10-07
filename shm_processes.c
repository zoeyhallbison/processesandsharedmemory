#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void DearOldDad(int []);
void PoorStudent(int []);

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Create shared memory for 2 integers: BankAccount and Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }
    printf("Server has received shared memory for BankAccount and Turn...\n");

    // Attach the shared memory
    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }
    printf("Server has attached the shared memory...\n");

    // Initialize the shared variables
    ShmPTR[0] = 0;  // BankAccount
    ShmPTR[1] = 0;  // Turn

    printf("Server is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    } else if (pid == 0) {
        // Child process
        PoorStudent(ShmPTR);
        exit(0);
    }

    // Parent process (Dear Old Dad)
    DearOldDad(ShmPTR);

    // Wait for child process to finish
    wait(&status);
    printf("Server has detected the completion of its child...\n");

    // Detach and remove the shared memory
    shmdt((void *) ShmPTR);
    printf("Server has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Server has removed its shared memory...\n");
    printf("Server exits...\n");

    return 0;
}

void DearOldDad(int ShmPTR[]) {
    int account;
    int i;  // Declare loop variable outside the for loop
    srand(time(NULL));  // Seed the random number generator

    for (i = 0; i < 25; i++) {
        sleep(rand() % 6);  // Sleep for a random time between 0 and 5 seconds

        account = ShmPTR[0];  // Copy BankAccount to local variable

        while (ShmPTR[1] != 0);

        if (account <= 100) {
            int balance = rand() % 101;  // Random amount between 0 and 100

            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        ShmPTR[0] = account;
        ShmPTR[1] = 1;
    }
}

void PoorStudent(int ShmPTR[]) {
    int account;
    int i;  // Declare loop variable outside the for loop
    srand(time(NULL) ^ (getpid() << 16));  // Different seed for child process

    for (i = 0; i < 25; i++) {
        sleep(rand() % 6);  // Sleep for a random time between 0 and 5 seconds

        account = ShmPTR[0];  // Copy BankAccount to local variable

        while (ShmPTR[1] != 1);

        int balance = rand() % 51;  // Random amount between 0 and 50
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        ShmPTR[0] = account;
        ShmPTR[1] = 0;
    }
}