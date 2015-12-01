#include "util.h"

int write_file(unsigned char* buffer, int start, int num_bytes, unsigned char* string ,unsigned int size, 
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n, bytes_gotten = 0;
	char *output = calloc(sizeof(char),num_bytes);
	strcpy(output,"");
	char *temp = calloc(sizeof(char),1);

//	printf("First NextClusterNumber is %d or 0x%X\n\n",NextClusterNumber,NextClusterNumber);
	

	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		



		int val = 0;
		for(int i = start; i < start+num_bytes; i++){
			buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i] = string[i];
			temp[0] = buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i];
			//printf("%s\n", temp[0]);
			if(temp[0] != 0x0){
				strncat(output,temp,1);
				bytes_gotten++;
			}else{
				return 0;
			}
		}

		//printf("%d\n", bytes_gotten );

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
	printf("Write success!\n");
	return 1;
	//end of function
}

void write(unsigned char* buffer, char* dir, int start, int num_bytes, unsigned char* string,
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

				int new_size = start + num_bytes;

				size = ((int)cluster[(i/32)+1].DIR_FileSize[0] *16*16*16*16*16*16) +
					((int)cluster[(i/32)+1].DIR_FileSize[1] *16*16*16*16) +
					((int)cluster[(i/32)+1].DIR_FileSize[2] *16*16) +
					((int)cluster[(i/32)+1].DIR_FileSize[3]);

				if(new_size > size){
					//cluster[(i/32)+1].DIR_FileSize[i] = new_size >> (8^i)
					for(int j = 3; j > -1; j--){
					    int digit = new_size % 256;
					    buffer[FirstSectorofCluster*SIZE_OF_SECTOR+i+32+28+3-j] = digit;
					    new_size /= 256;
					}
				} 

				cluster[(i/32)+1].DIR_FileSize[3] = num_bytes;

				return_val = write_file(buffer,start,num_bytes,string,size,num,FirstDataSector,
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

