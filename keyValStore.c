#include "keyValStore.h"
#include <string.h>

void put(KeyValueStore *kv_store, const char *key, const char *value) {
    for (int i = 0; i < STORE_SIZE; i++) {
        if (!kv_store->store[i].in_use || strcmp(kv_store->store[i].key, key) == 0) {
            strncpy(kv_store->store[i].key, key, KEY_SIZE);
            strncpy(kv_store->store[i].value, value, VALUE_SIZE);
            kv_store->store[i].in_use = true;
            return;
        }
    }
}

bool get(KeyValueStore *kv_store, const char *key, char *value) {
    for (int i = 0; i < STORE_SIZE; i++) {
        if (kv_store->store[i].in_use && strcmp(kv_store->store[i].key, key) == 0) {
            strncpy(value, kv_store->store[i].value, VALUE_SIZE);
            return true;
        }
    }
    return false;
}

bool del(KeyValueStore *kv_store, const char *key) {
    for (int i = 0; i < STORE_SIZE; i++) {
        if (kv_store->store[i].in_use && strcmp(kv_store->store[i].key, key) == 0) {
            kv_store->store[i].in_use = false;
            return true;
        }
    }
    return false;
}


