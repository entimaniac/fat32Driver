#include <ctype.h>
#include "util.h"

void makeDotDirectories(unsigned char* buffer, unsigned int start, 
			unsigned int DotClusterNumber, unsigned int DotDotClusterNumber)
{
	//printf("making dot directories at 0x%X\n",start*SIZE_OF_SECTOR);
	//DOT
	int n = start*SIZE_OF_SECTOR;
	buffer[n++] = (unsigned char)0x2E;
	for( ; n <= 10+start*SIZE_OF_SECTOR; ++n){
		buffer[n] = (unsigned char)0x20;
	}
	buffer[n++] = (unsigned char)0x10;
	for( ; n <= 25+start*SIZE_OF_SECTOR; ++n){
		buffer[n] = (unsigned char)0x0;
	}
	buffer[n++] = (DotClusterNumber & 0x00FF);
	buffer[n++] = (DotClusterNumber & 0xFF00)/16/16;
	for( ; n <= 31+start*SIZE_OF_SECTOR; ++n){
		buffer[n] = (unsigned char)0x0;
	}
	//DOTDOT
	buffer[n++] = (unsigned char)0x2E;
	buffer[n++] = (unsigned char)0x2E;
	for( ; n <= 10+32+start*SIZE_OF_SECTOR; ++n){
		buffer[n] = (unsigned char)0x20;
	}	
	buffer[n++] = (unsigned char)0x10;
	for( ; n <= 25+32+start*SIZE_OF_SECTOR; ++n){
		buffer[n] = (unsigned char)0x0;
	}
	buffer[n++] = (DotDotClusterNumber & 0x00FF);
	buffer[n++] = (DotDotClusterNumber & 0xFF00)/16/16;
	for( ; n <= 31+32+start*SIZE_OF_SECTOR; ++n){
		buffer[n] = (unsigned char)0x0;
	}
}

void shortDirName(unsigned char *s, char* filename)
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
	//printf("about to assign short attr c = %d\n",c);	
	s[c++] = (unsigned char)0x00;
	for( ; c <= 13; ++c){
	}
	for( ; c <= 31 && f < strlen(filename); ++c){
		if(c%2 == 0){
			s[c] = filename[f++];
		}
	}
}

void longDirName(unsigned char *s, char* filename, unsigned int FstClusLO)
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
	//printf("about to assign long attr c = %d\n",c);	
	s[11] = (unsigned char)0x10;
	c = 12;
	for( ; c <= 25; ++c){
	}
	s[c++] = (FstClusLO & 0x00FF); //bits 1 and 0 of value;
	s[c++] = (((FstClusLO & 0xFF00)/16)/16); //bits 3 and 2 of value;
}

int mkdir(unsigned char* buffer, char* filename, int DataCluster,
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
				shortDirName(shortn,filename);
				longDirName(longn,filename,DataCluster);	
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
