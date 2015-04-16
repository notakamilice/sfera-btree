//public database API - my_close, my_sync, my_select, my_insert, my_delete

#include "my_functions.h"
#include <string.h>

//**************************************************************************************
int my_close(DB *db) {
    
  	if (!db && !db->bitmap && !db->root) {
	  	printf("error in my_close db\n");
		return -1;
	}
    
    free(db->bitmap);
    free_block(db->root);
    free(db);

    return 0;
}

//*************************************************************************************
int my_select(DB *db, DBT *key, DBT *value){
	if (!db && !db->root && !key && !key->data) {
	  	printf("error in my_select db\n");
		return -1;
	}
  	return btree_search(db, db->root, key, value);	
}

//-------------------------------------------------------------------------------------
//find key in btree - 0 if exist
int btree_search(DB *db, block *bl, DBT *key, DBT *value) {
	int return_value = 0;
   	size_t index=0;
	
	while (index<bl->kv_num && (compare(key, bl->keys[index]) >0) ) { 
	  	//search for min index: k<=bl->keys[index]
		index++;
	}
	if (index<bl->kv_num && (compare(key, bl->keys[index]) == 0) ) { 
		//key found
		//value? check params?
		if (!bl->keys[index] && !bl->values[index] && value) {
	  		printf("error in btree_search\n");
			return_value = -1;//*******
		}
	  	value->size=bl->values[index]->size;
		value->data=(void *)calloc(1, value->size);
		memcpy(value->data, bl->values[index]->data, value->size);
		return_value = 0;
	}
	else if (bl->is_leaf) 
	  	//if is_leaf then key wasn't found		
	  	//value=NULL;
		//return NULL; 
		return_value=-1;
	else {
	  	//search key in child node
	  	block *child = db->read_block(db, bl->child_ind[index]);
    	if (!child) {
	  		printf("error in btree_search - read_block\n");
			return -1; //***********
		}
    	btree_search(db, child, key, value);
    	free_block(child);	 
	}
	return return_value;
}

//*************************************************************************************
int my_insert(DB *db, DBT *key, DBT *value) {
	if (!db && !db->root && !key && !key->data && !value && !value->data) {
	  	printf("error in my_insert db\n");
		return -1;
	}
	int return_value=0;
	
	if (db->root->kv_num==2*T-1) {
	  	//root is full
	  	size_t fb_i = db->find_free_block(db);
        if (fb_i >= 0) {
        	new_root(db,fb_i); // make new root  - block with index fb_i
        	return_value=btree_split_child(db, db->root, db->root_ind, 0);
        	if (return_value == 0) 
        		return_value = btree_insert_nonfull(db, db->root, db->root_ind, key, value);
        } 
        else {
        	//no free blocks for inserting 
        	return_value = -1;
        	printf("can't insert - there are no more free blocks in file\n");
        }
	}
	else 
	  	return_value= btree_insert_nonfull(db, db->root, db->root_ind, key, value);
	
	if (return_value == -1)
        printf("error in my_insert with key %s\n", key->data);
	
	return return_value;
}
//-------------------------------------------------------------------------------------
int btree_insert_nonfull(DB *db, block *x, size_t x_i, DBT *key, DBT *value) {}

//-------------------------------------------------------------------------------------
//function receive nonfull block x and index of his full child and split child node into 2
int btree_split_child(DB *db, block *x, size_t x_i, size_t child_i) {
  	if (child_i>x->child_num) {
	  	printf("error in btree_split_child with child_i\n");
		return -1;
	}
	int return_value=0;
	
	//y and z will be child nodes (left and right resp.)
	
	//z - new block, z_i - its index
	size_t z_i=db->find_free_block(db);
	block *z;
	
	if (z_i >= 0) {
		//y_i - index of full child
		size_t y_i= x->child_ind[child_i];
		block *y;
		//read block with y_i index
		block *read_bl = db->read_block(db, y_i);
		if (!y) {
		  	printf ("error in btree_split_child -read child\n");
			return -1;
		}
		//fill y and z
		size_t split_i = T-1;
		y = fill_child(read_bl,0,split_i-1);
		z = fill_child(read_bl,split_i+1,read_bl->child_num-1);
		if (!y && !z) {
		  	printf("error in btree_split_child - fill_child\n");
			return -1;
		}
		
		//move read_bl[split_i] into x
		DBT *median_key = read_bl->keys[split_i];
		DBT *median_value = read_bl->values[split_i];
		
		return_value = btree_insert_key_value(db, x, x_i, z_i, median_key, median_value);
    	if (return_value != 0) {
		  	printf ("error in btree_split_child -insert_median_to_parent\n");
			
		}
		
		//write y and z 
		int ret_value1 = db->write_block(db,  y_i, y);
		int ret_value2 = db->write_block(db,  z_i, z);
		
		if ( ret_value1 == 0 &&  ret_value2 == 0 && return_value ==0 ) return_value = 0;
		else {
			printf ("error in btree_split_child  - write_block \n");
		  	return_value = -1;
		}
		
    	free_block(y);
    	free_block(z);
	    	
		
	}
	else return_value = -1;
	
	return return_value;
}

//****************************************************************************************
int my_sync(DB *db){
	/*
    if (!db && !db->bitmap && !db->root) {
	  	printf("error in my_sync db\n");
		return -1;
	}
	*/
    return 0;
}

//****************************************************************************************
//*****************additional functions that were used in public API**********************
//****************************************************************************************
int free_block(block *bl) {
	if (!bl) {
	  	printf("error in free_block - \n");
		return -1;
	}	
	
	if (bl->kv_num)
		for (size_t i=0; i<bl->kv_num; i++) {
	  		if (bl->keys[i]) {
                free(bl->keys[i]);
                bl->keys[i] = NULL;
            }
			if (bl->values[i]) {
                free(bl->values[i]);
                bl->values[i] = NULL;
            }
			free(bl->keys);
			bl->keys = NULL;
			free(bl->values);
        	bl->values = NULL;
		}
	
	if (bl->child_ind) {
        free(bl->child_ind);
        bl->child_ind = NULL;
    }
    free(bl);
    bl = NULL;
   
	return 0;
}

//-------------------------------------------------------------------------------------------
int compare (DBT *key1, DBT *key2) { // return >0 if key1>key2, ==0 if they equal and <0 else
	if (!key1 && !key2 && !key1->data && !key2->data) {
		printf("error in parameteres in compare function db\n");
		return -1;
	} 	
	
	size_t min_key_size=key1->size;
	if (key2->size<min_key_size) min_key_size=key2->size;
	
	int return_value=strncmp(key1->data,key2->data,min_key_size);
	if (return_value==0) {
		if (key1->size > key2->size) return_value=1;
		if (key1->size < key2->size) return_value=-1;
	}
	
	return return_value;
}

//-------------------------------------------------------------------------------------------
int new_root(DB *db, size_t fb_i) {
    free_block(db->root);
    db->root = (block *)calloc(1, sizeof(block));
	db->root->is_leaf = false;
	db->root->child_num = 1;
    db->root->child_ind = (size_t *)calloc(1, sizeof(size_t));
    db->root->child_ind[0] = db->root_ind;
    db->root_ind = fb_i;
	
    return db->mark_block(db, fb_i, 0);//
}

//-------------------------------------------------------------------------------------------

block* fill_child(block* read_bl, size_t from, size_t to){
	if (!bl && bl->kv_num > from && bl->kv_num < to) {
	  	printf("error in fill_child\n");
		return NULL;
	}
  	block *bl = (block *)calloc(1, sizeof(block));
    bl->kv_num =  to - from + 1;
	
	bl->keys = (DBT **)calloc(bl->kv_num, sizeof(item *));
	bl->values = (DBT **)calloc(bl->kv_num, sizeof(item *));
	if (!bl->keys && !bl->values) {
		printf("error in fill_child - memory for block->keys or block->values wasn't allocated\n");
	  	return -1;
	}
		
    //copy keys and values from read_bl to bl
    for (int i = 0; i < bl->kv_num; i++) { 
	  	int j = from+i;
	  	DBT *key_j=read_bl->keys[j];
		DBT *val_j=read_bl->values[j];
		
		//allocate memory for single key-DBT value-DBT
		bl->keys[j] = (DBT *)calloc(1, sizeof(DBT));
		bl->values[j] = (DBT *)calloc(1, sizeof(DBT));
		
		if (!bl->keys[j] && !bl->values[j]){
			printf("error in fill_child  - memory for DBT wasn't allocated\n");
	  		return -1;
		}
		
		bl->keys[j]->size = key_j->size;
		bl->values[j]->size = val_j->size;
		bl->keys[j]->data = (void *)calloc(1,key_j->size);
		bl->values[j]->data = (void *)calloc(1,val_j->size);
		
		if (!bl->keys[j]->data && !bl->values[j]->data){
			printf("memory for DBT->data wasn't allocated\n");
	  		return -1;
		}
		memcpy(bl->keys[j]->data, key_j->data, key_j->size);
		memcpy(bl->values[j]->data, val_j->data, val_j->size);
	}
    
    bl->is_leaf=read_bl->is_leaf;
    
	//copy children    
    if (!read_bl->is_leaf) {
	  	
		bl->child_num=bl->kv_num +1;
    	bl->child_ind = (size_t *)calloc(bl.child_num, sizeof(size_t));
    	for (int i = 0; i < bl->child_num; i++) {
		  	int j=from+i;
    		bl->child_ind[i] = read_bl->child_ind[j];
       	}
    }
    else {
		bl->child_num = 0;
	  	bl->child_ind =NULL;
	}
    
    return bl;
}

//-------------------------------------------------------------------------------------------

int btree_insert_key_value(DB* db, block* x, size_t x_i, size_t child_i, DBT* key, DBT* value) {
	
}






