#include <inttypes.h>
#include <string>
#include <nata.h>

#define __UNCONST(p) ((void *)((char*)NULL+((char*)p-(char*)NULL)))

#define SETUP_STR void *str_hash = NULL;

#define RESERVE_STR(size)
#define INSERT_STR_INTO_HASH(key, value) *nataF8ins(&str_hash, __UNCONST(key.c_str()), key.size(), NULL) = value;
#define DELETE_STR_FROM_HASH(key) nataF8del(&str_hash, __UNCONST(key.c_str()), key.size(), NULL);
#define FIND_STR_EXISTING_FROM_HASH(key) if (nataF8get(str_hash, __UNCONST(key.c_str()), key.size()) == NULL) { printf("error"); exit(1); }
#define FIND_STR_MISSING_FROM_HASH(key) if (nataF8get(str_hash, __UNCONST(key.c_str()), key.size()) != NULL) { printf("error"); exit(1); }
#define FIND_STR_EXISTING_FROM_HASH_COUNT(key, count) if (nataF8get(str_hash, __UNCONST(key.c_str()), key.size()) != NULL) { count++; }

#define LOAD_FACTOR_STR_HASH(hash) (0.0f)

#include "template.c"
