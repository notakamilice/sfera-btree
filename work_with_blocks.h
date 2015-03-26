#include "main_data_structures.h"


size_t find_free_block(DB *db);
int    mark_block(DB *db, size_t nb, bool is_free);
block  *read_block(DB *db, size_t nb);
int    write_block(DB *db, size_t nb, block *bl);


//find free block in file
size_t find_free_block(DB* db) {
	if (!db->bitmap) printf("database wasn't initialized")
  	for (int i=0; i<db->bitmap_size; i++) {
		if (!db->bitmap[i])  return db.start_ind+i;
	}
	printf("Smth's going wrong - can't found free block");
	return -1;
}


//mark block as free if it's full and vice versa
int mark_block(DB* db, size_t nb, bool is_free) {
	if (!db->bitmap) printf("database wasn't initialized");
	if (nb<db.start_ind || nb>db->block_num) printf("you want impossible things - out of memory");   
	
}
