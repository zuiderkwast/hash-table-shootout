#include <inttypes.h>
#include <string>
#include <tsl/robin_map.h>

typedef tsl::robin_map<int64_t, int64_t, std::hash<int64_t>> hash_t;
typedef tsl::robin_map<std::string, int64_t, std::hash<std::string>> str_hash_t;

#include "hash_map_base.h"

#include "template.c"

