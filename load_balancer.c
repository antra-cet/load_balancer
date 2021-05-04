/* Copyright 2021 <Bivolaru Andra> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "Hashtable.h"

struct load_balancer {
	server_memory **servers;
    long long *hash_ring;

    unsigned int num_servers;
    unsigned int num_hash_ring_servers;
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *) a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}


load_balancer* init_load_balancer() {
    // Initializing memory for the new load balancer
	load_balancer *new_main = malloc(sizeof(load_balancer));
    DIE(new_main == NULL, "Memory error from the load balancer!\n");

    // Initializing memory for the servers and setting them on NULL
    new_main->servers = malloc(E_5 * sizeof(server_memory *));
    DIE(new_main->servers == NULL, "Memory error from the servers!\n");
    for (unsigned int i = 0; i < E_5; i++) {
        new_main->servers[i] = NULL;
    }

    // Initializing memory fot the hash ring array
    new_main->hash_ring = calloc(E_5 * 3, sizeof(long long));
    DIE(new_main->hash_ring == NULL, "Memory error of the hash ring!\n");

    /* Setting on zero the number of servers and elements
    on the hash ring */
    new_main->num_servers = 0;
    new_main->num_hash_ring_servers = 0;

    return new_main;
}

char *init_curr_key(ll_node_t *curr) {
    /* Initializing memory for the current key that
    is being used and equaling it to the string*/
    char *curr_key;
    curr_key = malloc(sizeof(char) *
                      (strlen(((struct info *)curr->data)->key)) + 1);
    DIE(curr_key == NULL, "Memory error from the key name!\n");
    memcpy(curr_key, ((struct info *)curr->data)->key,
           strlen(((struct info *)curr->data)->key) + 1);

    return curr_key;
}

char *init_curr_value(ll_node_t *curr) {
    /* Initializing memory for the current value that
    is being used and equaling it to the string*/
    char *curr_value;
    curr_value = malloc(sizeof(char) *
                      (strlen(((struct info *)curr->data)->value)) + 1);
    DIE(curr_value == NULL, "Memory error from the value name!\n");
    memcpy(curr_value, ((struct info *)curr->data)->value,
           strlen(((struct info *)curr->data)->value) + 1);

    return curr_value;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id) {
	unsigned int key_hash = hash_function_key(key);

    // Finding which server the value should be placed on
    unsigned int i;
    for (i = 0; i < main->num_hash_ring_servers; i++) {
        if (key_hash < hash_function_servers(&main->hash_ring[i])) {
            break;
        }
    }
    i = i % main->num_hash_ring_servers;

    *server_id = main->hash_ring[i] % E_5;

    // Placing the value in the found server and changing the server_id
    server_store(main->servers[*server_id], key, value);
}


char* loader_retrieve(load_balancer* main, char* key, int* server_id) {
    unsigned int key_hash = hash_function_key(key);
    unsigned int i;

    // Finding which server the value was placed
    for (i = 0; i < main->num_hash_ring_servers; i++) {
        if (key_hash < hash_function_servers(&main->hash_ring[i])) {
            break;
        }
    }
    i = i % main->num_hash_ring_servers;

    // Retriving the value from the found server
    *server_id = main->hash_ring[i] % E_5;
    return server_retrieve(main->servers[*server_id], key);
}

void add_new_server(load_balancer *main, long long label) {
    int pos_server1, pos_server2;
    int hash_ring_pos1, hash_ring_pos2;
    unsigned int i;

    // Finding the new added server
    for (i = 0; i < main->num_hash_ring_servers; i++) {
        if (label == main->hash_ring[i]) {
            break;
        }
    }
    // Initializing the hash ring positions of the two servers
    hash_ring_pos1 = i;
    hash_ring_pos2 = (i + 1) % main->num_hash_ring_servers;

    // Calculating the server's positions in the server array
    pos_server1 = main->hash_ring[hash_ring_pos1] % E_5;
    pos_server2 = main->hash_ring[hash_ring_pos2] % E_5;

    // Initializing server 2
    server_memory *server2;
    server2 = main->servers[pos_server2];

    /* Verifying if the servers have consecutive positions
    in the hash ring */
    if (pos_server1 != pos_server2) {
        /* If they don't all the values are verified if
        their position in the server is correct based on
        the add of another server */
        for (unsigned int i = 0; i < server2->ht->hmax; i++) {
            ll_node_t *curr = server2->ht->buckets[i]->head;

            while (curr != NULL) {
                int new_server_id = -1;
                char *curr_key = init_curr_key(curr);
                char *curr_value = init_curr_value(curr);
                curr = curr->next;

                // Placing the value again in the load balancer
                loader_store(main, curr_key, curr_value, &new_server_id);

                /* If the value isn't placed on the same server then
                it is deleted from server2 */
                if (new_server_id != pos_server2) {
                    server_remove(server2, curr_key);
                }

                free(curr_key);
                free(curr_value);
            }
        }
    }
}

void hash_ring_add_server(load_balancer *main, long long label) {
    unsigned int position_hr;
    unsigned int i = 0;

    // Searching for the place to put the label in the hash ring
    for (i = 0; i < main->num_hash_ring_servers; i++) {
        if (hash_function_servers(&label) <
            hash_function_servers(&main->hash_ring[i])) {
            break;
        }
    }
    position_hr = i;

    // Placing the value label in the hash ring and keeping its values in order
    for (unsigned int j = main->num_hash_ring_servers; j > position_hr; j--) {
        main->hash_ring[j] = main->hash_ring[j - 1];
    }
    main->hash_ring[position_hr] = label;

    main->num_hash_ring_servers++;
}

void loader_add_server(load_balancer* main, int server_id) {
	main->servers[server_id] = init_server_memory();

    // Placing all the labels in the hash ring
    for (unsigned int copy = 0; copy < 3; copy++) {
        long long label = E_5 * copy + server_id;
        hash_ring_add_server(main, label);
    }

    /* Adding the server in the load balancer and
    replacing the values in the servers */
    for (unsigned int copy = 0; copy < 3; copy++) {
        long long label = E_5 * copy + server_id;
        add_new_server(main, label);
    }

    main->num_servers++;
}

void delete_server(load_balancer *main, int server_id) {
    server_memory *delete_server = main->servers[server_id];

    /* Replacing all the values of the deleted server back in
    the load balancer */
    for (unsigned int i = 0; i < delete_server->ht->hmax; i++) {
        ll_node_t *curr = delete_server->ht->buckets[i]->head;

        while (curr != NULL) {
            int new_server_id = -1;
            char *curr_key = init_curr_key(curr);
            char *curr_value = init_curr_value(curr);
            curr = curr->next;

            loader_store(main, curr_key, curr_value, &new_server_id);

            free(curr_key);
            free(curr_value);
        }
    }
}

void hash_ring_remove_server(load_balancer *main, long long label) {
    unsigned int i;

    // Searching for the label in the hash ring
    for (i = 0; i < main->num_hash_ring_servers; i++) {
        if (label == main->hash_ring[i]) {
            break;
        }
    }

    // Removing the i-th position of the hash ring
    unsigned int position_hr = i;
    for (unsigned int j = position_hr ;
         j < main->num_hash_ring_servers - 1; j++) {
        main->hash_ring[j] = main->hash_ring[j + 1];
    }
    main->num_hash_ring_servers--;
}

void loader_remove_server(load_balancer* main, int server_id) {
    /* Calculating each label of the server id and removing
    it from the hash ring */
    for (unsigned int copy = 0; copy < 3; copy++) {
        long long label = E_5 * copy + server_id;
        hash_ring_remove_server(main, label);
    }

    // Replacing the values of the server in other servers
    delete_server(main, server_id);
	free_server_memory(main->servers[server_id]);
    main->servers[server_id] = NULL;
    main->num_servers--;
}

void free_load_balancer(load_balancer* main) {
    if (main == NULL) {
        return;
    }

    // Freeing the used memory from the servers
    for (int i = 0; i < E_5; i++) {
        if (main->servers[i] != NULL) {
            free_server_memory(main->servers[i]);
        }
    }

    free(main->servers);
    free(main->hash_ring);
    free(main);
}
