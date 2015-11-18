#include "util.h"

int main()
{
	FILE *fileptr;
	unsigned char *buffer;
	long filelen;
	unsigned int BPB_BytsPerSec, BPB_SecPerClus, BPB_ResvdSecCnt,
			BPB_NumFATs, BPB_FATSz32, BPB_RootClus,
			BPB_RootEntCnt, BPB_TotSec32;
	unsigned int RootDirSectors, FATSz, FirstSectorofCluster, 
			FirstDataSector, ThisFATSecNum, ThisFATEntOffset,
			DataSec, CountofClusters;
	unsigned int NextClusterNumber;
	struct directory current_dir;

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

	//LOOP TO ACCOUNT FOR A DIRECTORY THAT SPANS MORE THAN ONE CLUSTER
	NextClusterNumber = BPB_RootClus;
	while(NextClusterNumber != EOC){
		FirstDataSector = BPB_ResvdSecCnt+(BPB_NumFATs*FATSz)+RootDirSectors;
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		ThisFATSecNum = BPB_ResvdSecCnt+((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;
		for(int i = 0; i < 512; i+=32){
			current_dir = getDirectoryInformation(buffer,
							FirstSectorofCluster*SIZE_OF_SECTOR+i);
		}
		NextClusterNumber = getValueFromBootSector(buffer,
							(ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset,
							4);
	}



return 0;
}
