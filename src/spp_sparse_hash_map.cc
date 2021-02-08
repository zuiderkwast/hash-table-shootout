#include <inttypes.h>
#include <string>
#include <sparsepp/spp.h>

typedef spp::sparse_hash_map<int64_t, int64_t, std::hash<int64_t>, std::equal_to<int64_t>> hash_t;
typedef spp::sparse_hash_map<std::string, int64_t, std::hash<std::string>, std::equal_to<std::string>> str_hash_t;

#include "hash_map_int_base.h"
#include "hash_map_str_base.h"

#include "template.cc"

