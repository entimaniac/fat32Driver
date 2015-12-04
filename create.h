#include <ctype.h>
#include "util.h"

void shortName(unsigned char *s, char* filename)
{
	int c = 0;
	int f = 0;
	for(int i = 0; i < 32; ++i){
		s[i] = (unsigned char)0x0;
	}
	s[c++] = 0x41; //A
	for( ; c <= 10 && f < strlen(filename); ++c){
		if(c%2 == 1){
			s[c] = filename[f++];
		}
	}
	s[c++] = (unsigned char)0x0F;
	for( ; c <= 13; ++c){
	}
	for( ; c <= 31 && f < strlen(filename); ++c){
		if(c%2 == 0){
			s[c] = filename[f++];
		}
	}
}

void longName(unsigned char *s, char* filename, unsigned int FstClusLO)
{
	int c = 0;
	int f = 0;
	for(int i = 0; i < 32; i++){
		s[i] = (unsigned char)0x0;
	}
	//DIR_Name
	for( ; c <= 10 && f < strlen(filename); ++c){
		if(filename[f] != '.'){
			s[c] = (unsigned char)toupper(filename[f++]);	
		}
		else{ 
			s[c] = (unsigned char)0x20;
			f++;
		}
	}
	//DIR_Attribute
	s[11] = (unsigned char)0x20;
	for( ; c <= 25; ++c){
	}
	s[c++] = (FstClusLO & 0x00FF); //bits 1 and 0 of value;
	s[c++] = (((FstClusLO & 0xFF00)/16)/16); //bits 3 and 2 of value;
}

int fileExists(unsigned char* buffer, char *filename, unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n;

	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		int val = 0;
		for(int i = 0; i < 512; i += 32){
			cluster[i/32] = getDirectoryInformation(buffer,
						FirstSectorofCluster*SIZE_OF_SECTOR+i);	
			if(strcmp(cluster[i/32].filename,filename) == 0)
			{
				return 1;
			}
		} 

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
	}
	return 0;
	//end of function

}

int checkIfClusterIsFull(unsigned char* buffer, unsigned int NextClusterNumber, unsigned int FDS,
			 	unsigned int SPC, unsigned int RSC, unsigned int BPS)
{
	unsigned int val;
	struct directory dir;
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n;
	unsigned int num;
	int filename_flag = 0;

	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*SPC)+FDS;
		ThisFATSecNum = RSC + ((4*NextClusterNumber) / BPS);
		ThisFATEntOffset = (4*NextClusterNumber) % BPS;		

		int val = 0;
		for(int i = 0; i < 512; i += 32){
			dir = getDirectoryInformation(buffer,
					FirstSectorofCluster*SIZE_OF_SECTOR+i);
			for(int j = 0; j < 32 && filename_flag == 0; j++){
				if(buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i+j] != (unsigned char)0x0) 
					filename_flag = 1;
			}
			if(filename_flag == 0){
				return 0;
			}
			filename_flag = 0;
		}

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
	}
	return 1;
}

int create(unsigned char* buffer, char* filename, int DataCluster,
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n, return_val, m;
	unsigned int num;
	unsigned int size;
	unsigned char shortn[32];
	unsigned char longn[32];
	int filename_flag = 0;

	if(fileExists(buffer,filename,NextClusterNumber,FirstDataSector,BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec) == 1){
		return 0;
	}
	if(DataCluster < 0){
		return -1;
	}
	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		int val = 0;
		for(int i = 0; i < 512; i += 32){
			cluster[i/32] = getDirectoryInformation(buffer,
						FirstSectorofCluster*SIZE_OF_SECTOR+i);
			for(int j = 0; j < 32 && filename_flag == 0; j++){
				if(buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i+j] != (unsigned char)0x0){
					filename_flag = 1;
				}
			}
			if(filename_flag == 0){
				shortName(shortn,filename);
				longName(longn,filename,DataCluster);	
				for(int b1 = FirstSectorofCluster*SIZE_OF_SECTOR+i,
					b2 = FirstSectorofCluster*SIZE_OF_SECTOR+i+32, j = 0; 
					j < 32; ++b1, ++b2, ++j)
				{
					buffer[b1] = shortn[j];
					buffer[b2] = longn[j];						
				}
				return FirstSectorofCluster*SIZE_OF_SECTOR+i;
			}
			filename_flag = 0;
		}

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
	}
	//end of function
}

int extendClusterChain(unsigned char* buffer, unsigned int NewClusterNumber,
        		unsigned int ClusterNumber, unsigned int FirstDataSector, 
			unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int ThisFATSecNum, ThisFATEntOffset, FirstSectorofCluster;
	unsigned int PrevClusterNumber;
	unsigned int n, m;

	while(ClusterNumber != EOC && ClusterNumber != EOC2){
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*ClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*ClusterNumber) % BPB_BytsPerSec;		
		PrevClusterNumber = ClusterNumber;

		int val = 0;
		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		ClusterNumber = val;
	}

	//WRITE NEW CLUSTER NUMBER WHERE THE CLUSTER CHAIN PREVIOUSLY ENDED IN FAT TABLE
	ThisFATSecNum = BPB_ResvdSecCnt + ((4*PrevClusterNumber) / BPB_BytsPerSec);
	ThisFATEntOffset = (4*PrevClusterNumber) % BPB_BytsPerSec;
	m = ThisFATSecNum*SIZE_OF_SECTOR+ThisFATEntOffset;
	buffer[m] =   (NewClusterNumber & 0x000000FF);
	buffer[m+1] = (NewClusterNumber & 0x0000FF00)/16/16;
	buffer[m+2] = (NewClusterNumber & 0x00FF0000)/16/16/16/16;
	buffer[m+3] = (NewClusterNumber & 0xFF000000)/16/16/16/16/16/16;

	//WRITE NEW ENDING PLACE FOR CLUSTER CHAIN IN FAT TABLE
	ThisFATSecNum = BPB_ResvdSecCnt + ((4*NewClusterNumber) / BPB_BytsPerSec);
	ThisFATEntOffset = (4*NewClusterNumber) % BPB_BytsPerSec;
	n = ThisFATSecNum*SIZE_OF_SECTOR+ThisFATEntOffset;
	buffer[n] = 0xF8;
	buffer[n+1] = 0xFF;
	buffer[n+2] = 0xFF;
	buffer[n+3] = 0X0F;

	return 1;
}
