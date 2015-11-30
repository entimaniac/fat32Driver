#include "util.h"
#include "ls.h"
#include "open.h"
#include "cd.h"
#include "read.h"
#include "size.h"
#include "write.h"

/*
#include "close.h"
#include "rm.h"
#include "mkdir.h"
#include "rmdir.h"
*/

/******************************************************************************/
/*                      DEFINED CONSTANTS */
/******************************************************************************/

/* 
These are used for checking the number of paramters needed for each function 
call. For example, open needs 2 arguments: name and mode. 
*/
#define OPEN_ARG_NUM 2
#define	CLOSE_ARG_NUM 1
#define	CREATE_ARG_NUM 1
#define RM_ARG_NUM 1
#define SIZE_ARG_NUM 1
#define CD_ARG_NUM 1
#define LS_ARG_NUM 1
#define MKDIR_ARG_NUM 1
#define RMDIR_ARG_NUM 1
#define READ_ARG_NUM 3
#define WRITE_ARG_NUM 4


unsigned int getValueFromBootSector(unsigned char* buffer, int offset, int size)
{

	unsigned val = 0;	
	for(int i = offset+size-1; i >= offset; i--){
		val = val << 8;
		val += buffer[i];
	}
	return val;
}

void getValueFromDirectorySector(unsigned char* buffer, unsigned char* DIR_val, int offset, int size)
{
	for(int i = offset+size-1, j = 0; i >= offset; i--, j++){
		DIR_val[j] = buffer[i];	
	}
}

void getFileName(unsigned char* buffer, unsigned char* filename, int start)
{
	int count = 0;
	int file_done = 0;

	if(buffer[start] == 0x2E){//dot
		while(buffer[start] != 0x20){//space
			filename[count] = buffer[start];
			count++;
			start++;
		}
	}
	else{
		for(int i = start+1; i < start+10 && file_done == 0; i+=2){
			if((int)buffer[i] >= 32){	
				filename[count] = buffer[i];
				count++;
			}else{
				file_done = 1;
			}
		}
		for(int i = start+14; i < start+14+32 && file_done == 0; i+=2){
			if((int)buffer[i] >= 32){
				filename[count] = buffer[i];
				count++;
			}else{
				file_done = 1;
			}
		}
	}
	if(count < 16){
		filename[count] = '\0';
	}			
}

struct directory getDirectoryInformation(unsigned char* buffer, int start)
{
	struct directory d;
	int count = 0;
	int file_done = 0;
	getValueFromDirectorySector(buffer,d.DIR_Name,start+0,11);
	getValueFromDirectorySector(buffer,d.DIR_Attr,start+11,1);
	getValueFromDirectorySector(buffer,d.DIR_NTRes,start+12,1);
	getValueFromDirectorySector(buffer,d.DIR_CrtTimeTenth,start+13,1);
	getValueFromDirectorySector(buffer,d.DIR_CrtTime,start+14,2);
	getValueFromDirectorySector(buffer,d.DIR_CrtDate,start+16,2);
	getValueFromDirectorySector(buffer,d.DIR_LstAccDate,start+18,2);
	getValueFromDirectorySector(buffer,d.DIR_FstClusHI,start+20,2);
	getValueFromDirectorySector(buffer,d.DIR_WrtTime,start+22,2);
	getValueFromDirectorySector(buffer,d.DIR_WrtDate,start+24,2);
	getValueFromDirectorySector(buffer,d.DIR_FstClusLO,start+26,2);
	getValueFromDirectorySector(buffer,d.DIR_FileSize,start+28,4);	
	getFileName(buffer,d.filename,start);	
	
	int a = (int)d.DIR_Attr[0];
	switch(a){
		case 0x01:{d.Attribute = ATTR_READ_ONLY;break;}
		case 0x02:{d.Attribute = ATTR_HIDDEN;break;}
		case 0x04:{d.Attribute = ATTR_SYSTEM;break;}
		case 0x08:{d.Attribute = ATTR_VOLUME_ID;break;}
		case 0x10:{d.Attribute = ATTR_DIRECTORY;break;}
		case 0x20:{d.Attribute = ATTR_ARCHIVE;break;}
		default:{d.Attribute = ATTR_LONG_NAME;}
	}
	return d;
}

unsigned int currentClusterNumber(MODE mode, unsigned int num){
	static int CCN;
	switch(mode){
		case GET:{return CCN;}
		
		case SET:{
			CCN = num;
		return 0;}
	}
}

void presentWorkingDirectory(MODE mode, char* dir){
	static unsigned char PWD[256] = "/";
	static unsigned int size = 1;
	switch(mode){
		case GET:{
			strcpy(dir,PWD);
			break;}
		case ADD:{
			if(size != 1) strncat(PWD,"/",1);
			if(strlen(dir) > 0){
				strncat(PWD,dir,strlen(dir));
				size+=(1+strlen(dir));
			}
			break;}
		case SUB:{
			for(int i = size-1; i >= 0;i--){
				if(PWD[i] == '/'){
					if(i != 0){
						PWD[i] = '\0';
					}else 
						PWD[1] = '\0';					
					break;
				}else{
					PWD[i] = (char)0;
					size--;	
				}
			}
			break;}
	}
}
void getCluster(struct directory* CurrentCluster, unsigned char* buffer, 
		  unsigned int NextClusterNumber, unsigned int FirstDataSector,
		  unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,
		  unsigned int BPB_BytsPerSec)
{
  unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;

    FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
    ThisFATSecNum = BPB_ResvdSecCnt+((4*NextClusterNumber) / BPB_BytsPerSec);
    ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;

    for(int i = 0; i < 512; i+=32){
       CurrentCluster[i/32] = getDirectoryInformation(buffer,
                                  FirstSectorofCluster*SIZE_OF_SECTOR+i);
    }
    NextClusterNumber = getValueFromBootSector(buffer,
                                  (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset,
                                              4);
}

void removeTrailingNewline(char* s)
{
	char *pos;
	if ((pos=strchr(s, '\n')) != NULL)
	    *pos = '\0';	
}

unsigned int parseInput(struct directory* cluster, unsigned char* buffer, 
		unsigned int FDS, unsigned int SPC, unsigned int RSC,
		unsigned int BPS, char* PWD, char* command)
{
	char* args = (char*)calloc(sizeof(char),64);
	int return_val;	

	getc(stdin); //ignore space
	fgets(args,64,stdin);
	removeTrailingNewline(args);
	return_val = isCommand(cluster,buffer,FDS,SPC,RSC,BPS,command,args);
	if(return_val){
		switch(return_val){
			case 2:{
				presentWorkingDirectory(ADD,args);
				break;}
			case 3:{
				presentWorkingDirectory(SUB,"");
				break;}
		}
	}else{
		printf("%s is not a valid command\n",command);
	}
	return currentClusterNumber(GET,0);

}

int isCommand( struct directory* cluster, unsigned char* buffer, 
		unsigned int FDS, unsigned int SPC, unsigned int RSC,
		unsigned int BPS, char* input, char* args )
{

	/* Make a copy of args; iterate through it with strtok; count arguments */
	int argumentCount = 0;
	char * tempArgs = malloc( 1 + strlen ( args ));
    strcpy( tempArgs, args );
    for( tempArgs = strtok( tempArgs, " " ); tempArgs;
        tempArgs = strtok( NULL, " " )) {
			argumentCount++;   
    }
			printf( "Number of args: %d\n", argumentCount );	

	int dir_result = isDir( cluster, args );
	int file_result = isFile( cluster, args );
	int r,w;

	/* OPEN */
	if( strcmp( input, "open" ) == 0 ){

		// check number of args:
		if( checkArgumentCount( argumentCount, OPEN_ARG_NUM ))
			return 1;

		char * mode = calloc( sizeof( char ), 2 );
		char * ptr  = calloc( sizeof( char ), 64 );	
		ptr = strtok( args, " " );
		mode = strtok( NULL, " " );

		if( isFile( cluster, ptr )){
			open( args, mode );
		} else{
			printf( "Error: File does not exist!\n" );	
		}
		//free?
		return 1;

	}

	/* CLOSE */
	else if(strcmp(input,"close") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, CLOSE_ARG_NUM ))
			return 1;

		close(args);
		return 1;
	}
	/* CREATE */
	else if(strcmp(input,"create") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, CREATE_ARG_NUM ))
			return 1;

		return 1;
	}
	/* RM */
	else if(strcmp(input,"rm") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, RM_ARG_NUM ))
			return 1;

		return 1;
	}
	/* SIZE */
	else if(strcmp(input,"size") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, SIZE_ARG_NUM ))
			return 1;
		
		// call size:
		if(file_result){
			size(buffer,args,currentClusterNumber(GET,0),FDS,SPC,RSC,BPS);
		}else{
			printf("%s\n", "Error: Invalid file name!");
		}

		return 1;
	}
	/* CD */
	else if(strcmp(input,"cd") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, CD_ARG_NUM ))
			return 1;

		if(dir_result == -1){ //current dir
			//dont change currentClusterNumber	
		}else if(dir_result == -2){
			currentClusterNumber(SET,
				cd_back(buffer,
					currentClusterNumber(GET,0),
					FDS,SPC,RSC,BPS)	
			);
			return 3;		
		}else{
			r = cd(buffer,args,currentClusterNumber(GET,0),FDS,SPC,RSC,BPS);
			if(r > 0){
				currentClusterNumber(SET,r);
				return 2;
			}else{
				printf("%s: Invalid directory\n",args);
			}
		}
		return 1;
	}
	/* LS */
	else if(strcmp(input,"ls") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, LS_ARG_NUM ))
			return 1;

		if(dir_result > 0){
			ls(buffer,args,dir_result,FDS,SPC, RSC, BPS);
		}
		else if(dir_result < 0){
			ls(buffer,args,currentClusterNumber(GET,0),FDS,SPC,RSC,BPS);
		}
		else 
			printf("%s: Invalid directory\n",args);
		return 1;
	}
	/* MKDIR */
	else if(strcmp(input,"mkdir") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, MKDIR_ARG_NUM ))
			return 1;

		return 1;
	}
	/* RMDIR */
	else if(strcmp(input,"rmdir") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, RMDIR_ARG_NUM ))
			return 1;

		return 1;
	}
	/* READ */
	else if(strcmp(input,"read") == 0){

		// check number of args:
		if( checkArgumentCount( argumentCount, READ_ARG_NUM ))
			return 1;

		char* file = calloc(sizeof(char),64);
		long int start, num_bytes;
		char *pEnd;
		char *temp1 = calloc(sizeof(char),8);
		char *temp2 = calloc(sizeof(char),8);
		file = strtok(args," ");
		r = fileModeIsReadable(file);
		if(r == 1){
			temp1 = strtok(NULL," ");
			start = strtol(temp1,&pEnd,10);
			temp2 = strtok(NULL," ");
			num_bytes = strtol(temp2,&pEnd,10);
			if(start >= SIZE_OF_SECTOR){
				printf( "Error: attempt to read beyond EoF\n" );	
			}else{
				read( buffer, file, start, num_bytes, currentClusterNumber( GET, 0 ), FDS, SPC, RSC, BPS );
			}
		}else{
			printf( "Error: file is not open for writing!\n" );
		}
		return 1;
	}
	/* WRITE */
	else if( strcmp( input, "write" ) == 0 ){

		// check number of args:
		if( checkArgumentCount( argumentCount, WRITE_ARG_NUM ))
			return 1;
			
		char* file = calloc(sizeof(char),64);
		long int start, num_bytes;
		char *pEnd;
		char *temp1 = calloc(sizeof(char),8);
		char *temp2 = calloc(sizeof(char),8);
		char *temp3 = calloc(sizeof(char),8);
		file = strtok(args," ");
		w = fileModeIsWriteable(file);
		if(w == 1){
			temp1 = strtok(NULL," ");
			start = strtol(temp1,&pEnd,10);
			temp2 = strtok(NULL," ");
			num_bytes = strtol(temp2,&pEnd,10);
			temp3 = strtok(NULL,"\n");
			if(strlen(temp3) != num_bytes){
				printf("Error: Bytes requested does not match string size!\n");
				return 1;
			}
			if(start >= SIZE_OF_SECTOR){
				printf("Error: attempt to read beyond EoF\n");	
			}else{
				printf("%s\n", file);
				FILE *fileptr;
				write(buffer,file,start,num_bytes,temp3,currentClusterNumber(GET,0),FDS,SPC,RSC,BPS);
				fileptr = fopen("fat32.img", "wb");
				fwrite(buffer,1,67108864,fileptr);  // this third argument needs to be replaced with a variable
				fclose(fileptr);
			}
		}else{
			printf("Error: file is not open for writing!\n");
		}

		return 1;
	}
	/* EXIT */
	else if( strcmp( input,"exit" ) == 0 ){
		printf( "Goodbye!\n" );
		exit( 1 );
	}else return 0;
}

int isFile(struct directory* cluster, char* input)
{
	int val = 0;
	for(int i = 0; i < 16; i++){
		if(strcmp(cluster[i].filename,input) == 0){
			if(cluster[i+1].Attribute != ATTR_DIRECTORY){
				//matches name of file
				val = ((int)cluster[i+1].DIR_FstClusLO[0] << 2) +
					((int)cluster[i+1].DIR_FstClusLO[1]);

				return val;
			}else{
				//matches name of directory
				return val;	
			}
		}
	}
	//matches name of nothing
	return val;
}
int isDir(struct directory* cluster, char* input)
{
	int val = 0;
	if(strcmp(input,".") == 0){
		return -1;
	}else if(strcmp(input,"..") == 0){
		if(strcmp(cluster[2].filename,input) == 0){
			val = ((int)cluster[2].DIR_FstClusLO[0] << 2) +
				((int)cluster[2].DIR_FstClusLO[1]);

			return val;
		}else{
			//if referencing .. from root dir, pretend it is .
			return -2;
		}
	}	
	for(int i = 0; i < 16; i++){
		if(strcmp(cluster[i].filename,input) == 0){
			if(cluster[i+1].Attribute == ATTR_DIRECTORY){
				//matches name of directory
				val = ((int)cluster[i+1].DIR_FstClusLO[0] << 2) +
					((int)cluster[i+1].DIR_FstClusLO[1]);

				return val;
			}else{
				//matches name of file
				return val;	
			}
		}
	}
	//matches name of nothing
	return val;
}

/* 
Determines if the number of arguments given is correct, too few, or too many.
Returns 0 for equal, returns 1 for too few, returns 2 for too many.
Intended to be called by: isCommand()
 */
int checkArgumentCount( int argNum, int numReq ){

	int returnValue = 0;

	if( argNum == numReq ){
		returnValue = 0; 
	}
	else if( argNum < numReq ){
		printf( "Not enough arguments. %d provied. Needed:  %d \n", argNum, numReq );
		returnValue = 1;
	}
	else {
		printf( "Too many arguments. %d provided. Needed: %d \n", argNum, numReq );
		returnValue = 2;
	}

	return returnValue;
}
