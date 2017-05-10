#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#define BUFFERSIZE 256
#define READ_END 0
#define WRITE_END 1

int usr_defined_sig_win = SIGUSR1; //SIGUSR1 is a user define signal for win
int usr_defined_sig_lose = SIGUSR2; //SIGUSR2 is a user defined signal for lose

//function to send signal to C
void signal_handler_C(int s) {
    if (s == usr_defined_sig_win) {
        printf("C: I won \n");
    } else if (s == usr_defined_sig_lose) {
        printf("C: I lost \n");
    }
    exit(0);
}

//fucntion to send signal to D
void signal_handler_D(int s) {
    if (s == usr_defined_sig_win) {
        printf("D: I won \n");
    } else if (s == usr_defined_sig_lose) {
        printf("D: I lost \n");
    }
    exit(0);
}

int main() {

    int fd_C[2], fd_D[2]; //for pipes
    char number_C[BUFFERSIZE];
    char number_D[BUFFERSIZE];

    /* create the pipes */

    if (pipe(fd_C) == -1) {
        fprintf(stderr, "Pipe for C failed \n");
        return 1;
    }
    if (pipe(fd_D) == -1) {
        fprintf(stderr, "Pipe for D failed \n");
        return 1;
    }

    pid_t pid_C = fork();
    pid_t pid_D = fork();


    //Child - C

    if (pid_C == 0) {

        //close D's pipe
        close(fd_D[READ_END]);
        close(fd_D[WRITE_END]);
        close(fd_C[READ_END]); //closing the read end of C

        //defining the signals to be sent to the child - C
        signal(usr_defined_sig_win, signal_handler_C);
        signal(usr_defined_sig_lose, signal_handler_C);

        while (1) {
            int random_number = rand() % 100;
            sprintf(number_C, "%d", random_number);
            write(fd_C[WRITE_END], number_C, BUFFERSIZE);
        }

    }

        //child - D
    else if (pid_D == 0) {
        close(fd_C[READ_END]);
        close(fd_C[WRITE_END]);
        close(fd_D[READ_END]); //closing the read end of D

        //defining the signals to be sent to the child - D
        signal(SIGUSR1, signal_handler_D);
        signal(SIGUSR2, signal_handler_D);

        while (1) {
            int random_number = rand() % 100;
            sprintf(number_D, "%d", random_number);
            write(fd_D[WRITE_END], number_D, BUFFERSIZE);
        }
    }

        //parent

    else {

        //close both the write ends
        close(fd_C[WRITE_END]);
        close(fd_D[WRITE_END]);

        int points_C = 0;
        int points_D = 0;
        int flag_choice;

        while (points_C != 10 && points_D != 10) {
            flag_choice = rand() % 2; //0 is Minimum and 1 is Maximum
            if (flag_choice) {
                printf("The flag choice for the round is: Maximum \n");
            } else {
                printf("The flag choice for the round is: Minimum \n");
            }

            read(fd_C[READ_END], number_C, BUFFERSIZE);
            int num_C = atoi(number_C);
            printf("Number from C: %d \n", num_C);

            read(fd_D[READ_END], number_D, BUFFERSIZE);
            int num_D = atoi(number_D);
            printf("Number from D: %d \n", num_D);


            if (flag_choice) { //if flag choice is 1: Maximum
                if (num_C > num_D) {
                    points_C += 1;
                    printf("C wins this round! \n");
                } else if (num_D > num_C) {
                    points_D += 1;
                    printf("D wins this round! \n");
                } else {
                    printf("Round not considered as both the numbers are equal \n");
                }
            } else { //if flag choice is 0: Minimum
                if (num_C < num_D) {
                    points_C += 1;
                    printf("C wins this round! \n");
                } else if (num_D < num_C) {
                    points_D += 1;
                    printf("D wins this round! \n");
                } else {
                    printf("Round not considered as both the numbers are equal \n");
                }
            }

            printf("Current score of C: %d \n", points_C);
            printf("Current score of D: %d \n", points_D);

            printf("----------------------------- \n");
        }

        if (points_C == 10) {
            printf("Parent: C WON \n");
            kill(pid_C, usr_defined_sig_win);
            kill(pid_D, usr_defined_sig_lose);
        } else {
            printf("Parent: D WON \n");
            kill(pid_C, usr_defined_sig_lose);
            kill(pid_D, usr_defined_sig_win);
        }
    }
}