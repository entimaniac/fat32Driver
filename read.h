#include "util.h"

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
		for(int i = start; i < start+size; i++){
			temp[0] = buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i];	
			strncat(output,temp,1);
			bytes_gotten++;
			total_gotten++;
			if(total_gotten == num_bytes){
				printf("%s\n",output);
				return 1;
			}
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
	return 0;
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

