//header with main data structures

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <iostream>


//configuration of database
struct DBC {
	/* Maximum on-disk file size
	 * 512MB by default
	 * */
	size_t db_size;
	/* Page (node/data) size
	 * 4KB by default
	 * */
	size_t page_size;
	/* Maximum cached memory size
	 * 16MB by default
	 * */
	size_t cache_size;
};

typedef struct DBT {
	void  *data;
	size_t size;
} DBT;

//Database block
typedef struct block {
	size_t kv_num; 			//number of key-value pairs
	DBT **keys;				//keys
	DBT **values;			//values
	size_t child_num;		//number of children
	size_t *child_ind;		//children
	bool is_leaf; 			//if block is leaf
} block;

//Service information that will be written in file
typedef struct file_info { 
  	size_t block_size; 			//size of block
	size_t block_num; 			//number of blocks
	size_t root_ind; 			//root index
  	
} file_info;

//Database API
typedef struct DB {
	int    fd;						//file descriptor
	size_t block_size; 				//size of block
	size_t block_num; 				//number of blocks
	size_t bitmap_size; 			//length of bitmap
	bool*  bitmap; 					//bad bitmap (bytemap)
	size_t file_blocks_num;			//how many blocks (file_info + bitmap) use
	size_t root_ind; 				//root index
	size_t start_ind; 				//first block index
	size_t max_key_size;			//for keys with certain length
	block* root; 					//root block
	
	/* Public API */
	/* Returns 0 on OK, -1 on Error */
	int (*close)(struct DB *db);
	int (*delete)(struct DB *db, struct DBT *key);
	int (*insert)(struct DB *db, struct DBT *key, struct DBT *data);
	/* * * * * * * * * * * * * *
	 * Returns malloc'ed data into 'struct DBT *data'.
	 * Caller must free data->data. 'struct DBT *data' must be alloced in
	 * caller.
	 * * * * * * * * * * * * * */
	int (*select)(struct DB *db, struct DBT *key, struct DBT *data);
	/* Sync cached pages with disk
	 * */
	int (*sync)(struct DB *db);
	/* For future uses - sync cached pages with disk
	 * int (*sync)(const struct DB *db)
	 * */
	/* Private API */
	/*     ...     */
	//Work with blocks - read/write/find/mark
	size_t (*find_free_block)(struct DB *db); 
	int (*mark_block)(struct DB *db, size_t nb, bool is_free);
	block  *(*read_block)(struct DB *db, size_t nb);
	int    (*write_block)(struct DB *db, size_t nb, struct block *bl); 

} DB; /* Need for supporting multiple backends (HASH/BTREE) */


