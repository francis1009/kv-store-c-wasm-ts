#include <stdio.h>

#include "kvstore.h"

int main(void) {
	KVStore *store = kvstore_init();
	if (store == NULL) {
		fprintf(stderr, "Failed to initialize KV store.\n");
		return 1;
	}

	printf("----- Setting entries -----\n");
	kvstore_set(store, "name", "John");
	kvstore_set(store, "project", "kv-store-c-wasm-ts");
	printf("Name: %s\n", kvstore_get(store, "name"));
	printf("Project: %s\n", kvstore_get(store, "project"));

	printf("----- Updating entries -----\n");
	kvstore_set(store, "project", "game-of-life");
	printf("Name: %s\n", kvstore_get(store, "name"));
	printf("Project: %s\n", kvstore_get(store, "project"));

	printf("----- Deleting entries -----\n");
	kvstore_delete(store, "project");
	printf("Name: %s\n", kvstore_get(store, "name"));
	printf("Project: %s\n", kvstore_get(store, "project"));

	kvstore_destroy(store);
	return 0;
}
