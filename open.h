#include "util.h"

static Vector open_file_table;
static int is_init = 0;

void init(){
	if( is_init == 0 ){ 
		vector_init( & open_file_table ); 
		is_init = 1; 
	}
}

/* "Opens" a file. Add file name to vector of opened files. */
int open( char* filename, char* mode )
{	
	/* Check mode type, print error and return if invalid */
	if(strcmp(mode,"r") != 0 && strcmp(mode,"w") != 0 &&
	   strcmp(mode,"rw") != 0 && strcmp(mode,"wr") != 0){
		printf("Error: incorrect parameter!\n");
		return 0;	
	}else{
		/* Initialize vector */
		init();
		/* Check vector if file is already open and return if true */
		for( int i = 0; i < vector_get_size( & open_file_table ); i++ ){
			if( strcmp( filename, vector_get( & open_file_table, i )) == 0 ){
				printf( "Error: File already open!\n" );
				return 0;			
			}	
		}
		/* if filename was not found to be open, add to vector */
		vector_append( & open_file_table, filename, mode );	
		return 1;
	}
}

/* "Closes" a file. Remove file name from vector of opened files */
void close( char * filename )
{
	/* Loop through vector and remove entry with filename if found */
	for( int i = 0; i < vector_get_size( & open_file_table ); i++ ){
		if( strcmp( filename, vector_get( & open_file_table, i )) == 0 ){
			vector_remove( & open_file_table,filename );	
			return;
		}
	}
	/* If control reaches here, file not found in vector */
	printf( "Error: file not open\n" );
}

/* "Closes" a file. Remove file name from vector of opened files */
void close_without_check(char * filename)
{
	/* Loop through vector and remove entry with filename if found */
	for( int i = 0; i < vector_get_size( & open_file_table ); i++ ){
		if( strcmp( filename, vector_get( & open_file_table, i )) == 0 ){
			vector_remove( & open_file_table, filename );	
			return;
		}
	}
}

/* 
	Check vector of open files to see if file is open and readable;
 	Retrurn 1 for true; 0 for false
 */
int fileModeIsReadable( char * filename )
{
	/* Print operation  */
	printf( "fileModeIsReadable => %s\n", filename );
	/* Allocate space for file mode */
	char * mode = calloc( sizeof( char ), 2 );
	for( int i = 0; i < vector_get_size( & open_file_table ); i++ ){
		/* Search vector for file name */
		if( strcmp( filename, vector_get( & open_file_table, i )) == 0 ){
			/* Check if file is readable; return success */
			mode = vector_get_mode( & open_file_table, i );
			if( strcmp( mode, "r" ) == 0 || strcmp( mode, "rw" ) == 0 ||
			   strcmp( mode, "wr" ) == 0 ){
				return 1;
			}
			else{
				return 0;
			}
		}
	}
	return 0;
}

/* 
	Check vector of open files to see if file is open and writeable;
 	Retrurn 1 for true; 0 for false
 */
int fileModeIsWriteable( char * filename )
{
	/* Allocate space for file mode */
	char * mode = calloc( sizeof( char ), 2 );
	for( int i = 0; i < vector_get_size( & open_file_table ); i++ ){
		/* Search vector for file name */
		if( strcmp( filename, vector_get( & open_file_table, i )) == 0 ){
			/* Check if file is writeable; return success */
			mode = vector_get_mode( & open_file_table, i );
			if( strcmp( mode, "w" ) == 0 || strcmp( mode, "rw" ) == 0 ||
			   strcmp( mode, "wr" ) == 0 ){
				return 1;
			}
			else{
				return 0;
			}
		}
	}	
	return 0;
}

/* Display all open files in vector */
void dump_open_file_table()
{
	for( int i = 0; i < vector_get_size( & open_file_table ); i++ ){
		printf( "%s, ", vector_get( & open_file_table, i ));
	}
	printf( "\n" );
}
