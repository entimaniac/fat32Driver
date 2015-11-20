#include "util.h"
/*
#include "open.h"
#include "close.h"
#include "rm.h"
#include "size.h"
#include "ls.h"
#include "mkdir.h"
#include "rmdir.h"
#include "read.h"
#include "write.h"
*/

unsigned int getValueFromBootSector(unsigned char* buffer, int offset, int size)
{
	unsigned val = 0;
	for(int i = offset+size-1; i >= offset; i--){
//		printf("%02X ",buffer[i]);
		val = val << 8;
		val += buffer[i];
	}
//	printf("\n");
	return val;
}

void getValueFromDirectorySector(unsigned char* buffer, unsigned char* DIR_val, int offset, int size)
{
	for(int i = offset+size-1, j = 0; i >= offset; i--, j++){
		DIR_val[j] = buffer[i];	
//		printf("%02X ",DIR_val[j]);	
	}
//	printf("\n");
}

struct directory getDirectoryInformation(unsigned char* buffer, int start)
{
	struct directory d;
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
/*	
	if(d.DIR_Name[10] == FREE_DIR){
		printf("Directory is free\n");
	}else if(d.DIR_Name[10] == ZERO_DIR){
		printf("Directory contains only zeros\n");
	}else{
		if(d.Attribute != ATTR_LONG_NAME){
			if(d.Attribute == ATTR_DIRECTORY){
				printf("Directory is in use: ");
			}else printf("File is in use: ");
			for(int i = 10; i >= 0; i--){
				printf("%c",d.DIR_Name[i]);
			}printf("\n");
		}
	}
*/
	return d;
}

void getCluster(struct directory* CurrentCluster, unsigned char* buffer, 
		  unsigned int NextClusterNumber, unsigned int FirstDataSector,
		  unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,
		  unsigned int BPB_BytsPerSec)
{
  unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;

  while(NextClusterNumber != EOC){
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

}

void parseInput(char* PWD, char* command)
{
	//if isCommand(command)
	  //call function from whichever command.c necessary
	//else
	  //error
}
int isCommand(char* input)
{

}
int isFile(char* input)
{

}
int isDir(char* input)
{

}


