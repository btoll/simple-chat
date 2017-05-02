#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

/**
 * Add the new entry to the head of the linked list of the hash bucket.
 */
node_t *add_hash_entry(hash_table_t *hashtable, char* key, char *value) {
    if (hashtable == NULL) {
        fprintf(stderr, "No hash table\n");
        exit(1);
    }

    size_t hashval = hash(hashtable, key);
    node_t *new_node;

    if ((new_node = malloc(sizeof(node_t))) == NULL) {
        fprintf(stderr, "Could not allocate memory for new node\n");
        exit(2);
    }

    new_node->key = key;
    new_node->value = strdup(value);
    new_node->next = hashtable->table[hashval];

    hashtable->table[hashval] = new_node;
//     printf("%d\n", hashval);

    return new_node;
}

/**
 * Allocate memory for the hash table and its elements and initialize each element.
 */
hash_table_t *create_hashtable(size_t size) {
    if (size < 1) {
        fprintf(stderr, "Size must be greater than zero\n");
        exit(1);
    }

    size_t i;
    hash_table_t *new_table;

    if ((new_table = malloc(sizeof(hash_table_t))) == NULL) {
        fprintf(stderr, "Could not allocate memory for hash table\n");
        exit(2);
    }

    if ((new_table->table = malloc(sizeof(node_t) * size)) == NULL) {
        fprintf(stderr, "Could not allocate memory for hash table elements\n");
        exit(3);
    }

    for (i = 0; i < size; ++i)
        new_table->table[i] = NULL;

    new_table->size = size;

    return new_table;
}

// TODO: Need a (much) better hashing algo! This is just to keep moving along.
size_t hash(hash_table_t *hashtable, char *key) {
    if (hashtable == NULL) {
        fprintf(stderr, "No hash table\n");
        exit(1);
    }

    size_t hashval = 0;

    for (; *key != '\0'; ++key)
        hashval = *key + (hashval << 5) - hashval;

    return hashval % hashtable->size;
}

node_t *lookup_hash_entry(hash_table_t *hashtable, char *key) {
    if (hashtable == NULL) {
        fprintf(stderr, "No hash table\n");
        exit(1);
    }

    size_t hashval = hash(hashtable, key);

    node_t *bucket = hashtable->table[hashval];

    if (bucket == NULL) {
        fprintf(stderr, "No hash bucket\n");
        exit(2);
    }

    while (bucket && strncmp(bucket->key, key, strlen(key)) != 0)
        bucket = bucket->next;

    return bucket;
}

// int main(int argc, char **argv) {
//     char buf[10];
//     int i = 42;

//     memset(&buf, 0, strlen(buf));
//     sprintf(buf, "%d", 5);

//     hash_table_t *my_table = create_hashtable(20);
//     node_t *new_node = add(my_table, buf, "foo");
//     node_t *new_node2 = add(my_table, buf, "bar");
//     node_t *foo = lookup(my_table, buf);
//     printf("%s\n", foo->next->key);

//     return 0;
// }

