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
//должна вернуть упорядоченную пару (y,i) где y.key[i]==key (корман)
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
int btree_split_child(DB *db, block *x, size_t x_i, size_t child_i) {
  	if (child_i>x->child_num) {
	  	printf("error in btree_split_child with child_i\n");
		return -1;
	}
	int return_value=0;
	
}
//-------------------------------------------------------------------------------------
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






