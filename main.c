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

#include "args.h"
#include "debug.h"
#include "memory.h"

int main(int argc, char *argv[]) {
    struct gengetopt_args_info args;

    if(cmdline_parser(argc, argv, &args) != 0) {
        ERROR(1, "ERROR: error in cmdline_parser\n");
    }

    if (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0) {
        if (access(args.file_arg, F_OK) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                execlp("file", "file", "-E", args.file_arg, NULL);
                fprintf(stderr, "ERROR: cannot execute the command <execlp> -- %s\n", strerror(errno));
                exit(1);
            } else if(pid > 0) {
                wait(NULL);
            } else {
                ERROR(2, "ERROR: can't execute fork\n");
            }
        } else {
            fprintf(stderr, "ERROR: cannot open file <%s> -- %s\n", args.file_arg, strerror(errno));
        }
    }

    if (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--batch") == 0) {
        if (access(args.batch_arg, F_OK) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                // open file and read
                // ciclo for para cada linha
            } else if(pid > 0) {
                wait(NULL);
            } else {
                ERROR(2, "ERROR: can't execute fork\n");
            }
        } else {
            fprintf(stderr, "ERROR: cannot open file <%s> -- %s\n", args.file_arg, strerror(errno));
        }
    }

    if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--dir") == 0) {
        if (access(args.file_arg, F_OK) == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                // Verificar o tipo de ficheiro para o diretorio a apontar
            } else if(pid > 0) {
                wait(NULL);
            } else {
                ERROR(2, "ERROR: can't execute fork\n");
            }
        } else {
            fprintf(stderr, "ERROR: cannot open file <%s> -- %s\n", args.file_arg, strerror(errno));
        }
    }

	cmdline_parser_free(&args);
    return 0;
}
