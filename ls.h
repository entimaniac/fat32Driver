#include "util.h"

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

