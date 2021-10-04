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

void check_type(char *buf, char *file) {
    if (strcmp(buf, "application/pdf") == 0) {
        printf("[OK] '%s': extension 'pdf' matches file type 'pdf'\n", file);
    } else if (strcmp(buf, "image/gif") == 0) {
        printf("[OK] '%s': extension 'gif' matches file type 'gif'\n", file);
    } else {
        printf("[INFO] '%s': type '%s' is not supported by checkFile\n", file, buf);
    }
}
