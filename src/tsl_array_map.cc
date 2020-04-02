#include <inttypes.h>
#include <string>
#include <unordered_map>
#include <tsl/array_map.h>

#include <experimental/string_view>
template<class CharT>
struct str_hash {
    std::size_t operator()(const CharT* key, std::size_t key_size) const {
        return std::hash<std::experimental::string_view>()(std::experimental::string_view(key, key_size));
    }
};

typedef std::unordered_map<int64_t, int64_t> hash_t;
typedef tsl::array_map<char, int64_t, str_hash<char>> str_hash_t;

#include "hash_map_base.h"

#undef RESERVE_INT
#define RESERVE_INT(size) { printf("array_map can't be used for integer benchmark"); exit(6); }
#undef INSERT_INT_INTO_HASH
#define INSERT_INT_INTO_HASH(key, value) { printf("array_map can't be used for integer benchmark"); exit(6); }
#undef DELETE_INT_FROM_HASH
#define DELETE_INT_FROM_HASH(key) { printf("array_map can't be used for integer benchmark"); exit(6); }
#undef FIND_INT_EXISTING_FROM_HASH
#define FIND_INT_EXISTING_FROM_HASH(key) { printf("array_map can't be used for integer benchmark"); exit(6); }
#undef FIND_INT_MISSING_FROM_HASH
#define FIND_INT_MISSING_FROM_HASH(key) { printf("array_map can't be used for integer benchmark"); exit(6); }
#undef FIND_INT_EXISTING_FROM_HASH_COUNT
#define FIND_INT_EXISTING_FROM_HASH_COUNT(key, count) { printf("array_map can't be used for integer benchmark"); exit(6); }
#undef CHECK_INT_ITERATOR_VALUE
#define CHECK_INT_ITERATOR_VALUE(iterator, value) { printf("array_map can't be used for integer benchmark"); exit(6); }

#undef INSERT_STR_INTO_HASH
#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(key, value)

#include "template.c"
