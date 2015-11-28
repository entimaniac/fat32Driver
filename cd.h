#include "util.h"

int cd(unsigned char* buffer, char* dir, unsigned int NextClusterNumber, 
		unsigned int FirstDataSector, unsigned int BPB_SecPerClus,
		unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n;

	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

		val = 0;
		for(int i = 0; i < 512; i += 32){
			cluster[i/32] = getDirectoryInformation(buffer,
						FirstSectorofCluster*SIZE_OF_SECTOR+i);	
			if(strcmp(cluster[i/32].filename,dir) == 0)
			{
				cluster[i/32+1] = getDirectoryInformation(buffer,
						     FirstSectorofCluster*SIZE_OF_SECTOR+i+32);
				if(cluster[i/32].Attribute == ATTR_DIRECTORY ||
				   cluster[(i/32)+1].Attribute == ATTR_DIRECTORY)
				{
					val = ((int)cluster[i/32+1].DIR_FstClusLO[0] << 2) +
						((int)cluster[i/32+1].DIR_FstClusLO[1]);
					return val;
				}
			}	
	} 

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
	}
	return -1;
	//end of function
}

int cd_back(unsigned char* buffer, unsigned int NextClusterNumber, 
		unsigned int FirstDataSector, unsigned int BPB_SecPerClus,
		unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	struct directory dir;	
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	unsigned int val;

	FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;	
	ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
	ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		

	val = 0;
	dir = getDirectoryInformation(buffer,
				FirstSectorofCluster*SIZE_OF_SECTOR+32);	
	if(strcmp(dir.filename,"..") == 0)
	{	
		val = ((int)dir.DIR_FstClusLO[0] << 2) +
			((int)dir.DIR_FstClusLO[1]);
		return val;
	}	
	return 0;
	//end of funciton
}

