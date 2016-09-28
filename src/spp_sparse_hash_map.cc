#include <inttypes.h>
#include <string>
#include "sparsepp.h"
typedef spp::sparse_hash_map<int64_t, int64_t, std::hash<int64_t>, std::equal_to<int64_t>> hash_t;
typedef spp::sparse_hash_map<std::string, int64_t, std::hash<std::string>, std::equal_to<std::string>> str_hash_t;
#define SETUP hash_t hash; str_hash_t str_hash;
#define INSERT_INT_INTO_HASH(key, value) hash.insert(hash_t::value_type(key, value))
#define DELETE_INT_FROM_HASH(key) hash.erase(key);
#define FIND_FROM_INT_HASH(key) hash.find(key)->second
#define GET_VALUE_INT_FROM_KEY_VALUE(key_value) key_value.second
#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(str_hash_t::value_type(key, value))
#define DELETE_STR_FROM_HASH(key) str_hash.erase(key);
#define FIND_FROM_STR_HASH(key) str_hash.find(key)->second
#include "template.c"

