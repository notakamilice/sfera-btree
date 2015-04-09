#include "mydb.h"

DB* dbopen(char* file, DBC* conf) {
	if (!file) {
	  	printf("file isn't selected - can't open db");
		return -1;
	  	
	};
	DB *db = (DB *)calloc(1, sizeof(DB));
   	return db;
	
}

DB* dbcreate(char* file, DBC* conf) {
  	if (!file) {
	  	printf("file isn't selected - can't create db");
		return -1; 
	}
  	DB *db = (DB *)calloc(1, sizeof(DB));
	
	int fd = open(file, O_CREAT | O_RDWR | O_SYNC, 0700);
	if (fd == -1) {
		free(db); 
		printf("can't create db");
		return NULL;
	}
	
	size_t bl_size=conf->page_size;
	size_t number_of_blocks=conf->db_size/bl_size;
	size_t bitm_size= number_of_blocks;
	size_t bitm_num_blocks=ceil(bitm_size/bl_size);
	bool *bitm=(bool *)calloc(bitm_size, sizeof(bool));
	
	
	db->fd=fd;
	db->block_size=bl_size;
	db->block_num=number_of_blocks;
	db->bitmap_size=bitm_size;
	db->bitmap_num_blocks=bitm_num_blocks;
	db->bitmap=bitm;
	db->root_ind=bitm_num_blocks+1; //block number following bitmap and 1 block of db-service info
	db->start_ind=bitm_num_blocks+1;
	
	db->root = (block *)calloc(1, sizeof(block)); //********************
	
	printf("db information:");
	printf("fd 						= %d\n",  db->fd);
    printf("block_size    			= %lu\n", db->block_size);
    printf("block_num     			= %lu\n", db->block_num);
    printf("bitmap_size   			= %lu\n", db->bitmap_size);
	printf("bitmap_num_blocks		= %lu\n", db->bitmap_num_blocks);
    printf("root_index    			= %lu\n", db->root_ind);
	printf("start_index   			= %lu\n", db->start_ind);
	printf("root          			= %lu\n", db->root);
    
	
}


int db_close(struct DB *db) {
	return db->close(db);
}

int db_delete(struct DB *db, void *key, size_t key_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	return db->delete(db, &keyt);
}

int db_select(struct DB *db, void *key, size_t key_len,
	   void **val, size_t *val_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	struct DBT valt = {0, 0};
	int rc = db->select(db, &keyt, &valt);
	*val = valt.data;
	*val_len = valt.size;
	return rc;
}

int db_insert(struct DB *db, void *key, size_t key_len,
	   void *val, size_t val_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	struct DBT valt = {
		.data = val,
		.size = val_len
	};
	return db->insert(db, &keyt, &valt);
}
