//public database API - header for my_close, my_sync, my_select, my_insert, my_delete 

#include "main_data_structures.h"

//for keys with equal length
const size_t T = 5; 

int my_close(DB *db);
int my_select(DB *db, DBT *key, DBT *value);
int my_insert(DB *db, DBT *key, DBT *value);
int my_delete(DB *db, DBT *key);
int my_sync(DB *db);

//----------------------------------------------------------------------------------
int free_block(block *bl); //my_select, my_close, my_insert
int btree_search(block *bl, DBT *key, DBT *value); //my_select
int compare (DBT *key1, DBT *key2); //my_select
int btree_insert_nonfull(DB *db, block *x, size_t x_i, DBT *key, DBT *value); //my_insert
int btree_split_child(DB *db, block *x, size_t x_i, size_t child); //my_insert
int new_root(DB *db, size_t fb_i); //my_insert
block *fill_child(block *x, size_t from, size_t to); //my_insert
int btree_insert_key_value(DB *db, block *x, size_t x_i, size_t child_i, DBT *key, DBT *value); //my_insert
