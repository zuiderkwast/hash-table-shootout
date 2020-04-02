#include <inttypes.h>
#include <string>
#include <google/dense_hash_map>

typedef google::dense_hash_map<int64_t, int64_t, std::hash<int64_t>> hash_t;
typedef google::dense_hash_map<std::string, int64_t, std::hash<std::string>> str_hash_t;

#include "hash_map_base.h"

#undef SETUP
#define SETUP hash_t hash; hash.max_load_factor(0.9f); hash.set_empty_key(-1); hash.set_deleted_key(-2); \
              str_hash_t str_hash; str_hash.max_load_factor(0.9f); str_hash.set_empty_key(""); str_hash.set_deleted_key("d");

#undef RESERVE_INT
#define RESERVE_INT(size) hash.resize(size)

#undef RESERVE_STR
#define RESERVE_STR(size) str_hash.resize(size) 

#include "template.c"
