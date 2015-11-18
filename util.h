#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef int bool;

static const int SIZE_OF_SECTOR = 512;
static const int FREE_DIR = 0xE5; //229
static const int ZERO_DIR = 0;
static const int EOC = 0x0FFFFFF8;
typedef enum {ATTR_READ_ONLY,
	ATTR_HIDDEN,
        ATTR_SYSTEM,
        ATTR_VOLUME_ID,
	ATTR_DIRECTORY,
	ATTR_ARCHIVE,
	ATTR_LONG_NAME
} ATTR;


struct directory{
	unsigned char DIR_Name[11];
	unsigned char DIR_Attr[1];
	unsigned char DIR_NTRes[1];
	unsigned char DIR_CrtTimeTenth[1];
	unsigned char DIR_CrtTime[2];
	unsigned char DIR_CrtDate[2];
	unsigned char DIR_LstAccDate[2];
	unsigned char DIR_FstClusHI[2];
	unsigned char DIR_WrtTime[2];
	unsigned char DIR_WrtDate[2];
	unsigned char DIR_FstClusLO[2];
	unsigned char DIR_FileSize[4];
	unsigned char Short_Name1[16];
	unsigned char Short_Name2[16];
	ATTR Attribute;	
};

unsigned int getValueFromBootSector(unsigned char*, int, int);
void getValueFromDirectorySector(unsigned char*, unsigned char*, int, int);
struct directory getDirectoryInformation(unsigned char*, int);


#endif
