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
	
	size_t b_n=conf->db_size/conf->page_size;
	size_t bitm_s= b_n;
	bool *bitm=(bool *)calloc(bitm_s, sizeof(bool));
	
	db->fd=fd;
	db->block_size=conf->page_size;
	db->block_num=b_n;
	db->bitmap_size=bitm_s;
	db->bitmap=bitm;
	db->start_ind=bitm+bitm_s;
	db->root_ind=bitm+bitm_s;
	db->root = (block *)calloc(1, sizeof(block));
	
	
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
