#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "kvstore.h"

void print_hex(const char *label, const void *data, size_t len) {
	printf("%s (%zu bytes): ", label, len);
	const unsigned char *p = data;
	for (size_t i = 0; i < len; i++) {
		printf("%02x ", p[i]);
	}
	printf("\n");
}

int main(void) {
	KVStore *store = kvstore_init();
	if (store == NULL) {
		fprintf(stderr, "Failed to initialize KV store.\n");
		return 1;
	}

	printf("----- 1. Populating initial store -----\n");
	kvstore_set(store, "name", "Francis", strlen("Francis"));
	kvstore_set(store, "project", "kv-store", strlen("kv-store"));
	char blob[] = {'A', 'B', '\0', 'C', 'D'};
	size_t blob_len = sizeof(blob);
	kvstore_set(store, "myblob", blob, blob_len);
	kvstore_set(store, "name", "Gemini", strlen("Gemini"));
	kvstore_delete(store, "project");
	for (int i = 0; i < 30; i++) {
		char key[16];
		sprintf(key, "key_%d", i);
		kvstore_set(store, key, "v", 1);
	}
	printf("Store populated. Final size: %zu, Final capacity: %zu\n\n",
				 store->size, store->capacity);

	printf("----- 2. Saving snapshot to disk -----\n");
	kvstore_save(store);
	printf("Snapshot saved.\n\n");

	printf("----- 3. Destroying the in-memory store -----\n");
	kvstore_destroy(store);
	store = NULL;
	printf("Store destroyed.\n\n");

	printf(
			"----- 4. Creating a new, empty store and loading from snapshot -----\n");
	store = kvstore_init();
	if (store == NULL) {
		fprintf(stderr, "Failed to re-initialize KV store.\n");
		return 1;
	}
	kvstore_load(store);
	printf("Snapshot loaded into new store.\n\n");

	printf("----- 5. Verifying data in the loaded store -----\n");
	printf("Loaded store size: %zu, Loaded store capacity: %zu\n", store->size,
				 store->capacity);
	assert(store->size == 32);
	assert(store->capacity == 64);
	size_t len_out;
	const char *name = kvstore_get(store, "name", &len_out);
	printf("Get loaded 'name': %.*s\n", (int) len_out, name);
	assert(name != NULL);
	assert(len_out == strlen("Gemini"));
	assert(memcmp(name, "Gemini", len_out) == 0);
	const void *retrieved_blob = kvstore_get(store, "myblob", &len_out);
	print_hex("Get loaded 'myblob'", retrieved_blob, len_out);
	assert(retrieved_blob != NULL);
	assert(len_out == blob_len);
	assert(memcmp(retrieved_blob, blob, blob_len) == 0);
	printf("Verifying 'project' is still deleted...\n");
	assert(kvstore_get(store, "project", &len_out) == NULL);
	printf("Verifying one of the resized keys ('key_15')...\n");
	assert(kvstore_get(store, "key_15", &len_out) != NULL);
	printf("All loaded data verified successfully!\n\n");

	printf("----- 6. Final cleanup -----\n");
	kvstore_destroy(store);
	printf("Loaded store destroyed.\n\n");

	return 0;
}
