#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#include <stdbool.h>

#define KEY_SIZE 256
#define VALUE_SIZE 256
#define STORE_SIZE 1024

typedef struct {
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
    bool in_use;
} KeyValuePair;

typedef struct {
    KeyValuePair store[STORE_SIZE];
} KeyValueStore;

void put(KeyValueStore *kv_store, const char *key, const char *value);
bool get(KeyValueStore *kv_store, const char *key, char *value);
bool del(KeyValueStore *kv_store, const char *key);

#endif