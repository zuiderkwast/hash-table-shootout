#include <inttypes.h>
#include <string>
#include <kchashdb.h>

typedef kyotocabinet::HashDB hash_t;
typedef kyotocabinet::HashDB str_hash_t;

#undef SETUP_INT
#define SETUP_INT									 \
	hash_t hash;									 \
	if (!hash.open("zzz")){							 \
		std::cerr << "error 5\n";					 \
		exit(1);									 \
	}

#undef SETUP_STR
#define SETUP_STR \
	str_hash_t str_hash; \
	str_hash.open("zzz");

#undef RESERVE_INT
#define RESERVE_INT(size)

#undef RESERVE_STR
#define RESERVE_STR(size)

#undef INSERT_INT
#define INSERT_INT(key, value) hash.set(std::to_string(key), std::to_string(value))

#undef INSERT_STR
#define INSERT_STR(key, value) str_hash.set(key, std::to_string(value))

#undef DELETE_INT
#define DELETE_INT(key) hash.remove(std::to_string(key))

#undef DELETE_STR
#define DELETE_STR(key) str_hash.remove(key)

#undef FIND_INT_EXISTING
#define FIND_INT_EXISTING(key)						 \
	{												 \
		std::string s_val;							 \
		if(!hash.get(std::to_string(key), &s_val)) { \
			std::cerr << "error 1\n";				 \
			exit(1);								 \
		}											 \
	}

#undef FIND_STR_EXISTING
#define FIND_STR_EXISTING(key)						\
	{												\
		std::string s_val;							\
		if(!str_hash.get(key, &s_val)) {			\
			std::cerr << "error 2\n";				\
			exit(2);								\
		}											\
	}

#undef FIND_INT_MISSING
#define FIND_INT_MISSING(key)						\
	{												\
		std::string s_val;							\
		if(hash.get(std::to_string(key), &s_val)) { \
			std::cerr << "error 3\n";				\
			exit(3);								\
		}											\
	}

#undef FIND_STR_MISSING
#define FIND_STR_MISSING(key)						\
	if(str_hash.get(key, &s_val)) {					\
		std::cerr << "error 4\n";					\
		exit(4);									\
	}

#undef FIND_INT_EXISTING_COUNT
#define FIND_INT_EXISTING_COUNT(key, count)			\
	if(hash.get(std::to_string(key), &s_val)) {		\
		count++;									\
	}

#undef FIND_STR_EXISTING_COUNT
#define FIND_STR_EXISTING_COUNT(key, count)			\
	if(str_hash.get(key, &s_val)) {					\
		count++;									\
	}

#undef ITERATE_HASH
#define ITERATE_HASH(it)

#undef CHECK_INT_ITERATOR_VALUE
#define CHECK_INT_ITERATOR_VALUE(it, value)

#undef LOAD_FACTOR_INT_HASH
#define LOAD_FACTOR_INT_HASH(hash) 0.0f

#undef LOAD_FACTOR_STR_HASH
#define LOAD_FACTOR_STR_HASH(hash) 0.0f

#undef CLEAR_INT
#define CLEAR_INT hash.close()

#undef CLEAR_STR
#define CLEAR_STR str_hash.close();

#include "template.c"
