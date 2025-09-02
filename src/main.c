#include <stdio.h>

#include "kvstore.h"

int main(void) {
	KVStore *store = kvstore_init();
	if (store == NULL) {
		fprintf(stderr, "Failed to initialize KV store.\n");
		return 1;
	}

	kvstore_set(store, "name", "Unknown");
	kvstore_set(store, "project", "kv-store-c-wasm-ts");
	printf("Project: %s\n", kvstore_get(store, "project"));

	kvstore_destroy(store);
	return 0;
}
