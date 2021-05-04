/* Copyright 2021 <Bivolaru Andra> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#include "Hashtable.h"

#define MAX_BUCKET_SIZE 64
#define TRESHHOLD 0.75

/*
Comparing function for strings.
 */
int
compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

/*
 Hash function, the ones used in the laboratories.
 */
unsigned int
hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

/*
Function which allocates memory for the hashtable
and initializes the fields required by the hashtable_t structure.
*/
hashtable_t *
ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*))
{
	unsigned int i;
	hashtable_t *ht = malloc(sizeof(hashtable_t));
	DIE(ht == NULL, "Unable to allocate memory for the hashtable!\n");

	ht->buckets = malloc(hmax * sizeof(linked_list_t *));
	DIE(ht->buckets== NULL,
		"Unable to allocate memory for the hashtable's buckets!\n");

	ht->size = 0;
	ht->compare_function = compare_function;
	ht->hash_function = hash_function;

	ht->hmax = hmax;
	for (i = 0; i < ht->hmax; i++) {
		ht->buckets[i] = ll_create(sizeof(struct info));
	}


	return ht;
}

/*
Function which puts the value given in the allocated
spot of the hashtable.
*/
void
ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size) {
	if (ht == NULL) {
		printf("No list initialized!\n");
		return;
	}

	unsigned int poz = (ht->hash_function(key)) % ht->hmax;
	ll_node_t *curr = ht->buckets[poz]->head;

	while (curr != NULL &&
		   ht->compare_function(((struct info *)curr->data)->key, key) != 0) {
		curr = curr->next;
	}

	if (curr == NULL) {
		ht->size++;

		struct info new_data;
		new_data.key = malloc(key_size);
		DIE(new_data.key == NULL,
			"Unable to allocate memory for the new hashtable entry's key!\n");

		memcpy(new_data.key, key, key_size);

		new_data.value = malloc(value_size);
		DIE(new_data.value == NULL,
			"Unable to allocate memory for the new hashtable entry's value!\n");

		memcpy(new_data.value, value, value_size);

		ll_add_nth_node(ht->buckets[poz], ht->buckets[poz]->size, &new_data);
	} else {
		memcpy(((struct info *)curr->data)->value, value, value_size);
	}
}

/*
This function returns the value in the hashtable
described by the given key.
*/
char *
ht_get(hashtable_t *ht, void *key) {
	if (ht == NULL) {
		printf("No list!\n");
		return NULL;
	}

	unsigned int poz = (ht->hash_function(key)) % ht->hmax;
	ll_node_t *curr = ht->buckets[poz]->head;

	while (curr != NULL &&
		   ht->compare_function(((struct info *)curr->data)->key, key) != 0) {
		curr = curr->next;
	}

	if (curr == NULL) {
		return NULL;
	}

	return ((struct info *)curr->data)->value;
}

/*
Function which returns:
	*1 if the given key is alredy associated to a previously
put value;
	*0 on the contrary.
*/
int
ht_has_key(hashtable_t *ht, void *key) {
	if (ht == NULL) {
		printf("No list initialized!\n");
		return 0;
	}

	unsigned int poz = (ht->hash_function(key)) % ht->hmax;
	ll_node_t *curr = ht->buckets[poz]->head;

	while (curr != NULL &&
		   ht->compare_function(((struct info *)curr->data)->key, key) != 0) {
		curr = curr->next;
	}

	if (curr == NULL) {
		return 0;
	}

	return 1;
}

/*
Function which removes the value described by the given key
from the hashtable.
*/
void
ht_remove_entry(hashtable_t *ht, void *key)
{
	if (ht == NULL) {
		printf("No list initialized!\n");
		return;
	}

	unsigned int poz = (ht->hash_function(key)) % ht->hmax;

	ll_node_t *curr = ht->buckets[poz]->head;
	unsigned int entry_index = 0;

	while (curr != NULL &&
		   ht->compare_function(((struct info *)curr->data)->key, key) != 0) {
		curr = curr->next;
		entry_index++;
	}

	if (curr == NULL) {
		return;
	}

	curr = ll_remove_nth_node(ht->buckets[poz], entry_index);

	free(((struct info *)curr->data)->value);
	free(((struct info *)curr->data)->key);
	free(curr->data);
	free(curr);
}

/*
Procedure that frees all the memory allocated
for the hashtable, including all the value-key pairs.
*/
void
ht_free(hashtable_t *ht)
{
	unsigned int i, j;
	if (ht == NULL) {
		printf("No list initialized!\n");
		return;
	}

	for (i = 0; i < ht->hmax; i++) {
		unsigned int initial_size = ht->buckets[i]->size;
		for (j = 0; j < initial_size; j++) {
			ll_node_t *curr = ll_remove_nth_node(ht->buckets[i], 0);

			free(((struct info *)curr->data)->value);
			free(((struct info *)curr->data)->key);
			free(curr->data);
			free(curr);
		}
		free(ht->buckets[i]);
	}

	free(ht->buckets);
	free(ht);
}

/*
This function returns how many value-key pairs
are stored in the hashtable.
*/
unsigned int
ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

/*
Function which returns the number of buckets
allocated for the given hashtable.
*/
unsigned int
ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}

