ls.h                                                                                                0000664 0001750 0001750 00000002514 12626377446 011200  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 #include "util.h"

void ls(unsigned char* buffer, char* dir, unsigned int NextClusterNumber, 
	unsigned int FirstDataSector, unsigned int BPB_SecPerClus,
	unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec){

	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n;

//	printf("First NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		int val = 0;
		for(int i = 0; i < 512; i += 32){
			cluster[i/32] = getDirectoryInformation(buffer,
						FirstSectorofCluster*SIZE_OF_SECTOR+i);	
			if(cluster[i/32].Attribute == ATTR_LONG_NAME || 
				strcmp(cluster[i/32].filename,".") == 0 ||
				strcmp(cluster[i/32].filename,"..") == 0)
				printf("%s ",cluster[i/32].filename);
		} puts("");

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
//		printf("New NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	}
	//end of function
}

                                                                                                                                                                                    open.h                                                                                              0000664 0001750 0001750 00000003644 12626400476 011516  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 #include "util.h"

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

int fileModeIsReadable(char *filename)
{
	printf("fileModeIsReadable => %s\n",filename);
	char *mode = calloc(sizeof(char),2);
	for(int i = 0; i < vector_get_size(&open_file_table); i++){
		if(strcmp(filename,vector_get(&open_file_table,i)) == 0){
			mode = vector_get_mode(&open_file_table,i);
			if(strcmp(mode,"r") == 0 || strcmp(mode,"rw") == 0 ||
			   strcmp(mode,"wr") == 0){
				return 1;
			}
			else{
				return 0;
			}
		}
	}
	return 0;
}

int fileModeIsWriteable(char * filename)
{
	char *mode = calloc(sizeof(char),2);
	for(int i = 0; i < vector_get_size(&open_file_table); i++){
		if(strcmp(filename,vector_get(&open_file_table,i)) == 0){
			mode = vector_get_mode(&open_file_table,i);
			if(strcmp(mode,"w") == 0 || strcmp(mode,"rw") == 0 ||
			   strcmp(mode,"wr") == 0){
				return 1;
			}
			else{
				return 0;
			}
		}
	}	
	return 0;
}

void dump_open_file_table()
{
	for(int i = 0; i < vector_get_size(&open_file_table); i++){
		printf("%s, ",vector_get(&open_file_table,i));
	}printf("\n");
}
                                                                                            read.h                                                                                              0000664 0001750 0001750 00000006650 12626406316 011467  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 #include "util.h"

int read_file(unsigned char* buffer, int start, int num_bytes, unsigned int size,
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n, bytes_gotten = 0, total_gotten = 0;
	char *output = calloc(sizeof(char),num_bytes);
	strcpy(output,"");
	char *temp = calloc(sizeof(char),1);

//	printf("First NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	

	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		int val = 0;
		for(int i = start; i < size && total_gotten <= num_bytes/*start+num_bytes*/; i++){
			temp[0] = buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i];
			strncat(output,temp,1);
			bytes_gotten++;
			total_gotten++;
		}

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
		start = 0;
		num_bytes -= bytes_gotten; 
		bytes_gotten = 0;
//		printf("New NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	}
	printf("%s\n",output);
	return 1;
	//end of function
}

void read(unsigned char* buffer, char* dir, int start, int num_bytes,
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n, return_val;
	unsigned int num;
	unsigned int size;

//	printf("First NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		int val = 0;
		for(int i = 0; i < 512; i += 32){
			cluster[i/32] = getDirectoryInformation(buffer,
						FirstSectorofCluster*SIZE_OF_SECTOR+i);	
			if(strcmp(cluster[i/32].filename,dir) == 0)
			{		
				cluster[(i/32)+1] = getDirectoryInformation(buffer,
							FirstSectorofCluster*SIZE_OF_SECTOR+i+32);
				num = ((int)cluster[(i/32)+1].DIR_FstClusLO[0] << 2) +
					((int)cluster[(i/32)+1].DIR_FstClusLO[1]);

				size = ((int)cluster[(i/32)+1].DIR_FileSize[0] *16*16*16*16*16*16) +
					((int)cluster[(i/32)+1].DIR_FileSize[1] *16*16*16*16) +
					((int)cluster[(i/32)+1].DIR_FileSize[2] *16*16) +
					((int)cluster[(i/32)+1].DIR_FileSize[3]);
				printf("file size = 0x%08X\n",size);

				return_val = read_file(buffer,start,num_bytes,size,num,FirstDataSector,
						BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec);
				if(return_val == 0){
					printf("Error: attempt to read beyond EoF\n");
				}
				return;
			}
		}

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
//		printf("New NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	}
	//end of function
}

                                                                                        util.h                                                                                              0000664 0001750 0001750 00000003372 12626404442 011525  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 #ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vector.h"

typedef int bool;

static const int SIZE_OF_SECTOR = 512;
static const int FREE_DIR = 0xE5; //229
static const int ZERO_DIR = 0;
static const int EOC = 0x0FFFFFF8;
static const int EOC2 = 0x0FFFFFFF;
typedef enum {ATTR_READ_ONLY,
	ATTR_HIDDEN,
        ATTR_SYSTEM,
        ATTR_VOLUME_ID,
	ATTR_DIRECTORY,
	ATTR_ARCHIVE,
	ATTR_LONG_NAME
} ATTR;
typedef enum{
	GET,SET,ADD,SUB
} MODE;

struct directory{
	unsigned char DIR_Name[11];
	unsigned char DIR_Attr[1];
	unsigned char DIR_NTRes[1];
	unsigned char DIR_CrtTimeTenth[1];
	unsigned char DIR_CrtTime[2];
	unsigned char DIR_CrtDate[2];
	unsigned char DIR_LstAccDate[2];
	unsigned char DIR_FstClusHI[2];
	unsigned char DIR_WrtTime[2];
	unsigned char DIR_WrtDate[2];
	unsigned char DIR_FstClusLO[2];
	unsigned char DIR_FileSize[4];
	unsigned char filename[16];
	ATTR Attribute;	
};


unsigned int getValueFromBootSector(unsigned char*, int, int);
void getValueFromDirectorySector(unsigned char*, unsigned char*, int, int);
void getFileName(unsigned char*,unsigned char*,int);
struct directory getDirectoryInformation(unsigned char*, int);
unsigned int currentClusterNumber(MODE,unsigned int);
void presentWorkingDirectory(MODE,char*);
void getCluster(struct directory*,unsigned char*,unsigned int,unsigned int,
			unsigned int, unsigned int, unsigned int);

void removeTrailingNewline(char*);

unsigned int parseInput(struct directory*,unsigned char*,unsigned int,unsigned int,
			unsigned int,unsigned int,char*,char*);
int isCommand(struct directory*,unsigned char*,unsigned int,unsigned int,
			unsigned int, unsigned int, char*,char*);
int isFile(struct directory*,char*);
int isDir(struct directory*,char*);






#endif
                                                                                                                                                                                                                                                                      vector.h                                                                                            0000664 0001750 0001750 00000001641 12626362411 012046  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 /*NOTE:
 * Credit for the structure of vector.h and
 * vector.c is given to Happy Bear Software
 * Article via this link:
 * https://www.happybearsoftware.com/implementing-a-dynamic-array
*/
#ifndef VECTOR_H
#define VECTOR_H

#define VECTOR_INITIAL_CAPACITY 100

// Define a vector type
typedef struct {
  int size;      // slots used so far
  int capacity;  // total available slots
  char** data;     // char array we're storing
  char** data_mode; //mode relating to data in char** data
} Vector;

void vector_init(Vector *vector);
void vector_append(Vector *vector, char* value, char* mode);
char* vector_get(Vector *vector, int index);
char* vector_get_mode(Vector *vector, int index);
void vector_set(Vector *vector, int index, char* value);
void vector_double_capacity_if_full(Vector *vector);
int vector_get_size(Vector *vector);
void vector_remove(Vector *vector, char* value);
void vector_free(Vector *vector);

#endif
                                                                                               p3.c                                                                                                0000664 0001750 0001750 00000004354 12625506756 011100  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 #include "util.h"

int main()
{
	FILE *fileptr;
	unsigned char *buffer;
	long filelen;
	static unsigned int BPB_BytsPerSec, BPB_SecPerClus, BPB_ResvdSecCnt,
				BPB_NumFATs, BPB_FATSz32, BPB_RootClus,
				BPB_RootEntCnt, BPB_TotSec32;
	unsigned int RootDirSectors, FATSz, FirstSectorofCluster, 
			FirstDataSector, ThisFATSecNum, ThisFATEntOffset,
			DataSec, CountofClusters;
	unsigned int NextClusterNumber;
	char* PWD = (char*)calloc(sizeof(char),256);
	char* input = (char*)calloc(sizeof(char),256);
	char* command = (char*)calloc(sizeof(char),64);
	struct directory current_dir;
	struct directory CurrentCluster[16];
	

	//READ THE IMAGE FILE INTO A ARRAY OF BYTES
	//Example: buffer[0] = EB
	fileptr = fopen("fat32.img", "rb");
	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr);
	rewind(fileptr);
	buffer = (unsigned char*)calloc(sizeof(unsigned char),filelen+1);
	fread(buffer, filelen, 1, fileptr);
	fclose(fileptr);

	//GET IMPORTANT VALUES FROM BOOT SECTOR
	BPB_BytsPerSec = getValueFromBootSector(buffer,11,2);
	BPB_SecPerClus = getValueFromBootSector(buffer,13,1);
	BPB_ResvdSecCnt = getValueFromBootSector(buffer,14,2);
	BPB_NumFATs = getValueFromBootSector(buffer,16,1);
	BPB_FATSz32 = getValueFromBootSector(buffer,36,4);
	BPB_RootClus = getValueFromBootSector(buffer,44,4);
	BPB_RootEntCnt = getValueFromBootSector(buffer,17,2);
	BPB_TotSec32 = getValueFromBootSector(buffer,32,4);

	//FINDING THE ROOT DIRECTORY
	FATSz = BPB_FATSz32;
	RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec-1)) /
				BPB_BytsPerSec;

	//READ IN THE ROOT DIRECTORY
	NextClusterNumber = BPB_RootClus;
	FirstDataSector = BPB_ResvdSecCnt+(BPB_NumFATs*FATSz)+RootDirSectors;
	getCluster(CurrentCluster,buffer,NextClusterNumber,FirstDataSector,
		     BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec);		

	while(1){		
		presentWorkingDirectory(GET,PWD);
		printf("%s]: ",PWD);
		scanf("%s",command);
		currentClusterNumber(SET,NextClusterNumber);
		NextClusterNumber = parseInput(CurrentCluster,buffer,FirstDataSector,
				BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec,PWD,command);
		if(NextClusterNumber == 0) NextClusterNumber = BPB_RootClus;
		getCluster(CurrentCluster,buffer,NextClusterNumber,FirstDataSector,
			BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec);
	}

return 0;
}
                                                                                                                                                                                                                                                                                    util.c                                                                                              0000664 0001750 0001750 00000020717 12626373710 011525  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 #include "util.h"
#include "ls.h"
#include "open.h"
#include "cd.h"
#include "read.h"
/*
#include "close.h"
#include "rm.h"
#include "size.h"
#include "mkdir.h"
#include "rmdir.h"
#include "write.h"
*/

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
int isCommand(struct directory* cluster, unsigned char* buffer, 
		unsigned int FDS, unsigned int SPC, unsigned int RSC,
		unsigned int BPS, char* input, char* args)
{
	int dir_result = isDir(cluster,args);
	int file_result = isFile(cluster,args);
	int r;
	if(strcmp(input,"open") == 0){
		char* mode = calloc(sizeof(char),2);
		char* ptr  = calloc(sizeof(char),64);	
		ptr = strtok(args," ");
		mode = strtok(NULL," ");
		if(isFile(cluster,ptr)){
			open(args,mode);
		}else{
			printf("Error: File does not exist!\n");	
		}
		//free?
		return 1;
	}else if(strcmp(input,"close") == 0){
		close(args);
		return 1;
	}else if(strcmp(input,"create") == 0){
		return 1;
	}else if(strcmp(input,"rm") == 0){
		return 1;
	}else if(strcmp(input,"size") == 0){
		return 1;
	}else if(strcmp(input,"cd") == 0){
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
	}else if(strcmp(input,"ls") == 0){
		if(dir_result > 0){
			ls(buffer,args,dir_result,FDS,SPC, RSC, BPS);
		}
		else if(dir_result < 0){
			ls(buffer,args,currentClusterNumber(GET,0),FDS,SPC,RSC,BPS);
		}
		else 
			printf("%s: Invalid directory\n",args);
		return 1;
	}else if(strcmp(input,"mkdir") == 0){
		return 1;
	}else if(strcmp(input,"rmdir") == 0){
		return 1;
	}else if(strcmp(input,"read") == 0){
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
				printf("Error: attempt to read beyond EoF\n");	
			}else{
				read(buffer,file,start,num_bytes,currentClusterNumber(GET,0),FDS,SPC,RSC,BPS);
			}
		}else{
			printf("Error: file is not open for writing!\n");
		}
		return 1;
	}else if(strcmp(input,"write") == 0){
		return 1;
	}else if(strcmp(input,"exit") == 0){
		printf("Goodbye!\n");
		exit(1);
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



                                                 vector.c                                                                                            0000664 0001750 0001750 00000005327 12626362477 012062  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 /*NOTE:
 * Credit for the structure of vector.h and
 * vector.c is given to Happy Bear Software
 * Article via this link:
 * https://www.happybearsoftware.com/implementing-a-dynamic-array
*/

#include "vector.h"
#include "util.h"

void vector_init(Vector *vector) {
  // initialize size and capacity
  vector->size = 0;
  vector->capacity = VECTOR_INITIAL_CAPACITY;

  // allocate memory for vector->data
  vector->data = malloc(sizeof(char**) * vector->capacity);
  vector->data_mode = malloc(sizeof(char**) * vector->capacity);
}

void vector_append(Vector *vector, char* value, char* mode) {
  // make sure there's room to expand into
  vector_double_capacity_if_full(vector);

  // append the value and increment vector->size
//  vector->data[vector->size++] = value;
  vector->data[vector->size] = malloc(sizeof(char*)*vector->capacity);
  vector->data_mode[vector->size] = malloc(sizeof(char*)*vector->capacity);
  strcpy(vector->data[vector->size],value);
  strcpy(vector->data_mode[vector->size],mode);
  vector->size++;
}

char* vector_get(Vector *vector, int index) {
  if (index >= vector->size || index < 0) {
    printf("Index %d out of bounds for vector of size %d\n", index, vector->size);
    exit(1);
  }
  return vector->data[index];
}

char* vector_get_mode(Vector *vector, int index){
  if (index >= vector->size || index < 0) {
    printf("Index %d out of bounds for vector of size %d\n", index, vector->size);
    exit(1);
  }
  return vector->data_mode[index];
}

void vector_set(Vector *vector, int index, char* value) {
  // zero fill the vector up to the desired index
  while (index >= vector->size) {
    vector_append(vector,"","");
  }

  // set the value at the desired index
//  vector->data[index] = value;
  strcpy(vector->data[vector->size++],value);
}

void vector_double_capacity_if_full(Vector *vector) {
  if (vector->size >= vector->capacity) {
    // double vector->capacity and resize the allocated memory accordingly
    vector->capacity *= 2;
    vector->data = realloc(vector->data, sizeof(char**) * vector->capacity);
  }
}

int vector_get_size(Vector *vector)
{
	return vector->size;
}

void vector_remove(Vector *vector, char* value)
{
	int i, j;
	int flag = 0;
	for(i = 0; i < vector->size; i++){
		if(strcmp(vector->data[i],value) == 0){
			flag = 1;
			break;
		}
	}
	if(flag == 1){
		for(j = i; i < vector->size-1; i++){
			strcpy(vector->data[i],vector->data[i+1]);
			strcpy(vector->data_mode[i],vector->data_mode[i+1]);
		}
		vector->size--;
		vector->capacity = vector->size;
		vector->data = realloc(vector->data, sizeof(char**) * vector->capacity);
	}
}
void vector_free(Vector *vector) {
  for(int i = 0; i < vector->size; i++){
    free(vector->data[i]);
    free(vector->data_mode[i]);
  }
  free(vector->data);
}
                                                                                                                                                                                                                                                                                                         exec                                                                                                0000775 0001750 0001750 00000070057 12626406323 011255  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 ELF          >     @     @       �S          @ 8 	 @         @       @ @     @ @     �      �                   8      8@     8@                                          @       @     �A      �A                    N      N`     N`           p                    (N      (N`     (N`     �      �                   T      T@     T@     D       D              P�td   8;      8;@     8;@     4      4             Q�td                                                  R�td   N      N`     N`     �      �             /lib64/ld-linux-x86-64.so.2          GNU                       GNU �iA�j�x�	�� �ƽ��              (��           AM�|Z�;fUawכ|                            �                      H                                            2                      �                      �                      �                      7                      �                      ^                      e                      �                      �                      z                      �                      �                                            �                       s                      �                      �                      �                      P                                            l                      #                                            �     2@     7      �     �@     l       X     0R`                 �@     �        libc.so.6 strcpy exit fopen ftell __isoc99_scanf puts __stack_chk_fail putchar realloc stdin printf rewind strtok strtol fgets calloc strlen fseek fclose malloc strncat fread strchr _IO_getc strcmp __libc_start_main free __gmon_start__ GLIBC_2.7 GLIBC_2.4 GLIBC_2.2.5                                                  ii   �      ii   �      ui	         �O`                   0R`                   P`                    P`                   (P`                   0P`                   8P`                   @P`                   HP`                   PP`                   XP`        	           `P`        
           hP`                   pP`                   xP`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   �P`                   H��H��E  H��t�3  H���              �5�E  �%�E  @ �%�E  h    ������%�E  h   ������%�E  h   ������%�E  h   �����%�E  h   �����%�E  h   �����%�E  h   �����%�E  h   �p����%�E  h   �`����%zE  h	   �P����%rE  h
   �@����%jE  h   �0����%bE  h   � ����%ZE  h   �����%RE  h   � ����%JE  h   ������%BE  h   ������%:E  h   ������%2E  h   ������%*E  h   �����%"E  h   �����%E  h   �����%E  h   �����%
E  h   �p����%E  h   �`����%�D  h   �P����%�D  h   �@���1�I��^H��H���PTI��8@ H���7@ H���@ ������fD  �/R` UH-(R` H��H��w]ø    H��t�]�(R` ���    �(R` UH-(R` H��H��H��H��?H�H��u]ú    H��t�]H�ƿ(R` ���    �=�E   uUH���~���]�~E  ��@ H�=XA   t�    H��tU� N` H����]�{��� �s���UH��SH���  dH�%(   H�E�1��   �   ����H��@����   �   ����H��H����@   �   �����H��P����88@ �;8@ �g���H��X���H��X����   �    H���'���H��X���H�������H��`���H��X���H���R���H��`���H��H�ƿ   �z���H��h���H��`���H��X���H��h���H�Ѻ   H������H��X���H������H��h����   �   H���_  �,D  H��h����   �   H���@  �D  H��h����   �   H���!  ��C  H��h����   �   H���  ��C  H��h����   �$   H����  ��C  H��h����   �,   H����  ��C  H��h����   �   H���  ��C  H��h����   �    H���  �oC  �]C  ��4����YC  ���6C  Ѓ��+C  �    ��8����,C  ��0����C  ��4����C  �8���Љ�<����=�B  D��B  D��B  ��<�����0���H��h���H��p����<$H���  H��@���H�ƿ    �^  H��@���H�ƿE8@ �    �+���H��P���H�ƿK8@ �    ������0����ƿ   ��  D�UB  D�VB  �LB  ��<���H��h���H��p���H��P���H�|$H��@���H�<$H���^  ��0�����0��� u�B  ��0����=�A  D��A  D��A  ��<�����0���H��h���H��p����<$H���  ����UH��SH���  H������H��������|�����x���D��t���D��p�����  ��|�������t�����x���Љ�������|������    �u��p���Љ�������|������    �u������ǅ����    ǅ����    ��  �������P��H����¸   ��������������ȉ�Hc�H��H�H�H��H�H��H�}�H�H������H��0���H��������H��H����  H��0���H�H��8���H�CH��@���H�CH��H���H�CH��P���H�C H��X���H�C(��`����C0�������P��H���Hc�H��H�H�H��H�H��H�u�H�H-  � ����   �������P��H���H������Hc�H��H�H�H��H�H��H�� HȾd8@ H���������tG�������P��H���H������Hc�H��H�H�H��H�H��H�� HȾf8@ H��������uH�������P��H���H������Hc�H��H�H�H��H�H��H�� H�H�ƿi8@ �    ����������� �������  �����m8@ �j����   ������������Љ��������������������-������������Hc�H������H�� �������������������;�����}ŋ�������|�������;�|���t����;�|�������H���  []�UH���>  ��u�`R` �)  ��>     ]�UH��H�� H�}�H�u�H�E�n8@ H���I�����tPH�E�p8@ H���4�����t;H�E�r8@ H��������t&H�E�u8@ H���
�����t�x8@ �<����    �v�    �]����E�    �:�E��ƿ`R` ��  H��H�E�H��H���������u��8@ ������    �-�E��`R` �!  ;E��H�U�H�E�H�ƿ`R` �  �   ��UH��H�� H�}��E�    �<�E��ƿ`R` �X  H��H�E�H��H���G�����uH�E�H�ƿ`R` ��   ��E��`R` �   ;E����8@ �S�����UH��H�� H�}�H�E�H�ƿ�8@ �    �����   �   �����H�E��E�    �   �E�ƿ`R` ��  H��H�E�H��H��������u`�E�ƿ`R` ��  H�E�H�E��n8@ H��������t*H�E��r8@ H���r�����tH�E��u8@ H���]�����u�   �#�    ��E��`R` �  ;E��d����    ��UH��H�� H�}�   �   ����H�E��E�    �   �E�ƿ`R` ��  H��H�E�H��H���������u`�E�ƿ`R` �+  H�E�H�E��p8@ H��������t*H�E��r8@ H��������tH�E��u8@ H��������u�   �#�    ��E��`R` ��  ;E��d����    ��UH��H���E�    �%�E��ƿ`R` �>  H�ƿ�8@ �    ������E��`R` �  ;E�̿
   �0�����UH��SH���  H������H��������|�����x���D��t���D��p�����  ��|�������t�����x���Љ�������|������    �u��p���Љ�������|������    �u������ǅ����    ǅ����    ��  �������P��H����¸   ��������������ȉ�Hc�H��H�H�H��H�H��H�}�H�H������H��0���H��������H��H���  H��0���H�H��8���H�CH��@���H�CH��H���H�CH��P���H�C H��X���H�C(��`����C0�������P��H���H������Hc�H��H�H�H��H�H��H�� H�H������H��H���f�������  �������P��H���Hc�H��H�H�H��H�H��H�}�H�H-  � ��tC�������P��H�����Hc�H��H�H�H��H�H��H�u�H�H-  � ���N  ������ �������P��H����¸   ��������������ȉ�Hc�H��H�H�H��H�H��H�}�H�H������H��0���H��������H��H���  H��0���H�H��8���H�CH��@���H�CH��H���H�CH��P���H�C H��X���H�C(��`����C0�������P��H���Hc�H��H�H�H��H�H��H�u�H�H-0  �@
����    �������P��H���Hc�H��H�H�H��H�H��H�}�H�H-/  �@
��ȉ������������   ������ �������  �����   ������������Љ��������������������-������������Hc�H������H�� �������������������;�����}ŋ�������|�������;�|���t����;�|������������H���  []�UH��SH��   H�}��u��U��M�D�E�D�M�dH�%(   H�E�1��E����E��E�ЉE��E����    �u��E�ЉE��E����    �u��U��E�    �   �E��� ��H��@���H�M�H��H���_	  H��@���H�E�H��H���H�E�H��P���H�E�H��X���H�E�H��`���H�E�H��h���H�E؋�p����E�H�E�H�� �f8@ H���������u�E�����    �E���ЉE��E���    H�]�dH3%(   t�%���H�ĸ   []�UH��H��`H�}��u��U��M�D�E�D�M��E�    �E�    �E�H�H�ƿ   �N���H�E�H�E��  �   �   �4���H�E��  �E����E�E�ЉE��E����    �u �EЉE�E����    �u �U��E�    �E��E��H�   �E��E�Љ�H�E�H�� ��H�E��H�M�H�E�   H��H���s����E��E��E��E�;E�s�E�;E�~��   �E�E�ЉE�E���E���e��E�Hc�H�E�H�� ��Eԃm��E�;E�}ڋEԉE��E�    �E�)E��E�    ����;E�t����;E������H�E�H���U����   ��UH��SH���  H��x���H��p�����l�����h���D��d���D��`�����  ��d������E��`���Љ�������d������    �u �EЉ�������d������    �u ������ǅ����    ǅ����    ��  �������P��H����¸   ��������������ȉ�Hc�H��H�H�H��H�H��H�}�H�H������H�� ���H��x�����H��H���  H�� ���H�H��(���H�CH��0���H�CH��8���H�CH��@���H�C H��H���H�C(��P����C0�������P��H���H������Hc�H��H�H�H��H�H��H�� H�H��p���H��H���l�������  �������P��H����H�   ������������Ѓ� ��Hc�H��H�H�H��H�H��H�}�H�H������H�� ���H��x�����H��H���  H�� ���H�H��(���H�CH��0���H�CH��8���H�CH��@���H�C H��H���H�C(��P����C0�������P��H�����Hc�H��H�H�H��H�H��H�u�H�H-0  �@
����    �������P��H�����Hc�H��H�H�H��H�H��H�u�H�H-/  �@
��ȉ������������P��H�����Hc�H��H�H�H��H�H��H�u�H�H-0  �@�������������P��H�����Hc�H��H�H�H��H�H��H�]�H�H-/  �@��������������P��H�����Hc�H��H�H�H��H�H��H�}�H�H-.  �@��������������P��H�����Hc�H��H�H�H��H�H��H�u�H�H--  �@��ȉ������������ƿ�8@ �    ����D��`���D��������������h�����l���H��x����} �|$�}�|$�}�<$H������������������ u��8@ ������   �   ������ �������  �	����   ������������Љ��������������������-������������Hc�H��x���H�� �������������������;�����}ŋ�������d�������;�d���t����;�d�������H���  []�UH��H�}�u�U��E�    �E��U�Ѓ��E���e��E�Hc�H�E�H�� ��E��m��E�;E�}ڋE�]�UH��H�}�H�u��U܉M؋E؋U�Ѓ��E��E�    �'�E�Hc�H�E�HE�Hc�H�E�H�� ��m��E��E�;E�}�]�UH��H�}�H�u��U��E�    �E�    �E�Hc�H�E�H�� <.uB�'�E�Hc�H�E�HE�Hc�H�E�H�� ��E��E��E�Hc�H�E�H�� < u���   �E܃��E��D�E�Hc�H�E�H�� <v%�E�Hc�H�E�HE�Hc�H�E�H�� ��E���E�   �E��E܃�
;E�~�}� t��E܃��E��D�E�Hc�H�E�H�� <v%�E�Hc�H�E�HE�Hc�H�E�H�� ��E���E�   �E��E܃�.;E�~�}� t��}��E�Hc�H�E�H��  ]�UH��SH��xH�}�H�u��U�dH�%(   H�E�1��E�    �E�    �U�H�u�H�E��   H�������E��PH�E�H�pH�E��   H��������E��PH�E�H�pH�E��   H��������E��PH�E�H�pH�E��   H�������E��PH�E�H�pH�E��   H�������E��PH�E�H�pH�E��   H���w����E��PH�E�H�pH�E��   H���X����E��PH�E�H�pH�E��   H���9����E��PH�E�H�pH�E��   H�������E��PH�E�H�pH�E��   H��������E��PH�E�H�pH�E��   H��������E��PH�E�H�pH�E��   H�������U�H�E�H�H H�E�H��H��������E����E��}� wC�E�H�� 9@ ���E�    �4�E�   �+�E�   �"�E�   ��E�   ��E�   ��E�   H�E�H�U�H�H�U�H�PH�U�H�PH�U�H�PH�U�H�P H�U�H�P(�U��P0H�E�H�]�dH3%(   t�����H��x[]�UH��}��u��E���t��t
���*  ��E���*  �    � ]�UH��H�� �}�H�u��E��t'����   ����   H�E� Q` H���=�����   ��)  ��t.� Q` H������H�¸    H���H��H��H��H Q` f� / H�E�� ��tAH�E�H���(���H��H�E�H�ƿ Q` �d���H�E�H�������p)  Ѓ��e)  �c�a�[)  ���E��L�E�H��� Q` </u�}� t�E�H�ƀ Q`  �.�)(   �%�E�H�ƀ Q`  �)  ���)  �m��}� y����UH��SH��xH�}�H�uЉỦM�D�E�D�M��Ẽ��EĉE�ЉE�E����    �u�E�ЉE�E����    �u�U��E�    �   �E��P��H���Hc�H��H�H�H��H�H��H��H�E�H��   �E�E�Љ�H�E�H�M�H��H������H�E�H�H�E�H�CH�E�H�CH�E�H�CH�E�H�C H�E�H�C(�E��C0�E� �}��  �e����   �E�E�Љ�H�Eк   ��H�������E�H��x[]�UH��H�� H�}�H�E�
   H���_���H�E�H�}� tH�E��  ��UH��H��@H�}�H�u��U܉M�D�E�D�Mо@   �   �~���H�E�H�s'  H�������H�d'  H�E��@   H���C���H�E�H���k���D�M�D�EԋM؋U�H�u�H�E�H�}�H�|$H�}H�<$H���d   �E�}� t4�E��t��t�;H�E�H�ƿ   ������m8@ �   ������H�EH�ƿ(:@ �    �u����    �    �N�����UH��H��   H�}�H�u��U��M�D�E�D�M�H�UH�E�H��H���  �E�H�UH�E�H��H���r  �E��C:@ H�}�i�������   �   �   �B���H�EȾ@   �   �/���H�E�H�E�H:@ H������H�EоH:@ �    ����H�E�H�U�H�E�H��H����  ��tH�U�H�EH��H�������
�J:@ �����   ��  �f:@ H�}�������uH�EH���8����   �  �l:@ H�}������u
�   �  �s:@ H�}������u
�   �i  �v:@ H�}�e�����u
�   �M  �{:@ H�}�I�������   �}����   �}��uF�    �    ������D�E��}��M��U�H�E�E��A��H���(����ƿ   �}����   ��  �    �    �d�����D�M�D�E��M�H�uH�E��}��<$H�������E��}� ~�E��ƿ   �)����   �  H�EH�ƿ~:@ �    �!����   �m  ��:@ H�}�i�������   �}� ~&�U�D�M�D�E��M�H�uH�E��}��<$H���w����P�}� y4�    �    ������D�M�D�E��M�H�uH�E��}��<$H���=����H�EH�ƿ~:@ �    �����   ��  ��:@ H�}�������u
�   �  ��:@ H�}������u
�   �  ��:@ H�}�������?  �@   �   �n���H�Eؾ   �   �[���H�E�   �   �H���H�E�H�E�H:@ H�������H�E�H�E�H���"����E��}���   �H:@ �    ����H�E�H�M�H�E�
   H��H���+���H�E�H:@ �    �x���H�E�H�M�H�E�
   H��H�������H�E��   H�H;E���8@ ������Z�    �    �(�����H�E���H�E���D�M�H�u�H�E�D�E�D�D$D�E�D�D$D�E�D�$A��H���S����
��:@ �����   �D��:@ H�}�@�����u�   �+��:@ H�}�'�����u��:@ �Y����   �����    ��UH��H�� H�}�H�u��E�    �E�    ��   �E�Hc�H��H�H�H��H�H��H��H�E�H�H�P H�E�H��H����������   �E�H�H�PH��H�H�H��H�H��H��H�E�HЋ@0��tm�E�H�H�PH��H�H�H��H�H��H��H�E�H��@����    �E�H�H�PH��H�H�H��H�H��H��H�E�H��@��ȉE��E���E���E��}������E���UH��H�� H�}�H�u��E�    H�E�d8@ H���������u
������m  H�E�f8@ H��������u[H�E�H��hH�P H�E�H��H��������u2H�E�H��h�@����    H�E�H��h�@��ЉE��E��  �������   �E�    ��   �E�Hc�H��H�H�H��H�H��H��H�E�H�H�P H�E�H��H���	�������   �E�H�H�PH��H�H�H��H�H��H��H�E�HЋ@0��um�E�H�H�PH��H�H�H��H�H��H��H�E�H��@����    �E�H�H�PH��H�H�H��H�H��H��H�E�H��@��ȉE��E���E���E��}������E���UH��H��H�}�H�E��     H�E��@d   H�E��@H�H��H���W���H��H�E�H�PH�E��@H�H��H���7���H��H�E�H�P��UH��SH��(H�}�H�u�H�U�H�E�H����  H�E�H�PH�E� H�H��H�H�E�@H�H��H�������H�H�E�H�PH�E� H�H��H�H�E�@H�H��H������H�H�E�H�PH�E� H�H��H�H� H�U�H��H���p���H�E�H�PH�E� H�H��H�H� H�U�H��H���G���H�E� �PH�E�H��([]�UH��H��H�}��u�H�E�� ;E�~�}� y$H�E���E�ƿ;@ �    �g����   �m���H�E�H�@�U�Hc�H��H�H� ��UH��H��H�}��u�H�E�� ;E�~�}� y$H�E���E�ƿ;@ �    �	����   ����H�E�H�@�U�Hc�H��H�H� ��UH��H�� H�}��u�H�U��H�E��6;@ �6;@ H���0���H�E�� ;E�~�H�E�H�pH�E�� �HH�U��
H�H��H�H� H�U�H��H��������UH��H��H�}�H�E��H�E��@9�|=H�E��@� H�E��PH�E��@H�H��    H�E�H�@H��H�������H�U�H�B��UH��H�}�H�E�� ]�UH��H�� H�}�H�u��E�    �E�    �8H�E�H�@�U�Hc�H��H�H� H�U�H��H���(�����u	�E�   ��E�H�E� ;E���}���   �E�E��   H�E�H�@�U�Hc�H��H��H�H�H�E�H�@�M�Hc�H��H�H� H��H�������H�E�H�@�U�Hc�H��H��H�H�H�E�H�@�M�Hc�H��H�H� H��H�������E�H�E� ��;E��l���H�E� �P�H�E�H�E�H�E�PH�E�@H�H��    H�E�H�@H��H������H�U�H�B��UH��H�� H�}��E�    �DH�E�H�@�U�Hc�H��H�H� H�������H�E�H�@�U�Hc�H��H�H� H��������E�H�E� ;E��H�E�H�@H�������� AWA��AVI��AUI��ATL�%X  UH�-X  SL)�1�H��H���E���H��t�     L��L��D��A��H��H9�u�H��[]A\A]A^A_�ff.�     ��  H��H���        �       ������rb fat32.img %s]:  %s      �       ������. .. %s   r w rw wr Error: incorrect parameter! Error: File already open! Error: file not open fileModeIsReadable => %s
 %s,  file size = 0x%08X
   Error: attempt to read beyond EoF       i'@     3'@     <'@     i'@     E'@     i'@     i'@     i'@     N'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     W'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     i'@     `'@     %s is not a valid command
 open   Error: File does not exist! close create rm size cd %s: Invalid directory
 ls mkdir rmdir read        Error: file is not open for writing! write exit Goodbye!           �       ������    Index %d out of bounds for vector of size %d
   ;4  %   ����  ����P  �����  H����  z����  ����  ����0  ����P  ����p  �����  �����  /����  m���   ����   1���H  ����h  �����  &����  �����  �����  ����  ���8  M���X  D���x   ����  ����  �����  ����  ���   e���@  ����`  -����  �����  �����  �����  h���   ����H             zR x�      p���*                  zR x�  $      �����   FJw� ?;*3$"       D   ����   A�CH�    $   d   x���2   A�CH�%         �   ����$    A�C_      �   �����    A�C�     �   S���l    A�Cg     �   �����    A�C�       c����    A�C�     ,  ���Q    A�CL  $   L  B���5   A�CH�(      $   t  O���>   A�CH�1         �  e����   A�C� $   �  ����7   A�CH�*         �  ����P    A�CK       ���Z    A�CU     $  K���K   A�CF $   D  v���j   A�CE�`         l  ����2    A�Cm      �  ����(   A�C# $   �  ����2   A�CE�(         �  ����1    A�Cl      �  �����    A�C�       �����   A�C�    4  `���   A�C    T  V����   A�C�    t  ����c    A�C^  $   �  ����    A�CE��          �  ����^    A�CY     �  ���^    A�CY     �  [���j    A�Ce       ����\    A�CW     <  ����    A�CK      \  ����V   A�CQ    |  ���v    A�Cq  D   �  `���e    B�E�E �E(�H0�H8�M@l8A0A(B BBB    �  ����                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   �@     �@                                  
@            8@            N`                          N`                   ���o    �@            �@            �@     
                                                  P`            �                           �@            `@            0       	              ���o     @     ���o           ���o    �@                                                                                                             (N`                     V
@     f
@     v
@     �
@     �
@     �
@     �
@     �
@     �
@     �
@     �
@     @     @     &@     6@     F@     V@     f@     v@     �@     �@     �@     �@     �@     �@     �@     �@                                                     /                                                                                                                                                                                                                                                                  GCC: (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4 GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2  .symtab .strtab .shstrtab .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .text .fini .rodata .eh_frame_hdr .eh_frame .init_array .fini_array .jcr .dynamic .got .got.plt .data .bss .comment                                                                                     8@     8                                    #             T@     T                                     1             t@     t      $                              D   ���o       �@     �      4                             N             �@     �                                 V             �@     �                                   ^   ���o       �@     �      @                            k   ���o        @            @                            z             `@     `      0                            �             �@     �      �                          �             
@     
                                    �             @
@     @
      �                            �              @            ,                             �             8@     8      	                              �              8@      8                                   �             8;@     8;      4                             �             p<@     p<      ,                             �             N`     N                                    �             N`     N                                    �              N`      N                                    �             (N`     (N      �                           �             �O`     �O                                   �              P`      P      �                             �              Q`      Q      $                              �             0R`     $R      P                              �      0               $R      M                                                   qR                                                          [      @         K                 	                      @i      �                                                           8@                   T@                   t@                   �@                   �@                   �@                   �@                    @                  	 `@                  
 �@                   
@                   @
@                    @                   8@                    8@                   8;@                   p<@                   N`                   N`                    N`                   (N`                   �O`                    P`                    Q`                   0R`                                       ��                      N`                  0@             .     `@             A     �@             W     8R`            f     N`             �     �@             �     N`             �    ��                �     $8@            �     (8@            �     ,8@            �     08@            �     48@            �     <R`            �     @R`                DR`            $    HR`            5    LR`            F    PR`            X    TR`            l    XR`            ~   ��                �     P8@            �     T8@            �     X8@            �     \8@            �     `8@            �    `R`            �    xR`            �    |R`            �     Q`            �     R`            �   ��                �     �:@            �     �:@            �     �:@            �     �:@            �      ;@                ��                �    �A@             �     N`                  ��                �     N`             �    (N`             �     N`             	     P`                 8@            /                     A                     V    g@     >      ^    �2@     c       j                      �      Q`             �    ^%@     j      �                     �    �@     l       �                     �                     �    0R`            �    i#@     P           �5@            "    $R`             )    2@     5      ,                     @    '7@     v       L    8@             R                     f                     �    ?4@     ^       �                     �                     �                     �    �*@     �       �    �'@     (      �    ")@     2      �    �4@     ^           T*@     1           |+@     �      '    �@     2      *                     ?                     ^                     q                     �     Q`             �                     �    /@     �       �                     �                      �                     �   Q`             �     8@                2@     7          �@     $           �@     �          V3@     �       -    �7@     e       =    �@     Q       R                     f                     |    @     �       �    �R`             �     @             �                     �    80@           �    �5@     V      �                     �    $R`             �    �'@     2       �    �@     �      �    �#@     Z           N1@     �      !                     4                     H                      \                     v    �@     �       {    �4@     j       �                     �   (R`             �                      �    e5@     \       �    
@             �    $@     K       crtstuff.c __JCR_LIST__ deregister_tm_clones register_tm_clones __do_global_dtors_aux completed.6973 __do_global_dtors_aux_fini_array_entry frame_dummy __frame_dummy_init_array_entry p3.c SIZE_OF_SECTOR FREE_DIR ZERO_DIR EOC EOC2 BPB_BytsPerSec.2266 BPB_SecPerClus.2267 BPB_ResvdSecCnt.2268 BPB_NumFATs.2269 BPB_FATSz32.2270 BPB_RootClus.2271 BPB_RootEntCnt.2272 BPB_TotSec32.2273 util.c open_file_table is_init CCN.2491 PWD.2498 size.2499 vector.c __FRAME_END__ __JCR_END__ __init_array_end _DYNAMIC __init_array_start _GLOBAL_OFFSET_TABLE_ __libc_csu_fini free@@GLIBC_2.2.5 putchar@@GLIBC_2.2.5 cd_back vector_init _ITM_deregisterTMCloneTable data_start getDirectoryInformation strcpy@@GLIBC_2.2.5 close puts@@GLIBC_2.2.5 fread@@GLIBC_2.2.5 stdin@@GLIBC_2.2.5 getValueFromBootSector vector_get_size _edata cd fclose@@GLIBC_2.2.5 vector_free _fini strlen@@GLIBC_2.2.5 __stack_chk_fail@@GLIBC_2.4 vector_get strchr@@GLIBC_2.2.5 printf@@GLIBC_2.2.5 rewind@@GLIBC_2.2.5 parseInput presentWorkingDirectory getCluster vector_get_mode removeTrailingNewline isCommand ls strncat@@GLIBC_2.2.5 __libc_start_main@@GLIBC_2.2.5 fgets@@GLIBC_2.2.5 calloc@@GLIBC_2.2.5 __data_start strcmp@@GLIBC_2.2.5 fileModeIsReadable ftell@@GLIBC_2.2.5 __gmon_start__ strtol@@GLIBC_2.2.5 __dso_handle _IO_stdin_used read init read_file vector_append __libc_csu_init dump_open_file_table malloc@@GLIBC_2.2.5 _IO_getc@@GLIBC_2.2.5 fileModeIsWriteable _end _start fseek@@GLIBC_2.2.5 isFile vector_remove realloc@@GLIBC_2.2.5 __bss_start currentClusterNumber main getValueFromDirectorySector isDir fopen@@GLIBC_2.2.5 strtok@@GLIBC_2.2.5 _Jv_RegisterClasses __isoc99_scanf@@GLIBC_2.7 open vector_set exit@@GLIBC_2.2.5 __TMC_END__ _ITM_registerTMCloneTable vector_double_capacity_if_full _init getFileName                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  run                                                                                                 0000700 0001750 0001750 00000000143 12625506701 011106  0                                                                                                    ustar   connal                          connal                                                                                                                                                                                                                 gcc -o exec p3.c util.c vector.c -std=c99
if [ $? -eq 0 ]; then
    ./exec
else
    echo FAILED
fi
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             