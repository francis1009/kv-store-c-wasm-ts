#ifndef KVSTORE_H
#define KVSTORE_H

#include <stddef.h>

typedef enum {
	KV_SUCCESS = 0,
	KV_ERROR_KEY_NOT_FOUND = 1,
	KV_ERROR_OUT_OF_MEMORY = 2,
	KV_ERROR_INVALID_ARGUMENT = 3,
	KV_ERROR_IO_FAILURE = 4,
	KV_ERROR_UNKNOWN = 5
} KVStatus;

typedef struct {
	char *key;
	char *value;
	struct KVEntry *next;
} KVEntry;

typedef struct {
	KVEntry **entries;
	size_t capacity;
	size_t size;
} KVStore;

KVStore *kvstore_init(void);
void kvstore_destroy(KVStore *store);
KVStatus kvstore_set(KVStore *store, const char *key, const char *value);
KVStatus kvstore_delete(KVStore *store, const char *key);
const char *kvstore_get(KVStore *store, const char *key);

#endif
