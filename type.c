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

void check_mime(char *buf, char *file) {
    char * buf_extension;   /* Real extension (from the file command) */
    char * file_extension;  /* Extension given by the file (not the command) */
    const char slash = '/';
    const char dot = '.';

   /*
    * Find the last occurrence of the character '/' and '.' in the
    * strings buf_extension and file_extension respectively.
    */
    buf_extension = strrchr(buf, slash);
    file_extension = strrchr(file, dot);

   /*
    * Since the strings buf_extension and file_extension start with the
    * character '/' and '.' respectively, we need to remove that characters
    * in order to compare both strings and the code below.
    */
    if (buf_extension[0] == '/') buf_extension++;
    if (file_extension[0] == '.') file_extension++;

    if (strcmp(buf, "application/pdf") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }

    if (strcmp(buf, "image/gif") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }

    if (strcmp(buf, "image/jpeg") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }

    if (strcmp(buf, "image/png") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }

    if (strcmp(buf, "video/mp4") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }

    if (strcmp(buf, "application/zip") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }

    if (strcmp(buf, "text/html") == 0) {
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
        }
    }
}
