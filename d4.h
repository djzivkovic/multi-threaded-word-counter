#include <pthread.h>

#define MAX_WORD_LEN 64 // maximum allowed word length, including \0
#define MAX_SCANNERS 100 // number of scanners
#define MAX_STOP_WORDS 100

#define HM_CAPACITY 10000
#define HM_LOCK_SIZE 1
#define HM_LOCKS HM_CAPACITY / HM_LOCK_SIZE

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct hm_object // node within the hash_map structure
{
    char key[MAX_WORD_LEN]; // word
    int value; // current frequency
    struct hm_object* next;
} hm_object;

typedef struct hash_map
{
    hm_object values[HM_CAPACITY];
    pthread_mutex_t locks[HM_LOCKS];
} hash_map;

typedef struct search_result // search result structure
{
    char key[MAX_WORD_LEN]; // word
    int value; // current frequency
} search_result;

typedef struct scanned_file // file that the scanner has already scanned
{
    char file_name[256]; // file name
    time_t mod_time; // time of the last file modification
    int last_pos;
} scanned_file;

extern void scanner_init(); // called once at the beginning of the system's operation
extern void *scanner_work(void *_args); // scanner thread function

extern void map_init(); // called once at the beginning of the system's operation
extern void map_add_word_count(char *word); // operation to add words and their frequencies
extern search_result *map_get_frequency(char *word); // search operation
extern void add_stopword(char* word); // adds a word to the stop words array
