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

typedef struct KVEntry {
	char *key;
	void *value;
	size_t value_len;
	struct KVEntry *next;
} KVEntry;

typedef struct {
	KVEntry **entries;
	size_t capacity;
	size_t size;
} KVStore;

KVStore *kvstore_init(void);
void kvstore_destroy(KVStore *store);
KVStatus kvstore_set(KVStore *store, const char *key, const void *value,
										 size_t value_len);
KVStatus kvstore_delete(KVStore *store, const char *key);
const void *kvstore_get(KVStore *store, const char *key, size_t *value_len_out);

#endif
