#include <inttypes.h>
#include <string>
#include <Judy.h>

#define __UNCONST(p) ((void *)((char*)NULL+((char*)p-(char*)NULL)))

#define SETUP_INT void *hash_int = NULL;

#define RESERVE_INT(size)
#define INSERT_INT_INTO_HASH(key, value) *(Word_t *)JudyLIns(&hash_int, key, NULL) = value;
#define DELETE_INT_FROM_HASH(key) JudyLDel(&hash_int, key, NULL);
#define FIND_INT_EXISTING_FROM_HASH(key) if (JudyLGet(hash_int, key, NULL) == NULL) { printf("error"); exit(1); }
#define FIND_INT_MISSING_FROM_HASH(key) if (JudyLGet(hash_int, key, NULL) != NULL) { printf("error"); exit(1); }
#define FIND_INT_EXISTING_FROM_HASH_COUNT(key, count) if (JudyLGet(hash_int, key, NULL) != NULL) { count++; }
#define CHECK_INT_ITERATOR_VALUE(iterator, value)
#define ITERATE_HASH(key) \
	Word_t key = 0; \
	Word_t *p_value = (Word_t *)JudyLFirst(hash_int, &key, NULL); \
	for (; p_value != NULL; p_value = (Word_t *)JudyLNext(hash_int, &key, NULL))

#define LOAD_FACTOR_INT_HASH(hash) (0.0f)

#include "template.c"
