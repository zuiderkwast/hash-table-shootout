#include <inttypes.h>
#include <functional>
#include <string>

uint qHash(const std::string &key) {
    return std::hash<std::string>()(key);
}

#include <QHash>
typedef QHash<int64_t, int64_t> hash_t;
typedef QHash<std::string, int64_t> str_hash_t;

#include "hash_map_int_base.h"
#include "hash_map_str_base.h"

#undef LOAD_FACTOR_INT_HASH
#define LOAD_FACTOR_INT_HASH(hash) 0.0f
#undef INSERT_INT_INTO_HASH
#define INSERT_INT_INTO_HASH(key, value) hash.insert(key, value)
#undef DELETE_INT_FROM_HASH
#define DELETE_INT_FROM_HASH(key) hash.remove(key)
#undef CHECK_INT_ITERATOR_VALUE
#define CHECK_INT_ITERATOR_VALUE(iterator, val) \
	if(iterator != val) { printf("error"); exit(3); }

#undef LOAD_FACTOR_STR_HASH
#define LOAD_FACTOR_STR_HASH(hash) 0.0f
#undef INSERT_STR_INTO_HASH
#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(key, value)
#undef DELETE_STR_FROM_HASH
#define DELETE_STR_FROM_HASH(key) str_hash.remove(key);

#include "template.c"
