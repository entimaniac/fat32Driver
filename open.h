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
	if(strcmp(mode,"r") != 0 && strcmp(mode,"w") != 0 &&
	   strcmp(mode,"rw") != 0 && strcmp(mode,"wr") != 0){
		printf("Error: incorrect parameter!\n");
		return 0;	
	}else{
		init();
		for(int i = 0; i < vector_get_size(&open_file_table); i++){
			if(strcmp(filename,vector_get(&open_file_table,i)) == 0){
				printf("Error: File already open!\n");
				return 0;			
			}	
		}
		//if filename was not found to be open
		vector_append(&open_file_table,filename,mode);	
		return 1;
	}
}

void close(char* filename)
{
	for(int i = 0; i < vector_get_size(&open_file_table); i++){
		if(strcmp(filename,vector_get(&open_file_table,i)) == 0){
			vector_remove(&open_file_table,filename);	
			return;
		}
	}
	printf("Error: file not open\n");
}

void dump()
{
	for(int i = 0; i < vector_get_size(&open_file_table); i++){
		printf("%s, ",vector_get(&open_file_table,i));
	}printf("\n");
}
