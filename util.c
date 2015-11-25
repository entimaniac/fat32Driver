#include "util.h"
#include "ls.h"
/*
#include "open.h"
#include "close.h"
#include "rm.h"
#include "size.h"
#include "mkdir.h"
#include "rmdir.h"
#include "read.h"
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
	static unsigned int CCN;
	switch(mode){
		case GET:{return CCN;}
		
		case SET:{
			CCN = num;
		return 0;}
	}
}

void presentWorkingDirectory(MODE mode, char* dir){
	static unsigned char PWD[256] = "";
	switch(mode){
		case GET:{
			strncpy(dir,PWD,strlen(PWD));
			break;}
		case ADD:{
			strncat(PWD,"/",1);
			strncat(PWD,dir,strlen(dir));
			break;}
		case SUB:{
			strncpy(PWD,PWD,strlen(PWD)-strlen(dir)-1);
			break;}
	}
}
void getCluster(struct directory* CurrentCluster, unsigned char* buffer, 
		  unsigned int NextClusterNumber, unsigned int FirstDataSector,
		  unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,
		  unsigned int BPB_BytsPerSec)
{
  unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;

//  while(NextClusterNumber != EOC){

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
//  }
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
	int result = isDir(cluster,args);
	if(strcmp(input,"open") == 0){
		return 1;
	}else if(strcmp(input,"close") == 0){
		return 1;
	}else if(strcmp(input,"create") == 0){
		return 1;
	}else if(strcmp(input,"rm") == 0){
		return 1;
	}else if(strcmp(input,"size") == 0){
		return 1;
	}else if(strcmp(input,"cd") == 0){
		if(result > 0){
			currentClusterNumber(SET,result);
			return 2;
		}else if(result == -1){ //current dir
			//dont change currentClusterNumber	
		}else if(result == -2){
			//cd(buffer,args,result,FDS,SPC,RSC,BPS);
			//return 3;	
		}else{
			printf("%s: Invalid directory\n",args);
		}
		return 1;
	}else if(strcmp(input,"ls") == 0){
		if(result > 0){
			ls(buffer,args,result,FDS,SPC, RSC, BPS);
		}
		else if(result < 0){
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



