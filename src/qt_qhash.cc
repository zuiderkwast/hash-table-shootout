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
#define FIND_FROM_INT_HASH(key) hash.find(key).value()
#define GET_VALUE_INT_FROM_KEY_VALUE(key_value) key_value
#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(key, value)
#define DELETE_STR_FROM_HASH(key) str_hash.remove(key)
#define FIND_FROM_STR_HASH(key) str_hash.find(key).value()
#include "template.c"
