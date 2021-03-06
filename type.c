/*
 * @file type.c
 * @start_date 2021-09-28
 * @author_01 Belisa Lopes <2200724@my.ipleiria.pt>
 * @author_02 José P. Areia <2200655@my.ipleiria.pt>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#include "debug.h"
#include "memory.h"

#define ERR_EXEC        3
#define ERR_FORK        4
#define ERR_OPEN_FILE   5
#define NUM_OF_MIMES    7
#define BUFFER_SIZE     2047

extern void open_output_file();

int check_text_file(char *filename);
int match_mime(const char *cmd_file_extension, char **mime_list);

void check_mime(char *mime, char *file, int *num_ok, int *num_mismatch) {
    char *mime_list[NUM_OF_MIMES] = {"pdf", "gif", "jpg", "png", "mp4", "zip", "html"};
    char *cmd_file_extension;   /* Real extension (from the file command) */
    char *file_extension;       /* Extension given by the file (not the command) */
    const char slash = '/';
    const char dot = '.';
    int match = 0;

   /*
    * Find the last occurrence of the character '/' and '.' in the
    * strings cmd_file_extension and file_extension respectively.
    */
    if (strrchr(mime, slash) != NULL) cmd_file_extension = strrchr(mime, slash);
    if (strrchr(file, dot) != NULL) file_extension = strrchr(file, dot);

   /*
    * Since the strings cmd_file_extension and file_extension start with the
    * character '/' and '.' respectively, we need to remove that characters
    * in order to compare both strings and the code below.
    */
    if (cmd_file_extension[0] == '/') cmd_file_extension++;
    if (file_extension[0] == '.') file_extension++;

   /*
    * Handle the special case of the mime type JPG, since it comes as JPEG
    * so it's necessary to change the cmd_file_extension from JPEG to JPG to match
    * in the verifications made below in the function match_mime().
    */
    if (strcmp(file_extension, "jpg") == 0 && strcmp(cmd_file_extension, "jpeg") == 0)
        memcpy(cmd_file_extension, file_extension, strlen(file_extension) + 1);

    match = match_mime(cmd_file_extension, mime_list);

    if (match != -1) {
        /* Comparing the cmd_file_extension with the file_extension to verified if the file is legit */
        if (strcmp(cmd_file_extension, file_extension) == 0) {
            printf("[OK] '%s': extension '%s' matches file type '%s'\n", file, file_extension, cmd_file_extension);
            *num_ok += 1;
        } else {
            printf("[MISMATCH] '%s': extension is '%s', file type is '%s'\n", file, file_extension, cmd_file_extension);
            *num_mismatch += 1;
        }
    } else {
        printf("[INFO] '%s': type '%s' is not supported by checkFile\n", file, mime);
    }
}

/*
 * Comparing the cmd_file_extension given by the command file to the
 * array of mime types createad in the begging of the function main.
 */
int match_mime(const char *cmd_file_extension, char **mime_list) {
    for (int i = 0; i < NUM_OF_MIMES; i++) {
        if (strcmp(cmd_file_extension, mime_list[i]) == 0)
            return i;
    }
    return -1;
}

/*
 * This is function will verify if the file mime type is a text.
 * If true, the function will return 1 and the -b/--batch mode will proceed
 * correctly, otherwise the application it will end with an error.
 * It only works for the -b/--batch mode.
 */
int check_text_file(char *filename) {
    pid_t pid = fork();
    if (pid == 0) {
        open_output_file();
        execlp("file", "file", "-b", "--mime-type", filename, NULL);
        ERROR(ERR_EXEC, "Failed to execute execlp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        /* Check if there was an error while executing the execlp and proceed with a failure exit */
        if (WIFEXITED(status) && WEXITSTATUS(status) == ERR_EXEC)
            exit(EXIT_FAILURE);
    } else {
        ERROR(ERR_FORK, "Failed to execute fork");
    }

    int match = 0;
    char *extension;
    char *mime = MALLOC(BUFFER_SIZE * sizeof(char) + 1);

    FILE *fd = fopen("output.txt", "r");
    if (fd == NULL)
        ERROR(ERR_OPEN_FILE, "cannot open the file 'output.txt'");

    fscanf(fd, "%s", mime);

    extension = strtok(mime, "/");
    match = strcmp(extension, "text");

    fclose(fd);
    free(mime);

    return match == 0 ? 1 : 0;
}
