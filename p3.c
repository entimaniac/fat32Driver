#include "util.h"

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

	strcpy(PWD,"/");	
	while(1){		
		printf("%s]: ",PWD);
		scanf("%s",command);
		currentClusterNumber(SET,NextClusterNumber);
		parseInput(CurrentCluster,buffer,FirstDataSector,
				BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec,PWD,command);	
		getCluster(CurrentCluster,buffer,NextClusterNumber,FirstDataSector,
				BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec);
	}

return 0;
}
