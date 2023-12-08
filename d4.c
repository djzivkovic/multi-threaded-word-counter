#include <time.h>
#include "d4.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

scanned_file files[MAX_SCANNERS];
pthread_t scanners[MAX_SCANNERS];
pthread_t scanned_lock;
char stop_words[MAX_STOP_WORDS][MAX_WORD_LEN];
int n_stop_words = 0;
int scanned_files = 0;
int n = 0;
int quit = 0;
hash_map *hm;

void map_init()
{
    hm = calloc(1, sizeof(struct hash_map));
    for(int i = 0;i<HM_LOCKS;i++)
    {
        pthread_mutex_init(&(hm->locks[i]), NULL);
    }
}

int hash_key(const char* key) {
    unsigned long long hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (unsigned long long)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash % HM_CAPACITY;
}

void map_add_word_count(char *word)
{
    int key = hash_key(word);
    int lock_index = key / HM_LOCK_SIZE;
    hm_object *obj = &hm->values[key];
    if(!obj->value)
    {
        pthread_mutex_lock(hm->locks + lock_index);
        strcpy(obj->key, word);
        obj->value = 1;
        obj->next = NULL;
        pthread_mutex_unlock(hm->locks + lock_index);
        return;
    }
    hm_object *prev = obj;
    while(obj)
    {
        if(!strcmp(obj->key, word))
        {
            pthread_mutex_lock(hm->locks + lock_index);
            obj->value++;
            pthread_mutex_unlock(hm->locks + lock_index);
            return;
        }
        prev = obj;
        obj = obj->next;
    }
    pthread_mutex_lock(hm->locks + lock_index);
    hm_object *newObj = malloc(sizeof(struct hm_object));
    strcpy(newObj->key, word);
    newObj->value = 1;
    newObj->next = NULL;
    prev->next = newObj;
    pthread_mutex_unlock(hm->locks + lock_index);
}

search_result *map_get_frequency(char *word)
{
    int key = hash_key(word);
    hm_object *obj = &hm->values[key];
    search_result* result = malloc(sizeof(struct search_result));
    strcpy(result->key, word);
    if(obj->value == 0)
    {
        result->value = 0;
        return result;
    }
    while(obj)
    {
        if(!strcmp(obj->key, word))
        {
            result->value = obj->value;
            return result;
        }
        obj = obj->next;
    }
    result->value = 0;
    return result;
}

int get_scanned_file(char* file)
{
    for(int i = 0; i < scanned_files; i++)
    {
        scanned_file currFile = files[i];
        if (!strcmp(currFile.file_name, file))
        {
            return i;
        }
    }
    return -1;
}

int file_exists(char *file) {
  struct stat *buffer = malloc(sizeof(struct stat));
  int res = stat(file, buffer) == 0;
  free(buffer);
  return res;
}

int file_changed(char* file)
{
    struct stat *buff;
    buff = malloc(sizeof(struct stat));
    int val = stat(file, buff);
    if (val == 0)
    {
        int i = get_scanned_file(file);
        if(i >= 0)
        {
            scanned_file currFile = files[i];
            int modified = buff->st_mtime > currFile.mod_time;
            if (modified)
            {
                files[i].mod_time = buff->st_mtime;
            }
            free(buff);
            return modified;
        }
        scanned_file s_file = {.mod_time = buff->st_mtime};
        strcpy(s_file.file_name, file);

        pthread_mutex_lock(&scanned_lock);
        files[scanned_files] = s_file;
        scanned_files++;
        pthread_mutex_unlock(&scanned_lock);

        free(buff);
        return 1;
    }
    printf("File doesn't exist: %s\n", file);
    pthread_exit(NULL);
    return -1;
}



void process_word(char *word) {
    for(int i = 0;i<n_stop_words;i++)
    {
        if(!strcmp(stop_words[i], word))
            return;
    }
    map_add_word_count(word);
}

int scan_file(char* file, FILE* fp)
{
    char word[64];
    int index = 0;
    int valid_word = 1;
    int c;
    int last_pos = 0;
    int i = get_scanned_file(file);
    while ((c = fgetc(fp)) != EOF) {
        last_pos++;
        if(last_pos <= files[i].last_pos) continue;
        if (isspace(c)) {
            if (index != 0 && valid_word) {
                word[index] = '\0';
                process_word(word);
                index = 0;
            }
            valid_word = 1;
        } else if (isalpha(c)) {
            if (index < 63) {
                word[index++] = tolower((char)c);
            }
        } else {
            valid_word = 0;
        }
    }
    if (index != 0 && valid_word) {
        word[index] = '\0';
        process_word(word);
    }

    files[i].last_pos = last_pos;
    printf("Done scanning: %s\n", file);

}
void add_stopwords(char * file)
{
    FILE *fp = fopen(file, "r");
    int c;
    char word[64];
    int index = 0;
    int valid_word = 1;
    if (file != NULL) {
        while ((c = fgetc(fp)) != EOF) {
            if (isspace(c)) {
                if (index != 0 && valid_word) {
                    word[index] = '\0';
                    strcpy(stop_words[n_stop_words++], word);
                    index = 0;
                }
                valid_word = 1;
            } else if (isalpha(c)) {
                if (index < 63) {
                    word[index++] = tolower((char)c);
                }
            } else {
                valid_word = 0;
            }
        }
        if (index != 0 && valid_word) {
            word[index] = '\0';
            strcpy(stop_words[n_stop_words++], word);
        }
        fclose(fp);
    }
}

void scanner_init(char* file)
{
    printf("File: %s\n", file);
    if(!file_exists(file)) {
        printf("File doesn't exist: %s\n", file);
        return;
    }
    if(get_scanned_file(file) >= 0)
    {
        printf("File already scanned: %s\n", file);
        return;
    }
    pthread_create(&scanners[n++], NULL, scanner_work, file);
}

void *scanner_work(void *_args)
{
    char file[256];
    strcpy(file, (char *) _args);
    while (1)
    {
        if(quit){
            printf("Quitting.\n");
            pthread_exit(NULL);
        }
        if (file_changed(file) > 0)
        {
            FILE *fp = fopen(file, "r");
            if (file != NULL) {
                scan_file(file, fp);
                fclose(fp);
            }
        }
        sleep(5);
    }

}

int main(int argc, char *argv[])
{
    if(argc == 2)
    {
        char* stopFile = argv[1];
        add_stopwords(stopFile);
    }
    pthread_mutex_init(&scanned_lock, NULL);
    map_init();
    char command[256];
    while(1)
    {
        gets(command);

        if(!strncmp(command, "_count_ ", 8))
        {
            char *file = command + 8;
            scanner_init(file);
        }
        else if(!strcmp(command, "_stop_"))
        {
            quit = 1;
            for(int i=0;i<n;i++)
            {
                pthread_join(scanners[i], NULL);
            }
            for(int i = 0;i<HM_LOCKS;i++)
            {
                pthread_mutex_destroy(&(hm->locks[i]));
            }
            free(hm);
            return 0;
        }
        else
        {
            int i = strlen(command);
            while(i >= 1)
            {
                if(command[i-1] == ' ') break;
                i--;
            }
            char word[256];
            strcpy(word, command + i);
            search_result* result = map_get_frequency(word);
            printf("Search result: %s: %d\n", result->key, result->value);
            free(result);
        }
    }
}

