#define SETUP_STR str_hash_t str_hash;
#define RESERVE_STR(size) str_hash.reserve(size) 
#define LOAD_FACTOR_STR_HASH(str_hash) str_hash.load_factor()
#define INSERT_STR_INTO_HASH(key, value) str_hash.insert(str_hash_t::value_type(key, value))
#define DELETE_STR_FROM_HASH(key) str_hash.erase(key);
#define FIND_STR_EXISTING_FROM_HASH(key) if(str_hash.find(key) == str_hash.end()) { printf("error"); exit(4); }
#define FIND_STR_MISSING_FROM_HASH(key) if(str_hash.find(key) != str_hash.end()) { printf("error"); exit(5); }
#define FIND_STR_EXISTING_FROM_HASH_COUNT(key, count) if(str_hash.find(key) != str_hash.end()) { count++; }
