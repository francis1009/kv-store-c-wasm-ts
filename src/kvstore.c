#include "kvstore.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 32
#define SNAPSHOT_FILENAME "kvstore.snapshot"

static unsigned long hash(const char *key) {
	unsigned long hash = 5381;
	int c;
	while ((c = *key++)) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

static int resize(KVStore *store) {
	size_t new_capacity = store->capacity * 2;
	KVEntry **new_entries = calloc(new_capacity, sizeof(KVEntry *));
	if (new_entries == NULL) {
		return -1;
	}

	for (size_t i = 0; i < store->capacity; i++) {
		KVEntry *entry = store->entries[i];
		while (entry != NULL) {
			KVEntry *next_entry = entry->next;
			unsigned long h = hash(entry->key);
			size_t new_index = h & (new_capacity - 1);
			entry->next = new_entries[new_index];
			new_entries[new_index] = entry;
			entry = next_entry;
		}
	}

	free(store->entries);
	store->entries = new_entries;
	store->capacity = new_capacity;

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
	if (store == NULL) {
		return;
	}

	for (size_t i = 0; i < store->capacity; i++) {
		KVEntry *entry = store->entries[i];
		while (entry != NULL) {
			KVEntry *next_entry = entry->next;
			free(entry->key);
			free(entry->value);
			free(entry);
			entry = next_entry;
		}
	}

	free(store->entries);
	free(store);
}

void kvstore_save(KVStore *store) {
	if (store == NULL) {
		return;
	}

	FILE *snapshot = fopen(SNAPSHOT_FILENAME, "wb");
	if (snapshot == NULL) {
		perror("Error opening snapshot file for writing");
		return;
	}

	for (size_t i = 0; i < store->capacity; i++) {
		KVEntry *entry = store->entries[i];
		while (entry != NULL) {
			uint32_t key_len = strlen(entry->key);
			fwrite(&key_len, sizeof(uint32_t), 1, snapshot);
			fwrite(entry->key, key_len, 1, snapshot);
			uint64_t value_len = entry->value_len;

			fwrite(&value_len, sizeof(uint64_t), 1, snapshot);
			fwrite(entry->value, value_len, 1, snapshot);
			entry = entry->next;
		}
	}

	fclose(snapshot);
}

void kvstore_load(KVStore *store) {
	if (store == NULL) {
		return;
	}
	FILE *snapshot = fopen(SNAPSHOT_FILENAME, "rb");
	if (snapshot == NULL) {
		return;
	}

	uint32_t key_len;
	while (fread(&key_len, sizeof(uint32_t), 1, snapshot) == 1) {
		char *key = malloc(key_len + 1);
		if (fread(key, key_len, 1, snapshot) != 1) {
			break;
		}
		key[key_len] = '\0';

		uint64_t value_len;
		if (fread(&value_len, sizeof(uint64_t), 1, snapshot) != 1) {
			free(key);
			break;
		}

		void *value = malloc(value_len);
		if (value_len > 0 && fread(value, value_len, 1, snapshot) != 1) {
			free(key);
			free(value);
			break;
		}

		kvstore_set(store, key, value, value_len);
		free(key);
		free(value);
	}

	fclose(snapshot);
}

KVStatus kvstore_set(KVStore *store, const char *key, const void *value,
										 size_t value_len) {
	if (store == NULL || key == NULL || value == NULL) {
		return KV_ERROR_INVALID_ARGUMENT;
	}

	unsigned long h = hash(key);
	size_t index = h & (store->capacity - 1);
	KVEntry *curr_entry = store->entries[index];
	while (curr_entry != NULL) {
		if (strcmp(curr_entry->key, key) == 0) {
			void *new_value = realloc(curr_entry->value, value_len);
			if (new_value == NULL && value_len > 0) {
				return KV_ERROR_OUT_OF_MEMORY;
			}
			curr_entry->value = new_value;
			curr_entry->value_len = value_len;
			if (curr_entry->value_len > 0) {
				memcpy(curr_entry->value, value, curr_entry->value_len);
			}
			return KV_SUCCESS;
		}
		curr_entry = curr_entry->next;
	}

	if ((float) store->size / store->capacity >= 0.75) {
		if (resize(store) != 0) {
			return KV_ERROR_OUT_OF_MEMORY;
		}
		index = h & (store->capacity - 1);
	}

	KVEntry *new_entry = malloc(sizeof(KVEntry));
	if (new_entry == NULL) {
		return KV_ERROR_OUT_OF_MEMORY;
	}
	new_entry->key = strdup(key);
	if (new_entry->key == NULL) {
		free(new_entry);
		return KV_ERROR_OUT_OF_MEMORY;
	}
	new_entry->value_len = value_len;
	if (value_len > 0) {
		new_entry->value = malloc(value_len);
		if (new_entry->value == NULL) {
			free(new_entry->key);
			free(new_entry);
			return KV_ERROR_OUT_OF_MEMORY;
		}
		memcpy(new_entry->value, value, value_len);
	} else {
		new_entry->value = NULL;
	}

	new_entry->next = store->entries[index];
	store->entries[index] = new_entry;
	store->size++;

	return KV_SUCCESS;
}

KVStatus kvstore_delete(KVStore *store, const char *key) {
	if (store == NULL || key == NULL) {
		return KV_ERROR_INVALID_ARGUMENT;
	}

	unsigned long h = hash(key);
	size_t index = h & (store->capacity - 1);
	KVEntry *curr_entry = store->entries[index];
	KVEntry *prev_entry = NULL;
	while (curr_entry != NULL) {
		if (strcmp(curr_entry->key, key) == 0) {
			if (prev_entry == NULL) {
				store->entries[index] = curr_entry->next;
			} else {
				prev_entry->next = curr_entry->next;
			}
			free(curr_entry->key);
			free(curr_entry->value);
			free(curr_entry);
			store->size--;
			return KV_SUCCESS;
		}
		prev_entry = curr_entry;
		curr_entry = curr_entry->next;
	}

	return KV_ERROR_KEY_NOT_FOUND;
}

const void *kvstore_get(KVStore *store, const char *key,
												size_t *value_len_out) {
	if (store == NULL || key == NULL) {
		return NULL;
	}

	unsigned long h = hash(key);
	size_t index = h & (store->capacity - 1);
	KVEntry *curr_entry = store->entries[index];
	while (curr_entry != NULL) {
		if (strcmp(curr_entry->key, key) == 0) {
			if (value_len_out != NULL) {
				*value_len_out = curr_entry->value_len;
			}
			return curr_entry->value;
		}
		curr_entry = curr_entry->next;
	}

	return NULL;
}
