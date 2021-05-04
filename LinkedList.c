/* Copyright 2021 <Bivolaru Andra> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LinkedList.h"
#include "utils.h"

/* Function which initializez the linked list,
allocating the necessary memory and initializing
the fields of the linked_list_t structure. */
linked_list_t*
ll_create(unsigned int data_size)
{
    linked_list_t *list = malloc(sizeof(linked_list_t));

    DIE(list == NULL, "Unable to allocate memory for the linked list!\n");

    list->data_size = data_size;
    list->head = NULL;
    list->size = 0;

    return list;
}

/* This function creates a new node and adds it on the n-th position
in the given linked list; the positions start from the 0 index.
If the given n is smaller than 0 (n < 0), than an error occurs, but
if n is larger that the number of nodes in the linked list, than
the data is stored on the last position of the list. */
void
ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
    ll_node_t *new;
    ll_node_t *element = NULL;
    unsigned int i;

    if (list == NULL) {
        printf("No list initialized!\n");
        return;
    }

    new = malloc(sizeof(ll_node_t));

    DIE(new == NULL, "Unable to allocate memory for the linked list node!\n");
    new->data = malloc(list->data_size);
    DIE(new->data == NULL, "Unable to allocate memory for the nodes's data!\n");

    element = list->head;

    if (n >= list->size) {
        n = list->size;
    }

    if (n < 0) {
        printf("Wrong selected position, please try another value\n");
        return;
    }

    if (n == 0) {
        memcpy(new->data, new_data, list->data_size);
        new->next = list->head;
        list->head = new;

        list->size++;
        return;
    }

    for (i = 0; i < n - 1; i++) {
        element = element->next;
    }

    memcpy(new->data, new_data, list->data_size);
    new->next = element->next;
    element->next = new;

    list->size++;
}

/* Function which returns the n-th entry from the given list.
The same conditions regarding the n index as in the
ll_add_nth_node() function. */
ll_node_t *
ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
    ll_node_t *element = NULL;
    ll_node_t *free_pointer = NULL;
    unsigned int i;

    if (list == NULL) {
        printf("No list initialized!\n");
        return NULL;
    }

    if (n > list->size) {
        n = list->size - 1;
    }

    if (n < 0) {
        printf("Wrong selected position, please try another value\n");
        return NULL;
    }

    if (n == 0) {
        free_pointer = list->head;
        list->head = free_pointer->next;

        list->size--;
        return free_pointer;
    }

    element = list->head;
    for (i = 0; i < n - 1; i++) {
        element = element->next;
    }

    free_pointer = element->next;
    element->next = element->next->next;
    list->size--;

    return free_pointer;
}

/* Function which frees all the nodes in the list, as
well as the list itself in the end. */
void
ll_free(linked_list_t** pp_list)
{
    ll_node_t *element;
    DIE(pp_list == NULL, "No list initialized!\n");
    DIE(*pp_list == NULL, "No list initialized!\n");

    element = (*pp_list)->head;
    while ((*pp_list)->head != NULL) {
        (*pp_list)->head = (*pp_list)->head->next;

        free(element->data);
        free(element);
        element = (*pp_list)->head;
    }

    free(*pp_list);
}
