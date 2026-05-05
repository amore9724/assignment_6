#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "countnames.h"

/*
 *   NOTE: A history of this code is available on a GitHub repository.
 *   This repository can be made available upon request.
 */

void *GLOBAL = NULL;

void *thread_task(void *arg) {      // This is basically a version of countnames which executes within a thread.
    thread_args *targ = (thread_args*) arg;    // Arguments passed to the function through a structure.
    FILE *f = fopen(targ->filename, "r");   // Open file for counting names.
    if (f == NULL) {
        fputs("error: cannot open file\n\0", stderr); // This prints to stderr an error and exits the program.
        return NULL;
    }
    char namebuf[MSIZE] = {0}; // This buffer temporarily stores a line in the file.
    char *names[MSIZE] = {0}; // This stores all of the names and their occurences in the file.
    int i = 0, lnum = 0;
    while (fgets(namebuf, MSIZE, f) && i < MSIZE - 1) {
        lnum++;
        char *saveptr;
        char *tok = strtok_r(namebuf, "\n", &saveptr); // Uses "\n" character to tokenize string, strtok_r is thread safe.
        if (tok == NULL) {
            fprintf(stderr, "Warning: Line %d is empty.\n", lnum); // Prints warning if token is null and skips.
            continue;
        }
        names[i++] = strdup(tok); /* This allocates memory on the heap to store the string,
                                     which needs to be freed later. */
    }

    fclose(f);
    int count[MNAME] = {0}; // Contains the number of times a name occurs in the file.
    char *nused[MNAME] = {0}; // Contains the number of unique names used in the file.
    ncount(names, nused, count);
    for (i = 0; nused[i] != 0; i++) {
        NameCountMsg msg;
        snprintf(msg.name, MLINE, "%s", nused[i]);
        msg.count = count[i];
        insert(&msg);
    }
    clnup(names, nused); // This will free the allocated memory.
    return NULL;
}

int main(int argc, char *argv[]) /* int argc = argument count
                                  * char *argv[] = string array containing the actual arguments passed.*/
{
    char filename[256];
    sprintf(filename, "%s/%d.out", output_path, getpid());
    int fd_output = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    /* Create a PID.out for this child process
    and then set stdout to this PID.out */
    dup2(fd_output, STDOUT_FILENO);
    if (fd_output == -1) {
        perror("Failed to reassign output stream.");
        return 1;
    }

    char fileerr[256];
    sprintf(fileerr, "%s/%d.err", output_path, getpid());

    int fd_err = open(fileerr, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_err == -1) {
        perror("Failed to reassign error stream.");
        return 1;
    }
    dup2(fd_err, STDERR_FILENO);
    if (argc == 1) // If no file was provided
    {
        char buf[MAXLINE];
        while (fgets(buf, MAXLINE, stdin) != NULL) {

            if (buf[strlen(buf) - 1] == '\n')
                buf[strlen(buf) - 1] = 0; /* replace newline with null */

            char *token = strtok(buf, " ");

            while (token != NULL) {
                argv[argc++] = token;
                token = strtok(NULL, " ");
            }

            argv[argc] = NULL;
            if (argc == 1) {
                continue;
            }
            break;
        }
    }
    GLOBAL = calloc((argc-1)*MNAME, sizeof(NameCountMsg));
    pthread_t threads[argc - 1]; // Initialize thread count.
    thread_args *targ = calloc(argc-1, sizeof(thread_args));
    for (int i = 1; i < argc; i++) {
        targ[i-1].filename = strdup(argv[i]);
        targ[i-1].slot = i-1;
        pthread_create(&threads[i-1], NULL, thread_task, &targ[i-1]); // Create threads to do task.
        /* First argument is the thread in question,
         * third argument is the function to be executed,
         * fourth argument is the arguments for the function */
    }
    for (int i = 0; i < argc-1; i++) {pthread_join(threads[i], NULL);}  // Wait for all threads to join
    table_print();  // Print contents of table

    /* This section of the program frees all memory. */
    table_destroy();
    free(GLOBAL);
    for (int i = 0; i < argc-1; i++)
        free(targ[i].filename);
    free(targ);
    targ = NULL;
    GLOBAL = NULL;
    return 0;
}