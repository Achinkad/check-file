/*
 * @file type.c
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

#define NUM_OF_MIMES 7

int match_mime(const char *buf_extension, char **mime_list);

void check_mime(char *buf, char *file, int *num_ok, int *num_mismatch) {
    char *mime_list[NUM_OF_MIMES] = {"pdf", "gif", "jpg", "png", "mp4", "zip", "html"};
    char *buf_extension;   /* Real extension (from the file command) */
    char *file_extension;  /* Extension given by the file (not the command) */
    const char slash = '/';
    const char dot = '.';
    int match = 0;

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

   /*
    * Handle the special case of the mime type JPG, since it comes as JPEG
    * so it's necessary to change the buf_extension from JPEG to JPG to match
    * in the verifications made below in the function match_mime().
    */
    if (strcmp(file_extension, "jpg") == 0 && strcmp(buf_extension, "jpeg") == 0) memcpy(buf_extension, file_extension, strlen(file_extension)+1);

    match = match_mime(buf_extension, mime_list);

    if (match != -1) {
        /* Comparing the buf_extension with the file_extension to verified if the file is legit */
        if (strcmp(buf_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, buf_extension);
            *num_ok += 1;
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, buf_extension);
            *num_mismatch += 1;
        }
    } else {
        printf("[INFO] '%s': type '%s' is not supported by checkFile\n", file, buf);
    }
}

/*
 * Comparing the buf_extension given by the command file to the
 * array of mime types createad in the begging of the function main.
 */
int match_mime(const char *buf_extension, char **mime_list) {
    for (int i = 0; i < NUM_OF_MIMES; i++) {
        if (strcmp(buf_extension, mime_list[i]) == 0) {
            return i;
        }
    }
    return -1;
}
