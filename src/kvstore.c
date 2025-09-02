#include "kvstore.h"

#include <string.h>

#define INITIAL_CAPACITY 32

static unsigned long hash(const char *key) {
	unsigned long hash = 5381;
	int c;
	while ((c = *key++)) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

static int resize(KVStore *store) {
	printf("Resized");
	return 0;
}

KVStore *kvstore_init(void) {
	KVStore *store = malloc(sizeof(KVStore));
	if (store == NULL) {
		return NULL;
	}
	store->capacity = INITIAL_CAPACITY;
	store->size = 0;
	store->entries = calloc(store->capacity, sizeof(KVEntry *));
	if (store->entries == NULL) {
		free(store);
		return NULL;
	}
	return store;
}

void kvstore_destroy(KVStore *store) {
}

KVStatus kvstore_set(KVStore *store, const char *key, const char *value) {
	if (store == NULL || key == NULL || value == NULL) {
		return KV_ERROR_INVALID_ARGUMENT;
	}

	unsigned long h = hash(key);
	size_t index = h & (store->capacity - 1);
	KVEntry *curr_entry = store->entries[index];
	while (curr_entry != NULL) {
		if (strcmp(curr_entry->key, key) == 0) {
			free(curr_entry->value);
			curr_entry->value = strdup(value);
			if (curr_entry->value == NULL) {
				return KV_ERROR_OUT_OF_MEMORY;
			}
			return KV_SUCCESS;
		}
		curr_entry = curr_entry->next;
	}

	if ((float) store->size / store->capacity >= 0.75) {
		kvstore_resize(store);
		index = h & (store->capacity - 1);
	}

	KVEntry *new_entry = malloc(sizeof(KVEntry));
	if (new_entry == NULL) {
		return KV_ERROR_OUT_OF_MEMORY;
	}
	new_entry->key = strdup(key);
	new_entry->value = strdup(value);
	if (new_entry->key == NULL || new_entry->value == NULL) {
		free(new_entry->key);
		free(new_entry->value);
		free(new_entry);
		return KV_ERROR_OUT_OF_MEMORY;
	}

	new_entry->next = store->entries[index];
	store->entries[index] = new_entry;
	store->size++;

	return KV_SUCCESS;
}

KVStatus kvstore_delete(KVStore *store, const char *key) {
	return KV_SUCCESS;
}

const char *kvstore_get(KVStore *store, const char *key) {
	if (store == NULL || key == NULL) {
		return KV_ERROR_INVALID_ARGUMENT;
	}

	unsigned long h = hash(key);
	size_t index = h & (store->capacity - 1);
	KVEntry *curr_entry = store->entries[index];
	while (curr_entry != NULL) {
		if (strcmp(curr_entry->key, key) == 0) {
			return curr_entry->value;
		}
		curr_entry = curr_entry->next;
	}

	return NULL;
}
