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

#include "args.h"
#include "debug.h"
#include "memory.h"
#include "type.h"

#define LSIZ 128
#define RSIZ 10

struct gengetopt_args_info args;

int main(int argc, char *argv[]) {
    if(cmdline_parser(argc, argv, &args) != 0) {
        ERROR(1, "ERROR: error in cmdline_parser\n");
    }

    if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0) {
        for (int i = 0; i < (int) args.file_given; ++i) {
            /* Check if the file passed throught the command line exists or not. */
            if (access(args.file_arg[i], F_OK) == -1) {
                fprintf(stderr, "ERROR: cannot open file <%s> -- %s\n", args.file_arg[i], strerror(errno));
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    int fd = open("output.txt", O_TRUNC | O_WRONLY | O_CREAT, S_IRWXU);
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                    execlp("file", "file", "-E", "-b", "--mime-type", args.file_arg[i], NULL);
                    perror("execl");
                    exit(1);
                } else if (pid > 0) {
                    wait(NULL);
                    FILE *fp = fopen("output.txt", "r");
                    if (fp == NULL) {
                       exit(EXIT_FAILURE);
                    }
                    char *mime = malloc(sizeof(char)+1);
                    fscanf(fp, "%s", mime);
                    check_mime(mime, args.file_arg[i]);
                    free(mime);
                    exit(0);
                } else {
                    ERROR(1, "ERROR: cannot execute fork()");
                }
            }
        }
    }

    if (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--batch") == 0) {
        if (access(args.batch_arg, F_OK) == 0) {
            char line[RSIZ][LSIZ];
            FILE * fp;
            int count_lines = 0;
            int tot = 0;
            int i = 0;

            fp = fopen(args.batch_arg, "r");
            if (fp == NULL) {
               exit(EXIT_FAILURE);
            }

            printf("[INFO] analysing files listed in '%s'", args.batch_arg);
            while(fgets(line[i], LSIZ, fp)) {
               line[i][strlen(line[i]) - 1] = '\0';
               i++;
            }

            tot = i;
            printf("\nThe content of the file %s are: \n",args.batch_arg);
            for(i = 0; i < tot; ++i) {
                printf("%s", line[i]);
                count_lines += 1;
            }

            for (int i = 0; i < count_lines; i++) {
                pid_t pid = fork();
                if (pid == -1){
                    ERROR(2, "Erro na execucao do fork\n");
                }
                else if(pid == 0){ //apenas se for filho
                    execlp("file", "file", "-E", "-b", "--mime-type", line[i], NULL);
                }
            }

            for (int i=0; i < count_lines; i++) {
               wait(NULL); //o pai espera por todos os filhos
            }

            fclose(fp);
            exit(EXIT_SUCCESS);
       }
       else {
           fprintf(stderr, "ERROR: cannot open file <%s> -- %s\n", args.batch_arg, strerror(errno));
       }
    }

	cmdline_parser_free(&args);
    return 0;
}
