#include <inttypes.h>
#include <string>
#include <string.h>

#include "dash/dash.h"

static unsigned long IntHash(uint64_t key) {
  std::hash<int64_t> h;

  return h(key);
}

#define SETUP_INT base::DashTable hash
                  // hash.set_hash_fn(&IntHash)

#define RESERVE_INT(size) hash.Reserve(size)
#define INSERT_INT(key, value) hash.Insert(key, value)

#define DELETE_INT(key) hash.Delete(key)

#define FIND_INT_EXISTING(key) uint64_t rval; \
    if(!hash.Find(key, &rval)) { printf("error"); exit(1); }

#define FIND_INT_MISSING(key) uint64_t rval; \
    if(hash.Find(key, &rval)) { printf("error"); exit(1); }


#define FIND_INT_EXISTING_COUNT(key, count) uint64_t rval; \
    if(hash.Find(key, &rval)) { count++; }

#define ITERATE_INT(it)
#define CHECK_INT_ITERATOR_VALUE(iterator, value)

#define LOAD_FACTOR_INT_HASH(hash) (hash.load_factor())
#define CLEAR_INT

#if 0
#define SETUP_STR dict* str_hash = dictCreate(&StrDict, NULL); \
                  sds sk = sdsempty();

#define RESERVE_STR(size) dictExpand(str_hash, size)

#define LOAD_FACTOR_STR_HASH(str_hash)  (double(dictSize(str_hash)) / dictSlots(str_hash))

#define INSERT_STR(key, value) \
	dictAdd(str_hash, sdsnewlen(key.data(), key.size()), (void*)value)


#define DELETE_STR(key) sk = sdscpylen(sk, key.data(), key.size()); dictDelete(str_hash, sk);

#define FIND_STR_EXISTING(key) \
  sk = sdscpylen(sk, key.data(), key.size()); \
	if(dictFind(str_hash, sk) == NULL) { std::cerr << "error\n"; exit(4); }

#define FIND_STR_MISSING(key) \
	sk = sdscpylen(sk, key.data(), key.size()); \
	if(dictFind(str_hash, sk) != NULL) { std::cerr << "error\n"; exit(5); }


#define FIND_STR_EXISTING_COUNT(key, count) \
	sk = sdscpylen(sk, key.data(), key.size()); \
	if(dictFind(str_hash, sk) != NULL) { count++; }


#define CLEAR_STR
#endif

#include "template.cc"