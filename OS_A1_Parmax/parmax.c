#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int getMaximum(int array[], int start, int end) {

    int num_of_integers = end - start + 1;

    // if number of elements in the array < 10, then get the max directly
    if (num_of_integers < 10) {
        int max = array[start];
        for (int i = start; i <= end; i++) {
            if (array[i] > max) {
                max = array[i];
            }
        }
        exit(max); //returning max via exit
    } else { //if number of elements in the array >= 10, split into two arrays and find max

        pid_t pidL, pidR;

        pidL = fork();
        int statusL;

        pidR = fork();
        int statusR;

        int mid = (start + end) / 2;

        if (pidL == 0) { //left child
            getMaximum(array, start, mid);
        }
        if (pidR == 0) { //right child
            getMaximum(array, mid + 1, end);
        } else { //parent: gets maximum from left child and right child and computes which of them is greater
            waitpid(pidL, &statusL, 0);
            int maxL = statusL >> 8;
            waitpid(pidR, &statusR, 0);
            int maxR = statusR >> 8;

            //check which one of the max: maxL or maxR is greater
            int max;
            if (maxL > maxR) {
                max = maxL;
            } else {
                max = maxR;
            }
            exit(max);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {

    int range = 127;

    int n = atoi(argv[1]);
    int array[n];

    for (int i = 0; i < n; i++) {
        array[i] = rand() % range;
    }

    printf("The initial array is: \n");
    for (int i = 0; i < n; i++) {
        printf("%d", array[i]);
        if (i != n - 1) {
            printf(",");
        }
    }
    printf("\n");

    pid_t pid;
    int status;
    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed");
        return 1;
    }
    if (pid > 0) {
        //parent
        waitpid(pid, &status, 0); //waiting for the child to complete and getting the max via exit status
        //the exit returns a 16-bit value where the high 8 bits are the exit status. Hence we need to right shift by 8 bits
        int max = status >> 8;
        int PID = getpid();
        int PPID = getppid();
        printf("PPID is: %d \n", PPID);
        printf("PID is: %d \n", PID);
        printf("The maximum value is: %d \n", max);
    } else {
        //child
        getMaximum(array, 0, n - 1);
    }
    sleep(30);
    return 0;
}