#include "countnames.h"
#include <pthread.h>

static pthread_mutex_t tlock = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t init_lock = PTHREAD_MUTEX_INITIALIZER;

static NameCountData **hashtab = NULL; /* pointer table */
static int hashsize = 0;
static int entry_count = 0;


void table_init() {
    // Initialize table.
    hashsize = 10;
    hashtab = calloc(hashsize, sizeof(NameCountData *)); // Allocate memory.
    if (hashtab == NULL) {
        perror("calloc");
        exit(1);
    }
}

/* This is the hash function: form hash value for string s */
/* You can use a simple hash function: pid % hashsize */
unsigned hash(const char *name) {
    // Hashes entry in table.
    unsigned hashval = 0;
    for (int i = 0; name[i] != '\0'; i++)
        hashval += name[i];
    return hashval % hashsize;
}

/* lookup: Traverse hash table and look for name. */
NameCountData *lookup(const char *name) {
    if (hashtab == NULL) // Every time this if statement is used it checks if the hashtable has been allocated or not.
        return NULL;
    for (NameCountData *np = hashtab[hash(name)]; np != NULL; np = np->next)
        if (strcmp(name, np->name) == 0) {
            return np; /* found */
        }
    return NULL; /* not found */
}


static void rehash() {
    // Rehashes array
    int new_size = hashsize * 2;
    // Reallocates hash table size to new size
    NameCountData **new_table = realloc(hashtab, new_size * sizeof(NameCountData *));
    if (new_table == NULL) {
        perror("realloc");
        exit(1);
    }

    // Zeroes out the new slots in the table.
    for (int i = hashsize; i < new_size; i++)
        new_table[i] = NULL;

    hashtab = new_table;

    // Rehashes all existing nodes to the correct slots for the new size.
    int old_size = hashsize;
    hashsize = new_size;
    for (int i = 0; i < old_size; i++) {
        NameCountData *np = hashtab[i];
        hashtab[i] = NULL; // Clear the old slot.
        while (np != NULL) {
            NameCountData *next = np->next;
            unsigned hashval = hash(np->name); // Rehash entry with new hash size.
            np->next = hashtab[hashval]; // Move to next entry in table and set current entry.
            hashtab[hashval] = np;
            np = next;
        }
    }
}

NameCountData *insert(const NameCountMsg *ncm) {
    pthread_mutex_lock(&init_lock);
    if (hashtab == NULL) {
        table_init();
    }
    pthread_mutex_unlock(&init_lock);
    pthread_mutex_lock(&tlock);
    NameCountData *ncd;
    if ((ncd = lookup(ncm->name)) == NULL) {
        // Entry not found

        ncd = malloc(sizeof(NameCountData)); // Create new entry.
        if (ncd == NULL) {
            pthread_mutex_unlock(&tlock);
            return NULL;
        } // NULL tester
        ncd->name = strdup(ncm->name); // Copy name to np->name structure
        if (ncd->name == NULL) {
            // Check if name exists. If not then return NULL.
            pthread_mutex_unlock(&tlock);
            free(ncd);
            return NULL;
        }
        if (entry_count >= hashsize * 0.75)
            rehash();
        ncd->count = ncm->count; // Set count to actual count
        unsigned hashval = hash(ncm->name); // Set hashval to hash(name)
        ncd->next = hashtab[hashval]; // Set next entry to current entry at hashtable
        hashtab[hashval] = ncd; // Set current entry at hashtable to np.
        entry_count++;
    } else {
        ncd->count += ncm->count; // Increment counter for name.
    }
    pthread_mutex_unlock(&tlock);
    return ncd; // Return NameCountData structure
}

void table_print() {
    // Print contents of whole table.
    pthread_mutex_lock(&tlock);
    if (hashtab == NULL) {
        pthread_mutex_unlock(&tlock);
        return;
    }
    for (int i = 0; i < hashsize; i++) {
        NameCountData *np = hashtab[i]; // Get current element in table.
        while (np != NULL) {
            printf("%s: %d\n", np->name, np->count); // Print everything in table.
            np = np->next; // Go to next element in table.
        }
    }
    pthread_mutex_unlock(&tlock);
}

void table_destroy() {
    // Destroy table and initialize everything
    pthread_mutex_lock(&tlock);
    if (hashtab == NULL) {
        pthread_mutex_unlock(&tlock);
        return;
    }
    for (int i = 0; i < hashsize; i++) {
        NameCountData *np = hashtab[i];
        while (np != NULL) {
            NameCountData *next = np->next; // Save next pointer before freeing
            free(np->name); // Free strdup string
            free(np); // Free node itself
            np = next;
        }
    }
    free(hashtab); // Free array itself and clean up.
    hashtab = NULL;
    hashsize = 0;
    entry_count = 0;
    pthread_mutex_unlock(&tlock);
}
