/*
 * @file main.c
 * @date 2021-09-28
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

void handle_signal(int signal, siginfo_t * siginfo, void *context);

void handle_signal(int signal, siginfo_t * siginfo, void *context) {
    (void) context;
    int aux;
    aux = errno;

    if (signal == SIGQUIT) {
        printf("Captured SIGQUIT signal (sent by PID: %ld). Use SIGINT to terminate application.\n", (long) siginfo->si_pid);
        pause();
    }

    if (signal == SIGINT) {
        exit(EXIT_SUCCESS);
    }

    if (signal == SIGUSR1) {
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

        printf("Started process on %d.%02d.%02d_%02dh%02d:%02d\n", year, month, day, hours, minutes, seconds);
    }

    errno = aux;
}

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args;
    struct sigaction act;

    /* Declare and set all the variables used for statistic purposes at zero */
    int num_files = 0, num_errors = 0, num_ok = 0, num_mismatch = 0;

    if(cmdline_parser(argc, argv, &args) != 0) {
        ERROR(1, "Can't resolve cmdline_parser\n");
    }

    act.sa_sigaction = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags |= SA_SIGINFO;

    if (args.batch_arg != NULL) {
        if (sigaction(SIGUSR1, &act, NULL) < 0) {
            ERROR(1, "ERROR in sigaction - SIGUSR1");
        }
    } else {
        signal(SIGUSR1, SIG_IGN);
    }

    if (sigaction(SIGQUIT, &act, NULL) < 0) {
        ERROR(1, "ERROR in sigaction - SIGQUIT");
    }

    if (sigaction(SIGINT, &act, NULL) < 0) {
        ERROR(1, "ERROR in sigaction - SIGINT");
    }

    printf("The program is ready to receive the signals SIGQUIT and SIGUSR1 (just for batch '-b/--batch' mode).\nProgram PID: %d\n", getpid());

    /* -f/--file option */
    if ((int) args.file_given > 0) {
        for (int i = 0; i < (int) args.file_given; ++i) {
            /* Check if the file passed throught the command line exists or not. */
            if (access(args.file_arg[i], F_OK) == -1) {
                fprintf(stderr, "ERROR: cannot open file ''%s' -- %s\n", args.file_arg[i], strerror(errno));
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    int fd = open("output.txt", O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execlp("file", "file", "-E", "-b", "--mime-type", args.file_arg[i], NULL);
                    perror("execl");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    wait(NULL);

                    FILE *fp = fopen("output.txt", "r");
                    if (fp == NULL) {
                       exit(EXIT_FAILURE);
                    }

                    char *mime = MALLOC(sizeof(char) + 1);
                    fscanf(fp, "%s", mime);
                    check_mime(mime, args.file_arg[i], &num_ok, &num_mismatch);
                    free(mime);
                    fclose(fp);
                } else {
                    ERROR(1, "Can't execute fork()");
                }
            }
        }
        pause();
        exit(EXIT_SUCCESS);
    }

    /* -b/--batch option */
    if (args.batch_arg != NULL) {
        if (access(args.batch_arg, F_OK) == 0) {
            FILE * file_input = fopen(args.batch_arg, "r");
            if (file_input == NULL) {
               exit(EXIT_FAILURE);
            }
            pause();
            printf("[INFO] analysing files listed in '%s'\n", args.batch_arg);
            char *files = MALLOC(sizeof(char) + 1);
            while (fscanf(file_input, "%s", files) != EOF) {
                printf("Processing number: %d/%s\n", num_files+1, files);
                if (access(files, F_OK) == -1) {
                    fprintf(stderr, "[ERROR] cannot open file '%s' -- %s\n", files, strerror(errno));
                    num_errors++;
                } else {
                    pid_t pid = fork();
                    if (pid == 0) {
                        int file_input = open("output.txt", O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
                        dup2(file_input, STDOUT_FILENO);
                        close(file_input);
                        execlp("file", "file", "-E", "-b", "--mime-type", files, NULL);
                        perror("execl");
                        exit(EXIT_FAILURE);
                    } else if (pid > 0) {
                        wait(NULL);
                        FILE *file_output = fopen("output.txt", "r");
                        if (file_output == NULL) {
                           exit(EXIT_FAILURE);
                        }
                        char *mime = MALLOC(sizeof(char)+1);
                        while (fscanf(file_output, "%s", mime) != EOF) {
                            check_mime(mime, files, &num_ok, &num_mismatch);
                        }
                        free(mime);
                        fclose(file_output);
                    } else {
                        ERROR(1, "Can't execute fork");
                    }
                }
                num_files++;
            }
            printf("[SUMMARY] files analysed:%d; files OK:%d; files MISMATCH:%d, errors:%d\n", num_files, num_ok, num_mismatch, num_errors);
            exit(EXIT_SUCCESS);
       }
       else {
           fprintf(stderr, "ERROR: cannot open file '%s' -- %s\n", args.batch_arg, strerror(errno));
       }
    }

    /* -d/--dir option */
    if (args.dir_arg != NULL){
        DIR *folder;
        struct dirent *dir;
        folder = opendir(args.dir_arg); //opens directory passed in argument

        if (folder == NULL) {
            fprintf(stderr, "ERROR: cannot open dir '%s' -- %s\n", args.dir_arg, strerror(errno));
            exit(EXIT_FAILURE);
        }

        while((dir=readdir(folder)) != NULL) { //read all files of the directory
            if(strcmp (dir->d_name, ".") == 0 || strcmp (dir->d_name, "..") == 0) {
                continue;
            }
            pid_t pid = fork();
            if (pid == 0) {
                strcat(args.dir_arg, dir->d_name);
                int fdin = open("output.txt", O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU); //create a file to be written stdin and result from execlp
                dup2(fdin, STDOUT_FILENO);
                close(fdin);
                execlp("file", "file", "-E", "-b", "--mime-type", args.dir_arg, NULL); //exec to tell what type of file is
                perror("execlp");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                wait(NULL);
                FILE *fdout = fopen("output.txt", "r"); //open output file
                char *mimes = MALLOC(sizeof(char)+1);
                fscanf(fdout, "%s", mimes); //write to mimes
                check_mime(mimes, dir->d_name, &num_ok, &num_mismatch); //function that checks and compares every file extension(in external file named type.c)
                fclose(fdout);
                free(mimes);
            } else {
                ERROR(1, "Can't execute fork()");
            }
            num_files++;
        }
        closedir(folder);
        printf("[SUMMARY] files analysed:%d; files OK:%d; files MISMATCH:%d, errors:%d\n", num_files, num_ok, num_mismatch, num_errors);
        pause();
        exit(EXIT_SUCCESS);
    }

	cmdline_parser_free(&args);
    return 0;
}
