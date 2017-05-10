#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if (argv[1] != NULL) {
        printf("The result is displayed for 30 seconds. Close the windows to exit before 30 seconds!\n");
        char *cmd[] = {"xterm", "-e", "./sort1", argv[1], NULL};
        execvp(cmd[0], cmd);
    } else {
        printf("Invalid args! \n");
        printf("Please check the readme to execute the files. \n");
    }

    exit(0);
}


