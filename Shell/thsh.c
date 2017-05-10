/* Done by: Deva and Chaitanyashareef (Minchu) */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

#define BUFFERSIZE 256
#define READ_END 0
#define WRITE_END 1

pid_t pid_global;

char previous_directory[];
int exit_status = 0;
int background_jobs[MAX_INPUT];
int bg_itr = 0;

void signal_handler(int);

//to remove /n /t from a string
void strip(char *s) {
    char *p2 = s;
    while (*s != '\0') {
        if (*s != '\t' && *s != '\n') {
            *p2++ = *s++;
        } else {
            ++s;
        }
    }
    *p2 = '\0';
}

//counts the number of arguments in the input command
int count_arguments(char string[], char delim) {
    int count = 1, i;
    for (i = 0; i < strlen(string); i++) {
        if (string[i] == delim)
            count++;
    }
    return count;
}

int get_redirection_arg_num(char *args[], char str) {
    int i;
    for (i = 0; i < strlen(*args); i++) {
        if (*args[i] == str) {
            break;
        }
    }
    return i;
}

// Built-in cd command: uses chdir()
int cd(char *pth) {

    int retVal;

    char path[BUFFERSIZE];
    char temp_dir[BUFFERSIZE];
    char current_working_dir[BUFFERSIZE];

    strcpy(path, pth);
    getcwd(current_working_dir, sizeof(current_working_dir));
    strcpy(temp_dir, current_working_dir);

    if (pth[0] != '/') {
        strcat(current_working_dir, "/");
        strcat(current_working_dir, path);
        retVal = chdir(current_working_dir);
        getcwd(current_working_dir, sizeof(current_working_dir));
    } else {
        retVal = chdir(pth);
    }
    if (retVal == 0) {
        strcpy(previous_directory, temp_dir);

    }
    return retVal;
}

int execute_pipe_command(char *arguments[]) {

    int fd1[2];
    int fd2[2];

    int i = 0;
    int number_of_commands = 1;
    while (arguments[i] != NULL) {
        if (strcmp(arguments[i], "|") == 0) {
            number_of_commands += 1;
        }
        i++;
    }

    int cmd_number = 0;
    i = 0;
    int end = 0;
    while (arguments[i] != NULL & end != 1) {
        char *pipe_cmd[MAX_INPUT];
        int j = 0;
        while (strcmp(arguments[i], "|") != 0) {
            pipe_cmd[j] = arguments[i];
            i++;
            if (arguments[i] == NULL) {
                end = 1;
                j++;
                break;
            }
            j++;
        }
        pipe_cmd[j] = NULL;
        i++;

        if (cmd_number % 2 != 0) {
            pipe(fd1);
        } else {
            pipe(fd2);
        }

        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Fork failed");
            return 1;
        }
        if (pid > 0) {

            if (cmd_number == 0) {
                close(fd2[WRITE_END]);
            } else if (cmd_number == number_of_commands - 1) {
                if (number_of_commands % 2 != 0) {
                    close(fd1[READ_END]);
                } else {
                    close(fd2[READ_END]);
                }
            } else {
                if (cmd_number % 2 != 0) {
                    close(fd2[READ_END]);
                    close(fd1[WRITE_END]);
                } else {
                    close(fd1[READ_END]);
                    close(fd2[WRITE_END]);
                }
            }
            waitpid(pid, NULL, 0);
            cmd_number++;

        } else {
            if (cmd_number == 0) { //for the first command
                dup2(fd2[WRITE_END], STDOUT_FILENO);
            } else if (cmd_number == number_of_commands - 1) { //for the last command
                if (number_of_commands % 2 != 0) {
                    dup2(fd1[READ_END], STDIN_FILENO);
                } else {
                    dup2(fd2[READ_END], STDIN_FILENO);
                }
            } else { //for commands that are not the first and last
                if (cmd_number % 2 != 0) {
                    dup2(fd2[READ_END], STDIN_FILENO);
                    dup2(fd1[WRITE_END], STDOUT_FILENO);
                } else {
                    dup2(fd1[READ_END], STDIN_FILENO);
                    dup2(fd2[WRITE_END], STDOUT_FILENO);
                }
            }
            execvp(pipe_cmd[0], pipe_cmd);
            printf("Invalid Command! \n");
        }

    }
}

int execute_command(char cmd[], int debug) {

    strip(cmd);

    char cmd_copy[MAX_INPUT];
    strcpy(cmd_copy, cmd);

    if (debug == 1) {
        char debug_running[MAX_INPUT] = "RUNNING: ";
        strcat(debug_running, cmd_copy);
        printf("%s \n", debug_running);
    }

    int num_of_args = count_arguments(cmd, ' ');

    char *arguments[num_of_args + 1];
    int itr = 0;
    char *p;
    p = strtok(cmd, " ");
    while (p != NULL) {
        arguments[itr] = p;
        p = strtok(NULL, " ");
        itr++;
    }
    arguments[num_of_args] = NULL;

    int background = 0;
    if (strcmp(arguments[num_of_args - 1], "&") == 0) {
        background = 1;
        arguments[num_of_args - 1] = NULL;
        num_of_args = num_of_args - 1;
    }

    int built_in_command = 0;

    // Execute the command, handling built-in commands separately

    //echo env variable

    if (strcmp(arguments[0], "echo") == 0) {
        char *token = strtok(arguments[1], "$");
        char *s = getenv(token);
        if (s != NULL) {
            built_in_command = 1;
            printf("%s \n", s);
            exit_status = 0;
        }
        if (strcmp(arguments[1], "$?") == 0) {
            built_in_command = 1;
            printf("%d \n", exit_status);
            exit_status = 0;
        }
        if (debug == 1 && built_in_command == 1) {
            char debug_ended[MAX_INPUT] = "ENDED: ";
            strcat(debug_ended, cmd_copy);
            printf("%s \n", debug_ended);
        }
    }
        //set env var

    else if (strcmp(arguments[0], "set") == 0) {
        built_in_command = 1;
        char *environ[2];
        environ[0] = strtok(arguments[1], "=");
        environ[1] = strtok(NULL, " ");
        setenv(environ[0], environ[1], 1);
        char *s = getenv(arguments[1]);
        printf("%s set to: %s \n", environ[0], s);
        exit_status = 0;

        if (debug == 1) {
            char debug_ended[MAX_INPUT] = "ENDED: ";
            strcat(debug_ended, cmd_copy);
            printf("%s \n", debug_ended);
        }

    }

        //cd

    else if (strcmp(arguments[0], "cd") == 0) {
        built_in_command = 1;
        if (num_of_args == 1) {
            int ret = cd(getenv("HOME"));
            exit_status = 0;
        }
        if (num_of_args > 2) {
            printf("Error: Too many arguments \n");
            exit_status = 1;
        } else {
            if (*arguments[1] == '-') {
                int ret = cd(previous_directory);
                exit_status = 0;
            } else {
                int ret = cd(arguments[1]);
                if (ret == -1) {
                    printf("Error: no such directory \n");
                    exit_status = 1;
                } else {
                    exit_status = 0;
                }
            }
        }
        if (debug == 1) {
            char debug_ended[MAX_INPUT] = "ENDED: ";
            strcat(debug_ended, cmd_copy);
            printf("%s \n", debug_ended);
        }
    }

        //jobs

    else if (strcmp(arguments[0], "jobs") == 0) {
        printf("--------------------------------- \n");
        printf("Printing all the background jobs\n");
        printf("--------------------------------- \n");
        for (int i = 0; i < bg_itr; i++) {
            char jobs_cmd[MAX_INPUT];
            strcpy(jobs_cmd, "ps -p ");
            char buffer[BUFFERSIZE];
            sprintf(buffer, "%d", background_jobs[i]);
            strcat(jobs_cmd, buffer);
            printf("Job number : %d \n", i);
            execute_command(jobs_cmd, debug);
            int job_status;
            waitpid(background_jobs[i], &job_status, 0);
            if (job_status != 0) {
                job_status = 1;
            }
            printf("Exit status: %d\n", job_status);
            printf("----------------------------- \n");
        }
        built_in_command = 1;
    }

        //exit

    else if (strcmp(arguments[0], "exit") == 0) {
        return -1;
    }

    //for those commands that are not built-in: fork() and exec()

    int pipeCommand = 0;
    for (int i = 0; i < num_of_args; i++) {
        if (strcmp(arguments[i], "|") == 0) {
            pipeCommand = 1;
        }
    }
    if (!built_in_command && pipeCommand) {
        execute_pipe_command(arguments);
        if (debug == 1) {
            char debug_ended[MAX_INPUT] = "ENDED: ";
            strcat(debug_ended, cmd_copy);
            printf("%s \n", debug_ended);
        }
    }


    if (!built_in_command && !pipeCommand) {


        int status;
        pid_t pid, w;

        pid = fork();
        pid_global = pid;

        if (pid < 0) {
            fprintf(stderr, "Fork failed");
            return 1;
        }
        if (pid > 0) {
            //parent
            do {
                signal(SIGINT, signal_handler);
                if (background == 0) {
                    w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
                    if (w == -1) {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    printf("[%d] \n", pid);
                    background_jobs[bg_itr] = pid;
                    bg_itr++;
                }

                if (WIFEXITED(status)) {
//                        printf("exited, status=%d\n", WEXITSTATUS(status));
                    exit_status = WEXITSTATUS(status);
                } else if (WIFSIGNALED(status)) {
//                        printf("killed by signal %d\n", WTERMSIG(status));
                    exit_status = WTERMSIG(status);
                } else if (WIFSTOPPED(status)) {
//                        printf("stopped by signal %d\n", WSTOPSIG(status));
                    exit_status = WSTOPSIG(status);
                } else if (WIFCONTINUED(status)) {
//                    printf("continued\n");
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));

            if (debug == 1) {
                char debug_ended[MAX_INPUT] = "ENDED: ";
                strcat(debug_ended, cmd_copy);
                printf("%s \n", debug_ended);
            }

        } else {

            //redirection '>'
            int redirection = -1;
            for (int i = 0; i < num_of_args; i++) {
                if (strcmp(arguments[i], "<") == 0) {
                    redirection = 1;
                    break;
                }
                if (strcmp(arguments[i], ">") == 0) {
                    redirection = 2;
                    break;
                }
            }
            if (redirection == 1) { //if '<' char is found in the input command
                int redirect_arg_num = get_redirection_arg_num(arguments, '<');
                int fd_in = open(arguments[redirect_arg_num + 1], O_RDONLY);
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
                char *new_args[redirect_arg_num + 1];
                for (int i = 0; i < redirect_arg_num; i++) {
                    new_args[i] = arguments[i];
                }
                new_args[redirect_arg_num] = NULL;
                execvp(new_args[0], new_args);
                exit_status = 1;
            }
            if (redirection == 2) { //if '>' char is found in the input command
                int redirect_arg_num = get_redirection_arg_num(arguments, '>');
                int fd_out = open(arguments[redirect_arg_num + 1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd_out, STDOUT_FILENO);
                dup2(fd_out, STDERR_FILENO);
                close(fd_out);
                char *new_args[redirect_arg_num + 1];
                for (int i = 0; i < redirect_arg_num; i++) {
                    new_args[i] = arguments[i];
                }
                new_args[redirect_arg_num] = NULL;
                execvp(new_args[0], new_args);
                exit_status = 1;
            }

            if (redirection == -1) { //if there is no redirection
                execvp(arguments[0], arguments);
                exit_status = 1;
                printf("Invalid Command! \n");
            }

        }

    }
    return 0;
}

int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int ends_with_sh(const char *str) { return ends_with(str, ".sh"); }

char *read_file(char *filename) {
    char *buffer = NULL;
    int string_size, read_size;
    FILE *handler = fopen(filename, "r");

    if (handler) {
        // Seek the last byte of the file
        fseek(handler, 0, SEEK_END);
        // Offset from the first to the last byte, or in other words, filesize
        string_size = ftell(handler);
        // go back to the start of the file
        rewind(handler);

        // Allocate a string that can hold it all
        buffer = (char *) malloc(sizeof(char) * (string_size + 1));

        // Read it all in one operation
        read_size = fread(buffer, sizeof(char), string_size, handler);

        // fread doesn't set it so put a \0 in the last position
        // and buffer is now officially a string
        buffer[string_size] = '\0';

        if (string_size != read_size) {
            // set the buffer to NULL
            free(buffer);
            buffer = NULL;
        }

        fclose(handler);
    }

    return buffer;
}

void check_and_run_shellscript(char shell_file[]) {
    if (ends_with_sh(shell_file) == 1) {
        char path[BUFFERSIZE];
        getcwd(path, BUFFERSIZE);
        strcat(path, "/");
        strcat(path, shell_file);
        char *string = read_file(path);
        int num_of_lines = count_arguments(string, '\n');
        char *cmds[num_of_lines];
        char *ptr;

        ptr = strtok(string, "\n");
        int iter = 0;
        while (ptr != NULL) {
            cmds[iter] = ptr;
            ptr = strtok(NULL, "\n");
            iter++;
        }

        for (int i = 0; i < num_of_lines; i++) {
            char first_char = cmds[i][0];
            if (!(first_char == '#')) {
                execute_command(cmds[i], 0);
                printf("---------------------------------- \n");
            }
        }
    }
}

int main(int argc, char **argv, char **envp) {


    int debug = 0;

    if (argv[1] != NULL) {
        if (strcmp(argv[1], "-d") == 0) {
            debug = 1;
        } else {
            check_and_run_shellscript(argv[1]);
        }
    }

    int finished = 0;
    char *prompt = "thsh> ";
    char cmd[MAX_INPUT];
    char final_prompt[BUFFERSIZE];

    char current_working_dir[BUFFERSIZE];


    while (!finished) {

        char *cursor;
        char last_char;
        int rv;
        int count;

        getcwd(current_working_dir, sizeof(current_working_dir));

        //Exercise 3: [cwd] thsh> as prompt
        strcpy(final_prompt, "[");
        strcat(final_prompt, current_working_dir);
        strcat(final_prompt, "]");
        strcat(final_prompt, " ");
        strcat(final_prompt, prompt);


        // Print the prompt
        rv = write(1, final_prompt, strlen(final_prompt));
        if (!rv) {
            finished = 1;
            break;
        }

        // read and parse the input
        for (rv = 1, count = 0, cursor = cmd, last_char = 1;
             rv && (++count < (MAX_INPUT - 1)) && (last_char != '\n'); cursor++) {

            rv = read(0, cursor, 1);
            last_char = *cursor;
        }
        *cursor = '\0';

        if (!rv) {
            finished = 1;
            break;
        }

        if (cmd[0] != '#') {
            int ret = execute_command(cmd, debug);
            if (ret == -1) {
                break;
            }
        }
    }


}

void signal_handler(int sig) {
    char c = NULL;
    signal(sig, SIG_IGN);
    printf("\nDo you really want to quit? [y/n] \n");
    c = getchar();
    if (c == 'y' || c == 'Y') {
        printf("great\n");
        kill(pid_global, SIGKILL);
    } else {
        signal(SIGINT, signal_handler);
    }
}