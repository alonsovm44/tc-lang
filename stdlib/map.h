#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HASHMAP_INITIAL_CAPACITY 16
#define HASHMAP_LOAD_FACTOR 0.75

typedef struct HashMapEntry {
    char *key;
    int32_t value;
    struct HashMapEntry *next;
} HashMapEntry;

typedef struct HashMap {
    HashMapEntry **buckets;
    size_t capacity;
    size_t size;
} HashMap;

// Simple string hash function (djb2)
static uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

HashMap *hashmap_create(void) {
    HashMap *map = calloc(1, sizeof(HashMap));
    if (!map) return NULL;
    
    map->capacity = HASHMAP_INITIAL_CAPACITY;
    map->size = 0;
    map->buckets = calloc(map->capacity, sizeof(HashMapEntry *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    
    return map;
}

static void hashmap_resize(HashMap *map) {
    size_t new_capacity = map->capacity * 2;
    HashMapEntry **new_buckets = calloc(new_capacity, sizeof(HashMapEntry *));
    if (!new_buckets) return;
    
    // Rehash all entries
    for (size_t i = 0; i < map->capacity; i++) {
        HashMapEntry *entry = map->buckets[i];
        while (entry) {
            HashMapEntry *next = entry->next;
            uint32_t hash = hash_string(entry->key);
            size_t new_index = hash % new_capacity;
            
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;
            
            entry = next;
        }
    }
    
    free(map->buckets);
    map->buckets = new_buckets;
    map->capacity = new_capacity;
}

void hashmap_set(HashMap *map, const char *key, int32_t value) {
    if (!map || !key) return;
    
    // Check if we need to resize
    if ((double)map->size / map->capacity > HASHMAP_LOAD_FACTOR) {
        hashmap_resize(map);
    }
    
    uint32_t hash = hash_string(key);
    size_t index = hash % map->capacity;
    
    // Check if key already exists
    HashMapEntry *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    
    // Create new entry
    HashMapEntry *new_entry = malloc(sizeof(HashMapEntry));
    if (!new_entry) return;
    
    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    map->size++;
}

int32_t hashmap_get(HashMap *map, const char *key, int32_t default_value) {
    if (!map || !key) return default_value;
    
    uint32_t hash = hash_string(key);
    size_t index = hash % map->capacity;
    
    HashMapEntry *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    
    return default_value;
}

int hashmap_has(HashMap *map, const char *key) {
    if (!map || !key) return 0;
    
    uint32_t hash = hash_string(key);
    size_t index = hash % map->capacity;
    
    HashMapEntry *entry = map->buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return 1;
        }
        entry = entry->next;
    }
    
    return 0;
}

void hashmap_remove(HashMap *map, const char *key) {
    if (!map || !key) return;
    
    uint32_t hash = hash_string(key);
    size_t index = hash % map->capacity;
    
    HashMapEntry *entry = map->buckets[index];
    HashMapEntry *prev = NULL;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                map->buckets[index] = entry->next;
            }
            
            free(entry->key);
            free(entry);
            map->size--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

size_t hashmap_size(HashMap *map) {
    return map ? map->size : 0;
}

void hashmap_clear(HashMap *map) {
    if (!map) return;
    
    for (size_t i = 0; i < map->capacity; i++) {
        HashMapEntry *entry = map->buckets[i];
        while (entry) {
            HashMapEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
        map->buckets[i] = NULL;
    }
    map->size = 0;
}

void hashmap_destroy(HashMap *map) {
    if (!map) return;
    
    hashmap_clear(map);
    free(map->buckets);
    free(map);
}
