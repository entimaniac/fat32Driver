#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vector.h"
#include "stack.h"

typedef int bool;

static const int SIZE_OF_SECTOR = 512;
static const int FREE_DIR = 0xE5; //229
static const int ZERO_DIR = 0;
static const int EOC = 0x0FFFFFF8;
static const int EOC2 = 0x0FFFFFFF;
typedef enum {
	ATTR_READ_ONLY,
	ATTR_HIDDEN,
	ATTR_SYSTEM,
	ATTR_VOLUME_ID,
	ATTR_DIRECTORY,
	ATTR_ARCHIVE,
	ATTR_LONG_NAME
} ATTR;
typedef enum{
	GET,SET,ADD,SUB
} MODE;

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
	unsigned char filename[16];
	ATTR Attribute;	
};

/******************************************************************************/
/*                      Function Prototypes */
/******************************************************************************/

unsigned int getValueFromBootSector( unsigned char*, int, int );
void getValueFromDirectorySector( unsigned char*, unsigned char*, int, int );
void getFileName( unsigned char*, unsigned char*, int );
struct directory getDirectoryInformation( unsigned char*, int );
unsigned int currentClusterNumber( MODE, unsigned int );
void presentWorkingDirectory( MODE, char* );
void getCluster( struct directory*, unsigned char*, unsigned int, unsigned int,unsigned int, unsigned int, unsigned int);

void removeTrailingNewline( char* );
int findEmptyCluster(unsigned char*,unsigned int, unsigned int,unsigned int, unsigned int);

unsigned int parseInput( struct directory*, unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int, char*, char* );
int isCommand( struct directory*, unsigned char*, unsigned int, unsigned int,unsigned int, unsigned int, char*, char* );
int isFile( struct directory*, char* );
int isDir( struct directory*, char* );
int checkArgumentCount ( int argNum, int numReq );




#endif
