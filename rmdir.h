#include "util.h"

int rmdir_file(unsigned char* buffer, 
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val, location;
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	unsigned int n;
	struct directory dir;

	stack fat_stack;
	stack data_stack;
	stack_init(&fat_stack); 
	stack_init(&data_stack);

	//COLLECT LIST OF LOCATIONS TO ZERO OUT
	while(NextClusterNumber != EOC && NextClusterNumber != EOC2){
		FirstSectorofCluster = ((NextClusterNumber-2)*BPB_SecPerClus)+FirstDataSector;
		ThisFATSecNum = BPB_ResvdSecCnt + ((4*NextClusterNumber) / BPB_BytsPerSec);
		ThisFATEntOffset = (4*NextClusterNumber) % BPB_BytsPerSec;		
		stack_append(&data_stack,FirstSectorofCluster*SIZE_OF_SECTOR);

		dir = getDirectoryInformation(buffer,
				FirstSectorofCluster*SIZE_OF_SECTOR);	
		if(dir.filename[0] != (unsigned char)0x0){
			return 0;		
		}

		n = (ThisFATSecNum*SIZE_OF_SECTOR)+ThisFATEntOffset;
		stack_append(&fat_stack,n);
		for(int i = n+3; i >= n; i--){
			val = val << 8;	
			val = val + buffer[i];
		}
		NextClusterNumber = val;
	}

	for(int i = 0; i < stack_get_size(&fat_stack); i++){
		location = stack_get(&fat_stack,i);
		for(int j = location; j < location+4; j++){
//			printf("zeroing out 0x%X\n",j);
			buffer[j] = (unsigned char)0x00;
		}		
	}
	for(int i = 0; i < stack_get_size(&data_stack); i++){
		location = stack_get(&data_stack,i);
		for(int j = location; j < location+512; j++){
//			printf("zeroing out 0x%X\n",j);
			buffer[j] = (unsigned char)0x00;
		}
	}

	return 1;
	//end of function
}

int rmdir(unsigned char* buffer, char* filename, 
        unsigned int NextClusterNumber, unsigned int FirstDataSector, 
	unsigned int BPB_SecPerClus, unsigned int BPB_ResvdSecCnt,unsigned int BPB_BytsPerSec)
{
	unsigned int val;
	struct directory cluster[16];
	unsigned int FirstSectorofCluster, ThisFATSecNum, ThisFATEntOffset;
	int n, return_val;
	unsigned int num;

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
				cluster[(i/32)+1] = getDirectoryInformation(buffer,
							FirstSectorofCluster*SIZE_OF_SECTOR+i+32);
				if(cluster[(i/32)+1].Attribute == ATTR_DIRECTORY){				
					num = ((int)cluster[(i/32)+1].DIR_FstClusLO[0] << 2) +
						((int)cluster[(i/32)+1].DIR_FstClusLO[1]);
					
					return_val = rmdir_file(buffer,num,FirstDataSector,
								BPB_SecPerClus,BPB_ResvdSecCnt,BPB_BytsPerSec);
					if(return_val == 1){
						for(int j1 = FirstSectorofCluster*SIZE_OF_SECTOR+i,
							j2 = FirstSectorofCluster*SIZE_OF_SECTOR+i+32;
							j1 < FirstSectorofCluster*SIZE_OF_SECTOR+i+32; j1++, j2++)
						{
							buffer[j1] = (unsigned char)0x00;
							buffer[j2] = (unsigned char)0x00;
						}
						return 1;
					}else{
						//not empty
						return -2;
					}
				}else{ //not a directory
					return -1;
				}		
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
	return 0;
	//end of function
}

