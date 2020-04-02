#include <inttypes.h>
#include <string>
#include <emilib/hash_map.hpp>

typedef emilib::HashMap<int64_t, int64_t, std::hash<int64_t>> hash_t;
typedef emilib::HashMap<std::string, int64_t, std::hash<std::string>> str_hash_t;

#include "hash_map_int_base.h"
#include "hash_map_str_base.h"

#include "template.c"

