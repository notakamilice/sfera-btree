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
	
	//open file
	int fd = open(file, O_CREAT | O_RDWR | O_SYNC, 0700);
	if (fd == -1) {
		free(db); 
		printf("can't create db");
		return NULL;
	}
	
	//fill db parameters
	size_t block_size=conf->page_size;
	size_t number_of_blocks=conf->db_size/block_size;
	size_t bitm_size= number_of_blocks;
	bool *bitm=(bool *)calloc(bitm_size, sizeof(bool));
	size_t file_blocks_num=ceil((bitm_size+3*sizeof(size_t))/block_size);
	
	db->fd=fd;
	db->block_size=block_size;
	db->block_num=number_of_blocks;
	db->bitmap_size=bitm_size;
	db->bitmap=bitm;
	db->file_blocks_num=file_blocks_num;
	db->root_ind=file_blocks_num; 	//****************????????????? 
	db->start_ind=file_blocks_num; //start_ind and root_ind point to the next block after file_info and bitmap  
	db->max_key_size=0;
	db->root = (block *)calloc(1, sizeof(block)); //********************
	
	//fill root
	block *root = (block *)calloc(1, sizeof(block));
	root->kv_num=0;
	root->keys=NULL;
	root->values=NULL;
	root->child_num=0;
	root->child_ind=NULL;
	root->is_leaf=true;
	memcpy(db->root, &root, sizeof(root));
	
	
	//print db parameters
	printf("----db information:");
	printf("fd 						= %d\n",  db->fd);
    printf("block_size    			= %lu\n", db->block_size);
    printf("block_num     			= %lu\n", db->block_num);
    printf("bitmap_size   			= %lu\n", db->bitmap_size);
	printf("root_index    			= %lu\n", db->root_ind);
	printf("start_index   			= %lu\n", db->start_ind);
	printf("----in file %lu blocks are used for service information", db->file_blocks_num);
	
	//write to file structure file_info and bitmap
	file_info *info = (file_info *)calloc(1, sizeof(file_info));
	info->block_size=db->block_size;
	info->block_num=db->block_num;
	info->root_ind=db->root_ind;
	
	ssize_t written = write(db->fd, (void*)&info, sizeof(info));
    if (written != sizeof(info)) {
		free(db->bitmap);
		free(db->root);	
		free(db);
		free(info);
		free(bitm);
        return NULL;
    }
    
    lseek(db->fd, sizeof(info), SEEK_SET);
    written = write(db->fd, (void*)db->bitmap, db->bitmap_size);
    if (written != db->bitmap_size) {
		free(db->bitmap);
		free(db->root);	
		free(db);
		free(info);
		free(bitm);
        return NULL;
    }
    
    //private API
    db->write_block=write_block;
    db->read_block=read_block;
    db->find_free_block=find_free_block;
    db->mark_block=mark_block;
    
	//public API
    db->close  = my_close;
    db->select = my_select;
	db->insert = my_insert;
    db->delete = my_delete;
	db->sync   = my_sync;

    return db;
	
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
