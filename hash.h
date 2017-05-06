typedef struct node_s {
    char *key;
    char *value;
    struct node_s *next;
} node_t;

typedef struct hash_table_s {
    size_t size;
    node_t **table;
} hash_table_t;

node_t *add_hash_entry(hash_table_t *hashtable, char* key, char *value);
hash_table_t *create_hashtable(size_t size);
void free_hashtable(hash_table_t *hashtable);
size_t hash(hash_table_t *hashtable, char *str);
node_t *lookup_hash_entry(hash_table_t *hashtable, char *value);

