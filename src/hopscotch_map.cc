#include <inttypes.h>
#include <string>
#include "hopscotch_map.h"

typedef hopscotch_map<int64_t, int64_t, std::hash<int64_t>, std::equal_to<int64_t>> hash_t;
typedef hopscotch_map<std::string, int64_t, std::hash<std::string>, std::equal_to<std::string>> str_hash_t;

#define SETUP hash_t hash; str_hash_t str_hash;

#define INSERT_INT_INTO_HASH(key, value) hash.insert(hash_t::value_type(key, value))
#define DELETE_INT_FROM_HASH(key) hash.erase(key)
#define FIND_INT_EXISTING_FROM_HASH(key) if(hash.find(key) == hash.end()) { printf("error"); exit(1); }
#define FIND_INT_MISSING_FROM_HASH(key) if(hash.find(key) != hash.end()) { printf("error"); exit(2); }
#define CHECK_INT_ITERATOR_VALUE(iterator, value) if(iterator->second != value) { printf("error"); exit(3); }

#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(str_hash_t::value_type(key, value))
#define DELETE_STR_FROM_HASH(key) str_hash.erase(key);
#define FIND_STR_EXISTING_FROM_HASH(key) if(str_hash.find(key) == str_hash.end()) { printf("error"); exit(4); }

#include "template.c"

