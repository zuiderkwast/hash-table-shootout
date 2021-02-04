#include <khash.h>

#define __UNCONST(p) ((void *)((char*)NULL+((const char*)p-(const char*)NULL)))

KHASH_MAP_INIT_INT64(kh64, int64_t)
KHASH_MAP_INIT_STR(khstr, int64_t)

#define SETUP_INT \
	khash_t(kh64) *hash = kh_init(kh64);
#define SETUP_STR \
	khash_t(khstr) *str_hash = kh_init(khstr);
#define RESERVE_INT(n)
#define RESERVE_STR(n)
#define INSERT_INT(key, value)											\
	int r;																\
	khint_t k = kh_put(kh64, hash, key, &r);							\
	if (r > 0) {														\
		kh_value(hash, k) = value;										\
	}
#define INSERT_STR(key, value)											\
	int r;																\
	char *istr = strdup(key.c_str());									\
	khint_t k = kh_put(khstr, str_hash, istr, &r);						\
	if (r > 0) {														\
		kh_value(str_hash, k) = value;									\
	} else																\
		free(istr);
#define DELETE_INT(key)							\
	khint_t k = kh_get(kh64, hash, key);		\
	if (k != kh_end(hash))						\
		kh_del(kh64, hash, k);
#define DELETE_STR(key)													\
	khint_t k = kh_get(khstr, str_hash, key.c_str());					\
	if (k != kh_end(str_hash)) {										\
		char *istr = (char *) kh_key(str_hash, k);						\
		kh_del(khstr, str_hash, k);										\
		free(istr);														\
	}
#define FIND_INT_EXISTING(key)											\
	khint_t k = kh_get(kh64, hash, (key));								\
	if (k == kh_end(hash)) {											\
		std::cerr << "error 1\n";										\
		exit(1);														\
	}
#define FIND_STR_EXISTING(key)											\
	khint_t k = kh_get(khstr, str_hash, (key.c_str()));					\
	if (k == kh_end(str_hash)) {										\
		std::cerr << "error 2\n";										\
		exit(1);														\
	}
#define FIND_INT_MISSING(key)											\
	khint_t k = kh_get(kh64, hash, key);								\
	if (k != kh_end(hash)) {											\
		std::cerr << "error 3\n";										\
		exit(1);														\
	}
#define FIND_STR_MISSING(key)											\
	khint_t k = kh_get(khstr, str_hash, key.c_str());					\
	if (k != kh_end(str_hash)) {										\
		std::cerr << "error 4\n";										\
		exit(1);														\
	}
#define FIND_INT_EXISTING_COUNT(key, count)								\
	khint_t k = kh_get(kh64, hash, key);								\
	if (k == kh_end(hash)) {											\
		++count;														\
	}
#define FIND_STR_EXISTING_COUNT(key, count)								\
	khint_t k = kh_get(khstr, str_hash, key.c_str());					\
	if (k == kh_end(str_hash)) {										\
		++count;														\
	}
#define ITERATE_INT(key)												\
	khiter_t it;														\
	for (it = kh_begin(hash); it != kh_end(hash); ++it){				\
		if (kh_exist(hash, it)){										\
			kh_value(hash, it) = 1;										\
		}																\
	}
#define CHECK_INT_ITERATOR_VALUE(it, value)

#define LOAD_FACTOR_INT_HASH(h) 0.0f
#define LOAD_FACTOR_STR_HASH(h) 0.0f
#define CLEAR_INT
#define CLEAR_STR

#include "template.c"
