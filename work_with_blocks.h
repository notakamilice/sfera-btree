//private database API - work with blocks

#include "main_data_structures.h"

//check functions

size_t find_free_block(DB *db);
int    mark_block(DB *db, size_t nb, bool is_free);
block  *read_block(DB *db, size_t nb);
int    write_block(DB *db, size_t nb, block *bl);


//find free block in file
size_t find_free_block(DB* db) {
	if (!db && !db->bitmap) {
	  	printf("database wasn't initialized\n");
		return -1;
	}
  	for (int i=0; i<db->bitmap_size; i++) {
		if (!db->bitmap[i])  return db.start_ind+i;
	}
	printf("smth's going wrong - can't found free block\n");
	return -1;
}


//mark block as free if it's full and vice versa
int mark_block(DB* db, size_t nb, bool is_free) {
	if (!db && !db->bitmap) {
	  	printf("database wasn't initialized\n");
		return -1;
	}
	if (nb<db.start_ind || nb>db->block_num) {
	  	printf("you want impossible things - out of memory\n");
		return -1;
	}
	
	nb=nb-db->start_ind;
	
	//mark
	db->bitmap[nb]=is_free;
	
	//lseek to beginning of bitmap
	size_t offset=db->block_size;
	if (lseek(db->fd, 3*sizeof(size_t),SEEK_SET)==-1L) {
		printf("Seek Error\n");
		return -1;
	}
	
    write(db->fd, (void *)db->bitmap, db->bitmap_size);
	return 0;
}


//read block nb from file
block* read_block(DB* db, size_t nb) {
	if (!db && !db->bitmap) {
	  	printf("database wasn't initialized\n");
		return -1;
	}
	if (nb<db.start_ind || nb>db->block_num) {
	  	printf("you want impossible things - out of memory\n");
		return -1;
	}
	
	size_t offset = db->block_size * nb;
	
	block *bl = (struct block *)calloc(1, sizeof(struct block));
	
	//read number of key-value 
	lseek(db->fd, offset, SEEK_SET);
    read(db->fd, (void *)&bl->kv_num, sizeof(size_t));
	
	//allocate memory for keys
    bl->keys = (DBT **)calloc(bl->kv_num, sizeof(DBT *));
    if (!bl->keys) {
		printf("memory for block->keys wasn't allocated\n");
	  	return -1;
	}
	
	//read keys
	for (int i = 0; i < bl->kv_num; i++) { //************************
        //allocate memory for single key-DBT
        DBT *k_dbt = (DBT *)calloc(1, sizeof(DBT));
		if (!k_dbt){
			printf("memory for key-DBT wasn't allocated\n");
	  		return -1;
		}
        bl->keys[i] = k_dbt;
		
		//read key-DBT size and data
		read(db->fd,(void *)&k_dbt->size,sizeof(k_dbt->size));
		k_dbt->data=(void *)calloc(1,k_dbt->size);
		if (!k_dbt->data){
			printf("memory for key-DBT->data wasn't allocated\n");
	  		return -1;
		}
		read(db->fd, k_dbt->data, k_dbt->size);
	}
    
	//allocate memory for values
	bl->values = (DBT **)calloc(bl->kv_num, sizeof(DBT *));
    if (!bl->values) {
		printf("memory for block->values wasn't allocated\n");
	  	return -1;
	}
	
	//read values 
	for (int i = 0; i < bl->kv_num; i++) { //************************
        //allocate memory for single value-DBT
        DBT *v_dbt = (DBT *)calloc(1, sizeof(DBT));
        if (!v_dbt){
			printf("memory for value-DBT wasn't allocated\n");
	  		return -1;
		}
		bl->values[i] = v_dbt;
		
		//read value-DBT size and data
		read(db->fd,(void *)&v_dbt->size,sizeof(v_dbt->size));
		v_dbt->data=(void *)calloc(1,v_dbt->size);
		if (!v_dbt->data){
			printf("memory for value-DBT->data wasn't allocated\n");
	  		return -1;
		}
		read(db->fd, v_dbt->data, v_dbt->size);
    }
    
    // read number of children-blocks of nb block 
    read(db->fd, (void *)&bl->child_num, sizeof(bl->child_num));
    //check valid child_num  - should be kv_num+1 for not leaves and 0 for leaves
    if (bl->child_num != bl->kv_num + 1 && bl->child_num != 0) { //*********************
        for (int i = 0; i < bl->kv_num; i++) {
            free(bl->keys[i]->data);
            free(bl->values[i]->data);
            free(bl->keys[i]);
            free(bl->values[i]);
            
        }
        free(bl->keys);
		free(bl->values);
        free(bl);
        printf("read block error\n");
        return NULL;
    }
    
    // read children indeces
    bl->child_ind = (size_t *)calloc(bl->child_num, sizeof(size_t));
	for (int i = 0; i < bl->child_num; i++) {
        read(db->fd, (void *)&bl->child_ind[i], sizeof(bl->child_ind[i]));
    }
	
    //read bool - is_leaf
    read(db->fd, (void *)&bl->is_leaf, sizeof(bl->is_leaf));
	
    return bl;
}

int write_block(DB* db, size_t nb, block* bl) {
	
	//block can be written ? ***********************
	
	if (!db && !db->bitmap) {
	  	printf("database wasn't initialized\n");
		return -1;
	}
	if (!bl) {
		printf("null block - error in write_block\n");
		return -1;
	}
	if (bl->kv_num==0) {
		printf("error with block in write_block - no keys-values\n");
		return -1;
	}
	if (bl->child_num==00 || bl->child_ind) {
		printf("error with block's children in write_block\n");
		return -1;
	}
	if (nb<db.start_ind || nb>db->block_num) {
	  	printf("you want impossible things - out of memory\n");
		return -1;
	}
	

    size_t offset = db->block_size * nb;
	lseek(db->fd, offset, SEEK_SET);
	
	//write number of key-value pairs
    write(db->fd, (void *)&bl->kv_num, sizeof(size_t));
	
	//write keys
	for (int i = 0; i < bl->kv_num; i++) {
        DBT *k_dbt = bl->keys[i];
        if (!k_dbt && !k_dbt->data) {
			printf("no dbt data in keys - error in write_block\n");
			return -1;
		}
        
        write(db->fd, (void *)&k_dbt->size, sizeof(k_dbt->size));
        write(db->fd, k_dbt->data, k_dbt->size);
        
    }

    //write values
	for (int i = 0; i < bl->kv_num; i++) {
        DBT *v_dbt = bl->keys[i];
        if (!v_dbt && !v_dbt->data) {
			printf("no dbt data in values - error in write_block\n");
			return -1;
		}
        
        write(db->fd, (void *)&v_dbt->size, sizeof(v_dbt->size));
        write(db->fd, v_dbt->data, v_dbt->size);
        
    }
    
	//write children number
	size_t children_num = bl->child_num;
    write(db->fd, (void *)&children_num, sizeof(children_num));
	
	//write children indeces
	for (int i = 0; i < children_num; i++) {
        size_t children_ind = bl->child_ind[i];
        write(db->fd, (void *)&children_ind, sizeof(children_ind));
    }
	//write bool value - is_leaf
	bool leaf = bl->is_leaf;
    write(db->fd, (void *)&leaf, sizeof(leaf));
	
	//mark block as busy
	return mark_block(db, bl, true);
	
}

