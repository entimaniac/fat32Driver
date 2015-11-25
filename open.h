#include "util.h"

static Vector open_file_table;
static int is_init = 0;

void init(){
	if(is_init == 0){ 
		vector_init(&open_file_table); 
		is_init = 1; 
	}
}

int open(char* filename, char* mode)
{
	//Need to check for valid mode including possinility of NULL

	printf("filename = %s\nmode = %s\n",filename,mode);
	init();
	for(int i = 0; i < vector_get_size(&open_file_table); i++){
		if(strcmp(filename,vector_get(&open_file_table,i)) == 0){
			printf("File already open\n");
			return 0;			
		}	
	}
	//if filename was not found to be open
	vector_append(&open_file_table,filename,mode);	
	
	return 1;
}
