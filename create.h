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
//		else{
//			s[c] = (unsigned char)0x0;
//		}
	}
	s[c++] = (unsigned char)0x0F;
	for( ; c <= 13; ++c){
//		s[c] = (unsigned char)0x0;
	}
	for( ; c <= 31 && f < strlen(filename); ++c){
		if(c%2 == 0){
			s[c] = filename[f++];
		}
//		else{
//			s[c] = (unsigned char)0x0;
//		}
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
//		s[c++] = (unsigned char)0x0;
	}
	s[c++] = (((FstClusLO & 0xFF00)/16)/16); //bits 3 and 2 of value;
	s[c++] = (FstClusLO & 0x00FF); //bits 1 and 0 of value;
	for( ; c <= 31; ++c){
//		s[c++] = (unsigned char(0x0;
	}
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

int create(unsigned char* buffer, char* filename, int DataCluster,
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n, return_val;
	unsigned int num;
	unsigned int size;
	unsigned char shortn[32];
	unsigned char longn[32];

	if(fileExists(buffer,filename,NextClusterNumber,FirstDataSector,BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec) == 1){
		return 0;
	}
//	printf("First NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		int val = 0;
		for(int i = 0; i < 512; i += 32){
			cluster[i/32] = getDirectoryInformation(buffer,
						FirstSectorofCluster*SIZE_OF_SECTOR+i);
			if(cluster[i/32].filename[0] == (char)0x0){
				shortName(shortn,filename);
				longName(longn,filename,DataCluster);	
				for(int b1 = FirstSectorofCluster*SIZE_OF_SECTOR+i,
					b2 = FirstSectorofCluster*SIZE_OF_SECTOR+i+32, j = 0; 
					j < 32; ++b1, ++b2, ++j)
				{
					buffer[b1] = shortn[j];
					buffer[b2] = longn[j];						
				}
				printf("long attr = 0x%2X\n",longn[11]);
				return FirstSectorofCluster*SIZE_OF_SECTOR+i;
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
	return -1;
	//end of function
}

