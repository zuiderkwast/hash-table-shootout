#include <glib.h>

#define __UNCONST(p) ((void *)((char*)NULL+((char*)p-(char*)NULL)))

static gint intcmp(gconstpointer a, gconstpointer b)
{
	return (a < b) ? -1 : ((a > b) ? 1 : 0);
}

static gint gstrcmp(gconstpointer a, gconstpointer b)
{
	return strcmp((char*) a, (char*) b);
}

#define SETUP_INT_ GTree * hash = g_tree_new(&intcmp);
#define RESERVE_INT(size)
#define INSERT_INT_INTO_HASH(key, value) \
	g_tree_insert(hash, GINT_TO_POINTER(key), __UNCONST(&value))
#define FIND_INT_EXISTING_FROM_HASH(key) \
	if (g_tree_lookup(hash, GINT_TO_POINTER(key)) == NULL) \
		{ printf("error"); exit(1); }
#define FIND_INT_MISSING_FROM_HASH(key) \
	if (g_tree_lookup(hash, GINT_TO_POINTER(key)) != NULL) \
		{ printf("error"); exit(1); }
#define FIND_INT_EXISTING_FROM_HASH_COUNT(key, count) \
	if (g_tree_lookup(hash, GINT_TO_POINTER(key)) != NULL) \
		{ count++; }
#define DELETE_INT_FROM_HASH(key) \
	g_tree_remove(hash, GINT_TO_POINTER(key))
#define CHECK_INT_ITERATOR_VALUE(iterator, value) \
	printf("iteration for glib_tree is not implemented yet"); exit(73);
#define ITERATE_HASH(key) \
	printf("iteration for glib_tree is not implemented yet"); exit(73);
#define LOAD_FACTOR_INT_HASH(hash) (0.0f)

#define SETUP_STR GTree* str_hash = g_tree_new(&gstrcmp);
#define RESERVE_STR(size)
#define SHUFFLE_STR_ARRAY(keys)
#define INSERT_STR_INTO_HASH(key, value) \
	g_tree_insert(str_hash, __UNCONST(key.c_str()), (char*)0 + value)
#define FIND_STR_EXISTING_FROM_HASH(key) \
	if (g_tree_lookup(str_hash, __UNCONST(key.c_str())) == NULL) \
		{ printf("error 1"); exit(1); }
#define FIND_STR_MISSING_FROM_HASH(key) \
	if (g_tree_lookup(str_hash, __UNCONST(key.c_str())) != NULL) \
		{ printf("error 2"); exit(1); }
#define FIND_STR_EXISTING_FROM_HASH_COUNT(key, count) \
	if (g_tree_lookup(str_hash, __UNCONST(key.c_str())) != NULL) \
		{ count++; }
#define DELETE_STR_FROM_HASH(key) \
	g_tree_remove(str_hash, __UNCONST(key.c_str()));
#define LOAD_FACTOR_STR_HASH(hash) (0.0f)

#include "template.c"
