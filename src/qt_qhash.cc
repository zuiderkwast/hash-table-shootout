#include <inttypes.h>
#include <functional>
#include <string>

uint qHash(const std::string &key) {
    return std::hash<std::string>()(key);
}

#include <QHash>
typedef QHash<int64_t, int64_t> hash_t;
typedef QHash<std::string, int64_t> str_hash_t;

#define SETUP hash_t hash; str_hash_t str_hash;

#define INSERT_INT_INTO_HASH(key, value) hash.insert(key, value)
#define DELETE_INT_FROM_HASH(key) hash.remove(key)
#define FIND_INT_EXISTING_FROM_HASH(key) if(hash.find(key) == hash.end()) { printf("error"); exit(1); }
#define FIND_INT_MISSING_FROM_HASH(key) if(hash.find(key) != hash.end()) { printf("error"); exit(2); }
#define CHECK_INT_ITERATOR_VALUE(iterator, value) if(iterator.value() != value) { printf("error"); exit(3); }

#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(key, value)
#define DELETE_STR_FROM_HASH(key) str_hash.remove(key);
#define FIND_STR_EXISTING_FROM_HASH(key) if(str_hash.find(key) == str_hash.end()) { printf("error"); exit(4); }


#include "template.c"
