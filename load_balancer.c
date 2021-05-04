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
	load_balancer *new_main = malloc(sizeof(load_balancer));
    DIE(new_main == NULL,
        "Unable to allocate memory for the main load balancer!\n");

    new_main->servers = malloc(E_5 * sizeof(server_memory *));
    DIE(new_main->servers == NULL,
        "Unable to allocate memory for the servers!\n");
    for (unsigned int i = 0; i < E_5; i++) {
        new_main->servers[i] = NULL;
    }

    new_main->hash_ring = calloc(E_5 * 3, sizeof(long long));
    DIE(new_main->hash_ring == NULL,
        "Unable to allocate memory for the hash ring!\n");

    new_main->num_servers = 0;
    new_main->num_hash_ring_servers = 0;

    return new_main;
}

void readjust_values_server1(unsigned int hash0, server_memory *server1,
                             unsigned int hash1, server_memory *server2) {
    for (unsigned int i = 0; i < server2->ht->hmax; i++) {
       ll_node_t *curr = server2->ht->buckets[i]->head;

       while (curr != NULL) {
            char *curr_key;
            curr_key = malloc(sizeof(char) *
                              (strlen(((struct info *)curr->data)->key)) + 1);
            DIE(curr_key == NULL,
                "Unable to allocate memory for the key name!\n");
            memcpy(curr_key, ((struct info *)curr->data)->key,
                   strlen(((struct info *)curr->data)->key) + 1);

            char *curr_value;
            curr_value = malloc(sizeof(char) *
                                (strlen((((struct info *)curr->data)->value))
                                + 1));
            DIE(curr_value == NULL,
                "Unable to allocate memory for the value name!\n");
            memcpy(curr_value, ((struct info *)curr->data)->value,
                   strlen(((struct info *)curr->data)->value) + 1);

            unsigned int curr_key_hash;
            curr_key_hash = hash_function_key(((struct info *)curr->data)->key);

            if (curr_key_hash > hash0 && curr_key_hash < hash1) {
               server_store(server1, curr_key, curr_value);
            }

            curr = curr->next;
            free(curr_value);
            free(curr_key);
        }
    }
}

void readjust_values_server2(server_memory *server1, server_memory *server2) {
    for (unsigned int i = 0; i < server1->ht->hmax; i++) {
       ll_node_t *curr = server1->ht->buckets[i]->head;

       while (curr != NULL) {
            char *curr_key;
            curr_key = malloc(sizeof(char) *
                              (strlen(((struct info *)curr->data)->key)) + 1);
            DIE(curr_key == NULL,
                "Unable to allocate memory for the key name!\n");
            memcpy(curr_key, ((struct info *)curr->data)->key,
                   strlen(((struct info *)curr->data)->key) + 1);

            if (server_retrieve(server2, curr_key) != NULL) {
                server_remove(server2, curr_key);
            }

            curr = curr->next;
            free(curr_key);
        }
    }
}

void add_new_server(load_balancer *main, long long label) {
    unsigned int pos_server1, pos_server2;
    unsigned int i;

    for (i = 0; i < main->num_hash_ring_servers; i++) {
        if (label == main->hash_ring[i]) {
            break;
        }
    }
    pos_server1 = i;
    pos_server2 = (pos_server1 + 1) % main->num_hash_ring_servers;
    server_memory *server2;

    server2 = main->servers[main->hash_ring[pos_server2] % E_5];

    if (main->hash_ring[pos_server1] % E_5 !=
        main->hash_ring[pos_server2] % E_5) {
        for (unsigned int i = 0; i < server2->ht->hmax; i++) {
            ll_node_t *curr = server2->ht->buckets[i]->head;

            while (curr != NULL) {
                int new_server_id = -1;
                char *curr_key;
                curr_key = malloc(sizeof(char) *
                                (strlen(((struct info *)curr->data)->key)) + 1);
                DIE(curr_key == NULL,
                    "Unable to allocate memory for the key name!\n");
                memcpy(curr_key, ((struct info *)curr->data)->key,
                    strlen(((struct info *)curr->data)->key) + 1);

                char *curr_value;
                curr_value = malloc(sizeof(char) *
                                    (strlen(((struct info *)curr->data)->value))
                                    + 1);
                DIE(curr_value == NULL,
                    "Unable to allocate memory for the value name!\n");
                memcpy(curr_value, ((struct info *)curr->data)->value,
                    strlen(((struct info *)curr->data)->value) + 1);
                curr = curr->next;

                loader_store(main, curr_key, curr_value, &new_server_id);

                if (new_server_id != main->hash_ring[pos_server2] % E_5) {
                    server_remove(server2, curr_key);
                }

                free(curr_key);
                free(curr_value);
            }
        }
    }
}

void delete_server(load_balancer *main, int server_id) {
    server_memory *delete_server = main->servers[server_id];

    for (unsigned int i = 0; i < delete_server->ht->hmax; i++) {
        ll_node_t *curr = delete_server->ht->buckets[i]->head;

        while (curr != NULL) {
            int new_server_id = -1;
            char *curr_key;
            curr_key = malloc(sizeof(char) *
                              (strlen(((struct info *)curr->data)->key)) + 1);
            DIE(curr_key == NULL,
                "Unable to allocate memory for the key name!\n");
            memcpy(curr_key, ((struct info *)curr->data)->key,
                   strlen(((struct info *)curr->data)->key) + 1);

            char *curr_value;
            curr_value = malloc(sizeof(char) *
                              (strlen(((struct info *)curr->data)->value)) + 1);
            DIE(curr_value == NULL,
                "Unable to allocate memory for the value name!\n");
            memcpy(curr_value, ((struct info *)curr->data)->value,
                   strlen(((struct info *)curr->data)->value) + 1);

            loader_store(main, curr_key, curr_value, &new_server_id);

            curr = curr->next;
            free(curr_key);
            free(curr_value);
        }
    }
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
    if (i == main->num_hash_ring_servers) {
        i = 0;
    }

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

    // If the key is bigger that all the other keys,
    // than it is found in the first server in the hash ring (position 0)
    if (i == main->num_hash_ring_servers) {
        i = 0;
    }

    // Retriving the value from the found server
    *server_id = main->hash_ring[i] % E_5;
    return server_retrieve(main->servers[*server_id], key);
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

    for (unsigned int copy = 0; copy < 3; copy++) {
        long long label = E_5 * copy + server_id;
        hash_ring_add_server(main, label);
    }

    for (unsigned int copy = 0; copy < 3; copy++) {
        long long label = E_5 * copy + server_id;
        add_new_server(main, label);
    }

    main->num_servers++;
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
    for (unsigned int copy = 0; copy < 3; copy++) {
        long long label = E_5 * copy + server_id;
        hash_ring_remove_server(main, label);
    }

    delete_server(main, server_id);
	free_server_memory(main->servers[server_id]);
    main->servers[server_id] = NULL;
    main->num_servers--;
}

void free_load_balancer(load_balancer* main) {
    if (main == NULL) {
        return;
    }

    for (int i = 0; i < E_5; i++) {
        if (main->servers[i] != NULL) {
            free_server_memory(main->servers[i]);
        }
    }

    free(main->servers);
    free(main->hash_ring);
    free(main);
}
