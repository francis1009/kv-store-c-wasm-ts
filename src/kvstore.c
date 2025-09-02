#include "kvstore.h"

KVStore *kvstore_init(void) {
	KVStore kvstore;
	return &kvstore;
}

void kvstore_destroy(KVStore *store) {
}

KVStatus kvstore_set(KVStore *store, const char *key, const char *value) {
	return KV_SUCCESS;
}

KVStatus kvstore_delete(KVStore *store, const char *key) {
	return KV_SUCCESS;
}

const char *kvstore_get(KVStore *store, const char *key) {
	return NULL;
}
