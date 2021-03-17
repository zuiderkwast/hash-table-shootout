#include <inttypes.h>
#include <string>
#include <string.h>

#include "dash/dash.h"

extern "C" {
#include "redis/sds.h"
}

static unsigned long IntHash(uint64_t key) {
  std::hash<int64_t> h;

  return h(key);
}


static unsigned long StrHash(uint64_t key) {
	sds s = (sds)key;
	std::size_t hi = std::_Hash_impl::hash(s, sdslen(s));

	return hi;
}

static bool dictSdsKeyCompare(uint64_t key1, uint64_t key2) {
  sds k1 = (sds)key1;
  sds k2 = (sds)key2;
  int l1 = sdslen(k1);
  int l2 = sdslen(k2);
  if (l1 != l2)
    return false;
  return memcmp(k1, k2, l1) == 0;
}

#define SETUP_INT base::DashTable hash
                  // hash.set_hash_fn(&IntHash)

#define RESERVE_INT(size) hash.Reserve(size)
#define INSERT_INT(key, value) hash.Insert(key, value)

#define DELETE_INT(key) hash.Delete(key)

#define FIND_INT_EXISTING(key)  \
    if(!hash.Find(key)) { printf("error"); exit(1); }

#define FIND_INT_MISSING(key)  \
    if(hash.Find(key)) { printf("error"); exit(1); }


#define FIND_INT_EXISTING_COUNT(key, count)  \
    if(hash.Find(key)) { count++; }

#define ITERATE_INT(it)
#define CHECK_INT_ITERATOR_VALUE(iterator, value)

#define LOAD_FACTOR_INT_HASH(hash) (hash.load_factor())
#define CLEAR_INT

#define SETUP_STR base::DashTable str_hash; \
                  str_hash.set_hash_fn(&StrHash); \
                  str_hash.set_cmp(&dictSdsKeyCompare); \
                  str_hash.set_key_dtor([](auto k) { sdsfree((sds)k); }); \
                  sds sk = sdsempty();

#define RESERVE_STR(size)

#define LOAD_FACTOR_STR_HASH(str_hash)  (str_hash.load_factor())

#define INSERT_STR(key, value) \
  str_hash.Insert((uint64_t)sdsnewlen(key.data(), key.size()), value)

#define DELETE_STR(key) sk = sdscpylen(sk, key.data(), key.size()); \
    str_hash.Delete((uint64_t)sk)


#define FIND_STR_EXISTING(key) \
  sk = sdscpylen(sk, key.data(), key.size()); \
	if(!str_hash.Find((uint64_t)sk)) { std::cerr << "error\n"; exit(4); }

#define FIND_STR_MISSING(key) \
  sk = sdscpylen(sk, key.data(), key.size()); \
	if(str_hash.Find((uint64_t)sk)) { std::cerr << "error\n"; exit(5); }


#define FIND_STR_EXISTING_COUNT(key, count) \
  sk = sdscpylen(sk, key.data(), key.size()); \
	if(str_hash.Find((uint64_t)sk)) { count++; }


#define CLEAR_STR

#include "template.cc"