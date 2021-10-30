/*
 * @file main.c
 * @start_date 2021-09-28
 * @author_01 Belisa Lopes <2200724@my.ipleiria.pt>
 * @author_02 José P. Areia <2200655@my.ipleiria.pt>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>

#include "args.h"
#include "debug.h"
#include "memory.h"
#include "type.h"

#define ERR_CMD_LINE    1
#define ERR_SIGNAL      2
#define ERR_EXEC        3
#define ERR_FORK        4
#define ERR_OPEN_FILE   5
#define ERR_CLOSE_FILE  6
#define BUFFER_SIZE     2047

void open_output_file();
int check_text_file(char *filename);
void handle_signal(int signal, siginfo_t *siginfo, void *context);
void open_file_and_check_mime(char *filename, int *num_ok, int *num_mismatch);

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args;
    struct sigaction act;

    /* Declare and set all the variables used for statistic purposes at zero */
    int num_files = 0, num_errors = 0, num_ok = 0, num_mismatch = 0;
    int status;

    if(cmdline_parser(argc, argv, &args) != 0)
        ERROR(ERR_CMD_LINE, "cannot execute cmdline_parser\n");

    /* Signals */
    act.sa_sigaction = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags |= SA_SIGINFO;

    /* If the mode isn't -b/--batch the signal SIGUSR1 is ignored by the application */
    if (args.batch_arg == NULL) {
        if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)
            ERROR(ERR_SIGNAL, "Failed to execute the function signal");
    }

    if (sigaction(SIGQUIT, &act, NULL) < 0)
        ERROR(ERR_SIGNAL, "Failed to execute sigaction (SIGQUIT)");

    if (sigaction(SIGINT, &act, NULL) < 0)
        ERROR(ERR_SIGNAL, "Failed to execute sigaction (SIGINT)");

    if (args.signal_flag) {
        printf("The application is ready to receive the signals SIGINT, SIGQUIT and SIGUSR1.\nThe signal SIGUSR1 will only work for the batch '-b/--batch' mode.\n");
        printf("Use the following PID: %d to send a signal and proceed with the application.\n\n", getpid());
    }

    /* -f/--file option */
    if ((int) args.file_given > 0) {
        for (int i = 0; i < (int) args.file_given; i++) {
            /* Check if the file passed throught the command line exists or not. */
            if (access(args.file_arg[i], F_OK) == -1) {
                fprintf(stderr, "ERROR: cannot open file '%s' -- %s\n", args.file_arg[i], strerror(errno));
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    /* Opens the file that is going to receive the the information from std_out (execlp) */
                    open_output_file();
                    execlp("file", "file", "-b", "--mime-type", args.file_arg[i], NULL);
                    ERROR(ERR_EXEC, "Failed to execute execlp");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    waitpid(pid, &status, 0);
                    /* Check if there was an error while executing the execlp and proceed with a failure exit */
                    if (WIFEXITED(status) && WEXITSTATUS(status) == ERR_EXEC) {
                        exit(EXIT_FAILURE);
                    } else {
                        /*
                         * Open the output file with the mime files and check the mimes.
                         * More information avaiable down below (main@line:255).
                         */
                        open_file_and_check_mime(args.file_arg[i], &num_ok, &num_mismatch);
                    }
                } else {
                    ERROR(ERR_FORK, "Failed to execute fork");
                }
            }
        }
        if(args.signal_flag) pause();
        exit(EXIT_SUCCESS);
    }

    /* -b/--batch option */
    if (args.batch_arg != NULL) {
        /* Check if the file passed throught the command line exists or not */
        if (access(args.batch_arg, F_OK) == -1) {
            fprintf(stderr, "ERROR: cannot open file '%s' -- %s\n", args.batch_arg, strerror(errno));
        } else {
            /*
             * Check if the file passed throught the command line is a text
             * file or not, and proceeds in the most appropriated way.
             * More information avaiable at the file type.c@line:94.
             */
            int check_text = check_text_file(args.batch_arg);
            if (!check_text) {
                fprintf(stderr, "[ERROR] Cannot resolve '%s' -- Not a text file\n", args.batch_arg);
                exit(EXIT_FAILURE);
            } else {
                if (sigaction(SIGUSR1, &act, NULL) < 0)
                    ERROR(ERR_SIGNAL, "Failed to execute sigaction (SIGUSR1)");

                FILE *batch_file = fopen(args.batch_arg, "r");
                if (batch_file == NULL)
                    ERROR(ERR_OPEN_FILE, "Failed to open the file '%s'", args.batch_arg);

                if (args.signal_flag)
                    pause();

                printf("[INFO] analysing files listed in '%s'\n", args.batch_arg);

                char *file = MALLOC(BUFFER_SIZE * sizeof(char) + 1);
                while (fscanf(batch_file, "%s", file) != EOF) {
                    if(args.signal_flag)
                        printf("Processing number: %d/%s\n", num_files + 1, file);

                    if (access(file, F_OK) == -1) {
                        fprintf(stderr, "[ERROR] cannot open file '%s' -- %s\n", file, strerror(errno));
                        num_errors++;
                    } else {
                        pid_t pid = fork();
                        if (pid == 0) {
                            /* Opens the file that is going to receive the the information from std_out (execlp) */
                            open_output_file();
                            execlp("file", "file", "-b", "--mime-type", file, NULL);
                            ERROR(ERR_EXEC, "Failed to execute execlp");
                            exit(EXIT_FAILURE);
                        } else if (pid > 0) {
                            waitpid(pid, &status, 0);
                            /* Check if there was an error while executing the execlp and proceed with a failure exit */
                            if (WIFEXITED(status) && WEXITSTATUS(status) == ERR_EXEC) {
                                exit(EXIT_FAILURE);
                            } else {
                                /*
                                 * Open the output file with the mime files and check the mimes.
                                 * More information avaiable down below (main@line:255).
                                 */
                                open_file_and_check_mime(file, &num_ok, &num_mismatch);
                            }
                        } else {
                            ERROR(ERR_FORK, "Failed to execute fork");
                        }
                    }
                    num_files++;
                }
                fclose(batch_file);
                FREE(file);
                printf("[SUMMARY] files analysed:%d; files OK:%d; files MISMATCH:%d, errors:%d\n", num_files, num_ok, num_mismatch, num_errors);
                if (args.signal_flag) {
                    printf("\nPlease send a SIGQUIT/SIGINT in order to proceed with the application. (PID: %d)\n", getpid());
                    pause();
                }
                exit(EXIT_SUCCESS);
            }
        }
    }

    /* -d/--dir option */
    if (args.dir_arg != NULL){
        DIR *folder;
        struct dirent *dir;

        folder = opendir(args.dir_arg);
        if (folder == NULL) {
            fprintf(stderr, "ERROR: cannot open dir '%s' -- %s\n", args.dir_arg, strerror(errno));
            if (args.signal_flag) {
                printf("\nPlease send a SIGQUIT/SIGINT in order to proceed with the application. (PID: %d)\n", getpid());
                pause();
            }
        }

        while((dir=readdir(folder)) != NULL) {
            if(strcmp (dir->d_name, ".") == 0 || strcmp (dir->d_name, "..") == 0)
                continue;

            pid_t pid = fork();
            if (pid == 0) {
                strcat(args.dir_arg, "/");
                strcat(args.dir_arg, dir->d_name);
                /* Opens the file that is going to receive the the information from std_out (execlp) */
                open_output_file();
                execlp("file", "file", "-b", "--mime-type", args.dir_arg, NULL);
                ERROR(ERR_EXEC, "Failed to execute execlp");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                waitpid(pid, &status, 0);
                /* Check if there was an error while executing the execlp and proceed with a failure exit */
                if (WIFEXITED(status) && WEXITSTATUS(status) == ERR_EXEC) {
                    exit(EXIT_FAILURE);
                } else {
                    /*
                     * Open the output file with the mime files and check the mimes.
                     * More information avaiable down below (main@line:255).
                     */
                    open_file_and_check_mime(dir->d_name, &num_ok, &num_mismatch);
                }
            } else {
                ERROR(ERR_FORK, "Failed to execute fork");
            }
            num_files++;
        }
        closedir(folder);
        printf("[SUMMARY] files analysed:%d; files OK:%d; files MISMATCH:%d, errors:%d\n", num_files, num_ok, num_mismatch, num_errors);
        if (args.signal_flag) {
            printf("\nPlease send a SIGQUIT/SIGINT in order to proceed with the application. (PID: %d)\n", getpid());
            pause();
        }
        exit(EXIT_SUCCESS);
    }
	cmdline_parser_free(&args);
    return 0;
}

/* Functions that handles all the signal supported by the application */
void handle_signal(int signal, siginfo_t *siginfo, void *context) {
    (void) context;
    int aux;
    aux = errno;

    if (signal == SIGQUIT) {
        printf("Captured SIGQUIT signal (sent by PID: %ld). Use SIGINT to terminate application.\n", (long) siginfo->si_pid);
        pause();
    }

    if (signal == SIGINT) {
        printf("A SIGINT signal was captured (sent by PID: %ld). Terminating the application...\n", (long) siginfo->si_pid);
        exit(EXIT_SUCCESS);
    }

    if (signal == SIGUSR1) {
        /* Everything that is necessary to build a correct display of current time and date */
        int hours, minutes, seconds, day, month, year;
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);

        hours = local->tm_hour;
        minutes = local->tm_min;
        seconds = local->tm_sec;

        day = local->tm_mday;
        month = local->tm_mon + 1;
        year = local->tm_year + 1900;

        printf("Started process on %d.%02d.%02d_%02dh%02d:%02d\n\n", year, month, day, hours, minutes, seconds);
    }

    errno = aux;
}

/* Opens the file which will contain all the mimes (throught std_out execlp) */
void open_output_file() {
    int fd = open("output.txt", O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
    if (fd == -1)
        ERROR(ERR_OPEN_FILE, "Failed to open the file 'output.txt'");

    /* Write the std_out to the file 'output.txt' */
    dup2(fd, STDOUT_FILENO);

    if (close(fd) == -1)
        ERROR(ERR_CLOSE_FILE, "Failed to close the file 'output.txt'");
}

/* Opens the file which contains all the mimes and check if the mime is supported by the application */
void open_file_and_check_mime(char *filename, int *num_ok, int *num_mismatch) {
    char *mime = MALLOC(BUFFER_SIZE * sizeof(char) + 1);

    FILE *fd = fopen("output.txt", "r");
    if (fd == NULL)
        ERROR(ERR_OPEN_FILE, "cannot open the file 'output.txt'");

    /*
     * For every filename written in the file 'output.txt' the function
     * check_mime (present in the file 'type.c') is called in order to check
     * if the mime is supported or not.
     */
    while (fscanf(fd, "%s", mime) != EOF) {
        check_mime(mime, filename, &(*num_ok), &(*num_mismatch));
    }

    fclose(fd);
    FREE(mime);
}
