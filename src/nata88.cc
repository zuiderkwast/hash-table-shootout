#include <inttypes.h>
#include <string>
#include <nata.h>

#define __UNCONST(p) ((void *)((char*)NULL+((char*)p-(char*)NULL)))

#define SETUP_INT void *hash_int = NULL;

#define RESERVE_INT(size)
#define INSERT_INT_INTO_HASH(key, value) *nata88ins(&hash_int, key, NULL) = value;
#define DELETE_INT_FROM_HASH(key) //nata88del(&hash_int, key, NULL);
#define FIND_INT_EXISTING_FROM_HASH(key) if (nata88get(hash_int, key, NULL) == NULL) { printf("error"); exit(1); }
#define FIND_INT_MISSING_FROM_HASH(key) if (nata88get(hash_int, key, NULL) != NULL) { printf("error"); exit(1); }
#define FIND_INT_EXISTING_FROM_HASH_COUNT(key, count) if (nata88get(hash_int, key, NULL) != NULL) { count++; }
#define CHECK_INT_ITERATOR_VALUE(iterator, value)
#define ITERATE_HASH(key)

#define LOAD_FACTOR_INT_HASH(hash) (0.0f)

#include "template.c"
