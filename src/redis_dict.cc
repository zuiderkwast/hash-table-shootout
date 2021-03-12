#include <inttypes.h>
#include <string>

extern "C" {
#include "redis/dict.h"
}

static unsigned int IntHash(const void* key) {
  std::hash<int64_t> h;

  return h((int64_t)key);
}

static dictType IntDict = {IntHash, NULL, NULL, NULL, NULL, NULL};

#define SETUP_INT dict* hash = dictCreate(&IntDict, NULL);
#define RESERVE_INT(size) dictExpand(hash, size)
#define INSERT_INT(key, value) \
	dictAdd(hash, (void*)(key), (void*)value)

#define DELETE_INT(key) dictDelete(hash, (void*)key)

#define FIND_INT_EXISTING(key) if(dictFind(hash, (void*)key) == NULL) { printf("error"); exit(1); }
#define FIND_INT_MISSING(key) if(dictFind(hash, (void*)key) != NULL) { printf("error"); exit(1); }
#define FIND_INT_EXISTING_COUNT(key, count) if(dictFind(hash, (void*)key) != NULL) { count++; }

#define ITERATE_INT(it)
#define CHECK_INT_ITERATOR_VALUE(iterator, value)

#define LOAD_FACTOR_INT_HASH(hash) (double(dictSize(hash)) / dictSlots(hash))
#define CLEAR_INT

#include "template.cc"