#include <inttypes.h>
#include <string>
#include <flat_hash_map.hpp>

typedef ska::flat_hash_map<int64_t, int64_t, ska::power_of_two_std_hash<int64_t>> hash_t;
typedef ska::flat_hash_map<std::string, int64_t, ska::power_of_two_std_hash<std::string>> str_hash_t;

#include "hash_map_int_base.h"
#include "hash_map_str_base.h"

#include "template.c"

