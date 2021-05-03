/* Copyright 2021 <Bivolaru Andra> */
#ifndef LOAD_BALANCER_SKEL_HASHTABLE_H_
#define LOAD_BALANCER_SKEL_HASHTABLE_H_

#include "LinkedList.h"

struct info {
	char *key;
	char *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	linked_list_t **buckets;
	unsigned int size;
	unsigned int hmax;
	unsigned int (*hash_function)(void*);
	int (*compare_function)(void*, void*);
};

hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*));

void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

char *
ht_get(hashtable_t *ht, void *key);

int
ht_has_key(hashtable_t *ht, void *key);

void
ht_remove_entry(hashtable_t *ht, void *key);

unsigned int
ht_get_size(hashtable_t *ht);

unsigned int
ht_get_hmax(hashtable_t *ht);

void
ht_free(hashtable_t *ht);

/*
Comparing keys function:
*/
int
compare_function_strings(void *a, void *b);

/*
Hashing functions:
*/
unsigned int
hash_function_string(void *a);

#endif /* LOAD_BALANCER_SKEL_HASHTABLE_H_ */