#include <sys/types.h>
#include <unistd.h>
#include "countnames.h"


/*
 *   NOTE: A history of this code is available on a GitHub repository.
 *   This repository can be made available upon request.
 */

void* thread_runner(void*);
pthread_t tid1, tid2;

int main(int argc, char *argv[]) /* int argc = argument count
                                  * char *argv[] = string array containing the actual arguments passed.*/

{
    char filename[256];
    sprintf(filename, "%s/%d.out", output_path, getpid());

    /* Create a PID.out for this child process
    and then set stdout to this PID.out */

    if (freopen(filename, "w", stdout) == NULL) {
        perror("freopen failed");
        return 1;
    }
    size_t global_size = MAXARGS * MSIZE * sizeof(NameCountMsg);

    /* Create a PID.err for this child process
    and then set stderr to this PID.err */

    char fileerr[256];
    sprintf(fileerr, "%s/%d.err", output_path, getpid());

    if (freopen(fileerr, "w", stderr) == NULL) {
        perror("freopen failed");
        return 1;
    }
    if (argc == 1) // If no file was provided
    {
        puts("No file provided, exiting."); // This informs the user that there is no file.
        return 0;
    }

    FILE *f = fopen(argv[1], "r"); // Opens file to read from.
    if (f == NULL) {
        fprintf(stderr, "Error: Child %d could not open file %s and is exiting.\n", getpid(), argv[1]);
        return 1;
    }

    int i = 0, lnum = 0;
    char namebuf[MLINE] = {0}; // This buffer temporarily stores a line in the file.
    char **names = calloc(MSIZE, sizeof(char *)); // This stores all the names and their occurences in the file.
    while (fgets(namebuf, MLINE, f)) {
        lnum++;
        char *tok = strtok(namebuf, "\n"); // Uses "\n" character to tokenize string.
        if (tok == NULL) {
            fprintf(stderr, "Warning: Line %d in Child %d is empty.\n", lnum, getpid());
            continue;
        }
        names[i++] = strdup(tok); /* This allocates memory on the heap to store the string,
                                      which needs to be freed later. */
    }
    fclose(f);
    int *count = calloc(MNAME, sizeof(int)); // Contains the number of times a name occurs in the file.
    char **nused = calloc(MNAME, sizeof(char *)); // Contains the number of unique names used in the file.
    ncount(names, nused, count); // Counts the names used and sends it to the arrays.
    int mem_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0); // Open memory area in child process.

    if (mem_fd == -1) {
        perror("shm_open error"); // Can't open assigned memory location.
    }

    if (argv[2] == NULL) {
        perror("slot missing"); // Slot is not passed as an argument.
    }
    int slot = atoi(argv[2]); // Get slot from parent.
    void *child_mem = mmap(NULL, global_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0); // Map memory to that slot

    if (child_mem == MAP_FAILED) {
        // Free everything, the program needs to exit and no longer needs these.
        perror("mmap error");
        clnup(names, nused);
        free(names);
        free(nused);
        free(count);
        _exit(1);
    }

    NameCountMsg *space = (NameCountMsg *) child_mem + slot * MNAME; // Compute address where data will be written.

    for (i = 0; nused[i] != 0; i++) {
        snprintf(space[i].name, MLINE, "%s", nused[i]); // Put name to slot in space.
        space[i].count = count[i]; // Put count to slot in space.
    }

    // Cleanup routines (flushing output, freeing memory, unmapping shared memory, closing mem_fd)
    fflush(stdout);
    clnup(names, nused);
    free(names);
    free(nused);
    free(count);
    munmap(child_mem, global_size);
    close(mem_fd);
    return 0;
}
