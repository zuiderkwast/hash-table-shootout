#define SETUP_INT hash_t hash;
#define RESERVE_INT(size) hash.reserve(size)
#define LOAD_FACTOR_INT_HASH(hash) hash.load_factor()
#define INSERT_INT_INTO_HASH(key, value) hash.insert(hash_t::value_type(key, value))
#define DELETE_INT_FROM_HASH(key) hash.erase(key)
#define FIND_INT_EXISTING_FROM_HASH(key) if(hash.find(key) == hash.end()) { printf("error"); exit(1); }
#define FIND_INT_MISSING_FROM_HASH(key) if(hash.find(key) != hash.end()) { printf("error"); exit(1); }
#define FIND_INT_EXISTING_FROM_HASH_COUNT(key, count) if(hash.find(key) != hash.end()) { count++; }
#define CHECK_INT_ITERATOR_VALUE(iterator, value) if(iterator.second != value) { printf("error"); exit(1); }
