/* Copyright 2021 <Bivolaru Andra> */
#ifndef LOAD_BALANCER_SKEL_LOAD_BALANCER_H_
#define LOAD_BALANCER_SKEL_LOAD_BALANCER_H_

#include "server.h"

struct load_balancer;
typedef struct load_balancer load_balancer;

load_balancer* init_load_balancer();

/*
Function which moves the values from server2 to server1
*/
void readjust_values_server1(unsigned int hash0, server_memory *server1,
                     unsigned int hash1, server_memory *server2);
void readjust_values_server2(server_memory *server1, server_memory *server2);
/* This function adds a new server in the hash ring and
readjust the values stored on the servers */
void add_new_server(load_balancer *main, long long label);
/* This function deletes a server in the hash ring and
readjust the values stored on the servers */
void delete_server(load_balancer *main, int server_id);


void free_load_balancer(load_balancer* main);

/**
 * load_store() - Stores the key-value pair inside the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 * @arg4: This function will RETURN via this parameter 
 *        the server ID which stores the object.
 *
 * The load balancer will use Consistent Hashing to distribute the 
 * load across the servers. The chosen server ID will be returned 
 * using the last parameter.
 */
void loader_store(load_balancer* main, char* key, char* value, int* server_id);

/**
 * load_retrieve() - Gets a value associated with the key.
 * @arg1: Load balancer which distributes the work.
 * @arg2: Key represented as a string.
 * @arg3: This function will RETURN the server ID 
          which stores the value via this parameter.
 *
 * The load balancer will search for the server which should posess the 
 * value associated to the key. The server will return NULL in case 
 * the key does NOT exist in the system.
 */
char* loader_retrieve(load_balancer* main, char* key, int* server_id);

/**
 * load_add_server() - Adds a new server to the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the new server.
 *
 * The load balancer will generate 3 replica TAGs and it will
 * place them inside the hash ring. The neighbor servers will 
 * distribute some the objects to the added server.
 */
void hash_ring_add_server(load_balancer *main, long long label);
void loader_add_server(load_balancer* main, int server_id);

/**
 * load_remove_server() - Removes a specific server from the system.
 * @arg1: Load balancer which distributes the work.
 * @arg2: ID of the removed server.
 *
 * The load balancer will distribute ALL objects stored on the
 * removed server and will delete ALL replicas from the hash ring.
 */
void loader_remove_server(load_balancer* main, int server_id);


#endif  // LOAD_BALANCER_SKEL_LOAD_BALANCER_H_
