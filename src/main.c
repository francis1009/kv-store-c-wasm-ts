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

	printf("----- 1. Basic Set/Get Operations -----\n");
	kvstore_set(store, "name", "Francis", strlen("Francis"));
	kvstore_set(store, "project", "kv-store", strlen("kv-store"));

	size_t len_out;
	const char *name = kvstore_get(store, "name", &len_out);
	printf("Get 'name': %.*s (len: %zu)\n", (int) len_out, name, len_out);
	assert(len_out == strlen("Francis"));
	assert(memcmp(name, "Francis", len_out) == 0);
	printf("\n");

	printf("----- 2. Testing Binary Data with '\\0' -----\n");
	char blob[] = {'A', 'B', '\0', 'C', 'D'};
	size_t blob_len = sizeof(blob);

	kvstore_set(store, "myblob", blob, blob_len);
	print_hex("Set 'myblob'", blob, blob_len);

	const void *retrieved_blob = kvstore_get(store, "myblob", &len_out);
	print_hex("Get 'myblob'", retrieved_blob, len_out);

	assert(retrieved_blob != NULL);
	assert(len_out == blob_len);
	assert(memcmp(retrieved_blob, blob, blob_len) == 0);
	printf("Binary data verified successfully!\n");
	printf("\n");

	printf("----- 3. Testing Update and Edge Cases -----\n");
	kvstore_set(store, "name", "Gemini", strlen("Gemini"));
	name = kvstore_get(store, "name", &len_out);
	printf("Updated 'name': %.*s (len: %zu)\n", (int) len_out, name, len_out);
	assert(memcmp(name, "Gemini", len_out) == 0);

	printf("Getting non-existent key 'foo': %p\n",
				 kvstore_get(store, "foo", &len_out));
	assert(kvstore_get(store, "foo", &len_out) == NULL);

	printf("Deleting 'project'...\n");
	assert(kvstore_delete(store, "project") == KV_SUCCESS);
	assert(kvstore_get(store, "project", &len_out) == NULL);
	printf("Delete successful.\n\n");

	printf("----- 4. Testing Resize Functionality -----\n");
	printf("Initial capacity: %zu, Initial size: %zu\n", store->capacity,
				 store->size);
	printf("Adding 30 new keys to trigger a resize...\n");
	for (int i = 0; i < 30; i++) {
		char key[16];
		sprintf(key, "key_%d", i);
		kvstore_set(store, key, "v", 1);
	}
	printf("Final capacity: %zu, Final size: %zu\n", store->capacity,
				 store->size);
	assert(store->capacity > 32);
	printf("Resize successful!\n\n");

	printf("----- 5. Cleaning up -----\n");
	kvstore_destroy(store);
	printf("Store destroyed.\n");

	return 0;
}
