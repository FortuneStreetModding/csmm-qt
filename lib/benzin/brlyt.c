/******************************************************************************
 *  brlyt.c                                                                   *
 *  Part of Benzin                                                            *
 *  Handles BRLYT banner datas.                                               *
 *  Copyright (C)2009 SquidMan (Alex Marshall)        <SquidMan72@gmail.com>  *
 *  Copyright (C)2009 megazig  (Stephen Simpson)       <megazig@hotmail.com>  *
 *  Copyright (C)2009 Matt_P   (Matthew Parlane)                              *
 *  Copyright (C)2009 comex                                                   *
 *  Copyright (C)2009 booto                                                   *
 *  All Rights Reserved, HACKERCHANNEL.                                       *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mxml.h>
#include <gc/gc.h>

#include "general.h"
#include "types.h"
#include "brlyt.h"
#include "memory.h"
#include "xml.h"
#include "endian.h"

#ifdef DEBUGBRLYT
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGBRLYT

#define MAXIMUM_TAGS_SIZE		(0x1000)

// MAGIC
char pic1_magic[] = "pic1";
char pan1_magic[] = "pan1";
char bnd1_magic[] = "bnd1";
char wnd1_magic[] = "wnd1";
char lyt1_magic[] = "lyt1";
char grp1_magic[] = "grp1";
char txl1_magic[] = "txl1";
char mat1_magic[] = "mat1";
char fnl1_magic[] = "fnl1";
char txt1_magic[] = "txt1";
char gre1_magic[] = "gre1";
char grs1_magic[] = "grs1";
char pae1_magic[] = "pae1";
char pas1_magic[] = "pas1";
char usd1_magic[] = "usd1";

// CONSTANTS
char originX[3][15];
char originY[3][15];
char wraps[4][20];
char tgen_types[11][20];
char tgen_src[21][20];
char mtxsrc[42][20];
char tevcolor[4][20];
char matsrc[2][20];
char scale[10][20];
char compare[8][20];
char aop[5][20];
char blendmode[5][20];
char logicop[16][20];
char blendfactor[8][20];
char chanid[10][20];
char tevswapsel[4][20];

char texcoord[10][20] = 
{
"GX_TEXCOORD0",
"GX_TEXCOORD1",
"GX_TEXCOORD2",
"GX_TEXCOORD3",
"GX_TEXCOORD4",
"GX_TEXCOORD5",
"GX_TEXCOORD6",
"GX_TEXCOORD7",
"GX_MAXCOORD",
"GX_TEXCOORDNULL"
};

char texmap[11][20] =
{
"GX_TEXMAP0",
"GX_TEXMAP1",
"GX_TEXMAP2",
"GX_TEXMAP3",
"GX_TEXMAP4",
"GX_TEXMAP5",
"GX_TEXMAP6",
"GX_TEXMAP7",
"GX_MAX_TEXMAP",
"GX_TEXMAP_NULL",
"GX_TEXMAP_DISABLE"	
};

static size_t BRLYT_fileoffset = 0;

char *materials;
int numberOfMaterials;
int lengthOfMaterials;
char *textures;
int numberOfTextures;
int lengthOfTextures;

void SetupConstants( )
{
	int o;
	for ( o = 0 ; o < 3 ; o++ )
		memset( originX[o] , 0 , 15 );
	strcpy( originX[0] , "Left" );
	strcpy( originX[1] , "Center" );
	strcpy( originX[2] , "Right" );

	for ( o = 0 ; o < 3 ; o++ )
		memset( originY[o] , 0 , 15 );
	strcpy( originY[0] , "Top" );
	strcpy( originY[1] , "Center" );
	strcpy( originY[2] , "Bottom" );

	for ( o = 0 ; o < 4 ; o++ )
		memset( wraps[o] , 0 , 20 );
	strcpy( wraps[0] , "GX_CLAMP" );
	strcpy( wraps[1] , "GX_REPEAT" );
	strcpy( wraps[2] , "GX_MIRROR" );
	strcpy( wraps[3] , "GX_MAXTEXWRAPMODE" );

	for ( o = 0 ; o < 11 ; o++ )
		memset( tgen_types[o] , 0 , 20 );
	strcpy( tgen_types[0]  , "GX_TG_MTX3x4" );
	strcpy( tgen_types[1]  , "GX_TG_MTX2x4" );
	strcpy( tgen_types[2]  , "GX_TG_BUMP0" );
	strcpy( tgen_types[3]  , "GX_TG_BUMP1" );
	strcpy( tgen_types[4]  , "GX_TG_BUMP2" );
	strcpy( tgen_types[5]  , "GX_TG_BUMP3" );
	strcpy( tgen_types[6]  , "GX_TG_BUMP4" );
	strcpy( tgen_types[7]  , "GX_TG_BUMP5" );
	strcpy( tgen_types[8]  , "GX_TG_BUMP6" );
	strcpy( tgen_types[9]  , "GX_TG_BUMP7" );
	strcpy( tgen_types[10] , "GX_TG_SRTG" );

	for ( o = 0 ; o < 21 ; o++ )
		memset( tgen_src[o] , 0 , 20 );
	strcpy( tgen_src[0]  , "GX_TG_POS" );
	strcpy( tgen_src[1]  , "GX_TG_NRM" );
	strcpy( tgen_src[2]  , "GX_TG_BINRM" );
	strcpy( tgen_src[3]  , "GX_TG_TANGEN" );
	strcpy( tgen_src[4]  , "GX_TG_TEX0" );
	strcpy( tgen_src[5]  , "GX_TG_TEX1" );
	strcpy( tgen_src[6]  , "GX_TG_TEX2" );
	strcpy( tgen_src[7]  , "GX_TG_TEX3" );
	strcpy( tgen_src[8]  , "GX_TG_TEX4" );
	strcpy( tgen_src[9]  , "GX_TG_TEX5" );
	strcpy( tgen_src[10] , "GX_TG_TEX6" );
	strcpy( tgen_src[11] , "GX_TG_TEX7" );
	strcpy( tgen_src[12] , "GX_TG_TEXCOORD0" );
	strcpy( tgen_src[13] , "GX_TG_TEXCOORD1" );
	strcpy( tgen_src[14] , "GX_TG_TEXCOORD2" );
	strcpy( tgen_src[15] , "GX_TG_TEXCOORD3" );
	strcpy( tgen_src[16] , "GX_TG_TEXCOORD4" );
	strcpy( tgen_src[17] , "GX_TG_TEXCOORD5" );
	strcpy( tgen_src[18] , "GX_TG_TEXCOORD6" );
	strcpy( tgen_src[19] , "GX_TG_COLOR0" );
	strcpy( tgen_src[20] , "GX_TG_COLOR1" );

	for ( o = 0 ; o < 42 ; o++ )
		memset( mtxsrc[o] , 0 , 20 );
	strcpy( mtxsrc[0]  , "GX_PNMTX0" );
	strcpy( mtxsrc[1]  , "GX_PNMTX1" );
	strcpy( mtxsrc[2]  , "GX_PNMTX2" );
	strcpy( mtxsrc[3]  , "GX_PNMTX3" );
	strcpy( mtxsrc[4]  , "GX_PNMTX4" );
	strcpy( mtxsrc[5]  , "GX_PNMTX5" );
	strcpy( mtxsrc[6]  , "GX_PNMTX6" );
	strcpy( mtxsrc[7]  , "GX_PNMTX7" );
	strcpy( mtxsrc[8]  , "GX_PNMTX8" );
	strcpy( mtxsrc[9]  , "GX_PNMTX9" );
	strcpy( mtxsrc[10] , "GX_TEXMTX0" );
	strcpy( mtxsrc[11] , "GX_TEXMTX1" );
	strcpy( mtxsrc[12] , "GX_TEXMTX2" );
	strcpy( mtxsrc[13] , "GX_TEXMTX3" );
	strcpy( mtxsrc[14] , "GX_TEXMTX4" );
	strcpy( mtxsrc[15] , "GX_TEXMTX5" );
	strcpy( mtxsrc[16] , "GX_TEXMTX6" );
	strcpy( mtxsrc[17] , "GX_TEXMTX7" );
	strcpy( mtxsrc[18] , "GX_TEXMTX8" );
	strcpy( mtxsrc[19] , "GX_TEXMTX9" );
	strcpy( mtxsrc[20] , "GX_IDENTITY" );
	strcpy( mtxsrc[21] , "GX_DTTMTX0" );
	strcpy( mtxsrc[22] , "GX_DTTMTX1" );
	strcpy( mtxsrc[23] , "GX_DTTMTX2" );
	strcpy( mtxsrc[24] , "GX_DTTMTX3" );
	strcpy( mtxsrc[25] , "GX_DTTMTX4" );
	strcpy( mtxsrc[26] , "GX_DTTMTX5" );
	strcpy( mtxsrc[27] , "GX_DTTMTX6" );
	strcpy( mtxsrc[28] , "GX_DTTMTX7" );
	strcpy( mtxsrc[29] , "GX_DTTMTX8" );
	strcpy( mtxsrc[30] , "GX_DTTMTX9" );
	strcpy( mtxsrc[31] , "GX_DTTMTX1" );
	strcpy( mtxsrc[32] , "GX_DTTMTX1" );
	strcpy( mtxsrc[33] , "GX_DTTMTX12" );
	strcpy( mtxsrc[34] , "GX_DTTMTX13" );
	strcpy( mtxsrc[35] , "GX_DTTMTX14" );
	strcpy( mtxsrc[36] , "GX_DTTMTX15" );
	strcpy( mtxsrc[37] , "GX_DTTMTX16" );
	strcpy( mtxsrc[38] , "GX_DTTMTX17" );
	strcpy( mtxsrc[39] , "GX_DTTMTX18" );
	strcpy( mtxsrc[40] , "GX_DTTMTX19" );
	strcpy( mtxsrc[41] , "GX_DTTIDENTITY" );	// actually 125 not 124

	for ( o = 0 ; o < 4 ; o++ )
		memset( tevcolor[o] , 0 , 20 );
	strcpy( tevcolor[0]  , "GX_CH_RED" );
	strcpy( tevcolor[1]  , "GX_CH_GREEN" );
	strcpy( tevcolor[2]  , "GX_CH_BLUE" );
	strcpy( tevcolor[3]  , "GX_CH_ALPHA" );

	for ( o = 0 ; o < 2 ; o++ )
		memset( matsrc[o] , 0 , 20 );
	strcpy( matsrc[0]  , "GX_SRC_REG" );
	strcpy( matsrc[1]  , "GX_SRC_VTX" );

	for ( o = 0 ; o < 10 ; o++ )
		memset( scale[o] , 0 , 20 );
	strcpy( scale[0]  , "GX_ITS_1" );
	strcpy( scale[1]  , "GX_ITS_2" );
	strcpy( scale[2]  , "GX_ITS_4" );
	strcpy( scale[3]  , "GX_ITS_8" );
	strcpy( scale[4]  , "GX_ITS_16" );
	strcpy( scale[5]  , "GX_ITS_32" );
	strcpy( scale[6]  , "GX_ITS_64" );
	strcpy( scale[7]  , "GX_ITS_128" );
	strcpy( scale[8]  , "GX_ITS_256" );
	strcpy( scale[9]  , "GX_MAX_ITSCALE" );

	for ( o = 0 ; o < 8 ; o++ )
		memset( compare[o] , 0 , 20 );
	strcpy( compare[0]  , "GX_NEVER" );
	strcpy( compare[1]  , "GX_LESS" );
	strcpy( compare[2]  , "GX_EQUAL" );
	strcpy( compare[3]  , "GX_LEQUAL" );
	strcpy( compare[4]  , "GX_GREATER" );
	strcpy( compare[5]  , "GX_NEQUAL" );
	strcpy( compare[6]  , "GX_GEQUAL" );
	strcpy( compare[7]  , "GX_ALWAYS" );

	for ( o = 0 ; o < 5 ; o++ )
		memset( aop[o] , 0 , 20 );
	strcpy( aop[0]  , "GX_AOP_AND" );
	strcpy( aop[1]  , "GX_AOP_OR" );
	strcpy( aop[2]  , "GX_AOP_XOR" );
	strcpy( aop[3]  , "GX_AOP_XNOR" );
	strcpy( aop[4]  , "GX_MAX_ALPHAOP" );

	for ( o = 0 ; o < 5 ; o++ )
		memset( blendmode[o] , 0 , 20 );
	strcpy( blendmode[0]  , "GX_BM_NONE" );
	strcpy( blendmode[1]  , "GX_BM_BLEND" );
	strcpy( blendmode[2]  , "GX_BM_LOGIC" );
	strcpy( blendmode[3]  , "GX_BM_SUBTRACT" );
	strcpy( blendmode[4]  , "GX_MAX_BLENDMODE" );

	for ( o = 0 ; o < 8 ; o++ )
		memset( blendfactor[o] , 0 , 20 );
	strcpy( blendfactor[0]  , "GX_BL_ZERO" );
	strcpy( blendfactor[1]  , "GX_BL_ONE" );
	strcpy( blendfactor[2]  , "GX_BL_SRCCLR" );
	strcpy( blendfactor[3]  , "GX_BL_INVSRCCLR" );
	strcpy( blendfactor[4]  , "GX_BL_SRCALPHA" );
	strcpy( blendfactor[5]  , "GX_BL_INVSRCALPHA" );
	strcpy( blendfactor[6]  , "GX_BL_DSTALPHA" );
	strcpy( blendfactor[7]  , "GX_BL_INVDSTALPHA" );

	for ( o = 0 ; o < 16 ; o++ )
		memset( logicop[o] , 0 , 20 );
	strcpy( logicop[0]   , "GX_LO_CLEAR" );
	strcpy( logicop[1]   , "GX_LO_AND" );
	strcpy( logicop[2]   , "GX_LO_REVAND" );
	strcpy( logicop[3]   , "GX_LO_COPY" );
	strcpy( logicop[4]   , "GX_LO_INVAND" );
	strcpy( logicop[5]   , "GX_LO_NOOP" );
	strcpy( logicop[6]   , "GX_LO_XOR" );
	strcpy( logicop[7]   , "GX_LO_OR" );
	strcpy( logicop[8]   , "GX_LO_NOR" );
	strcpy( logicop[9]   , "GX_LO_EQUIV" );
	strcpy( logicop[10]  , "GX_LO_INV" );
	strcpy( logicop[11]  , "GX_LO_REVOR" );
	strcpy( logicop[12]  , "GX_LO_INVCOPY" );
	strcpy( logicop[13]  , "GX_LO_INVOR" );
	strcpy( logicop[14]  , "GX_LO_NAND" );
	strcpy( logicop[15]  , "GX_LO_SET" );

	for ( o = 0 ; o < 10 ; o++ )
		memset( chanid[o] , 0 , 20 );
	strcpy( chanid[0]  , "GX_COLOR0" );
	strcpy( chanid[1]  , "GX_COLOR1" );
	strcpy( chanid[2]  , "GX_ALPHA0" );
	strcpy( chanid[3]  , "GX_ALPHA1" );
	strcpy( chanid[4]  , "GX_COLOR0A0" );
	strcpy( chanid[5]  , "GX_COLOR1A1" );
	strcpy( chanid[6]  , "GX_COLORZERO" );
	strcpy( chanid[7]  , "GX_BUMP" );
	strcpy( chanid[8]  , "GX_BUMPN" );
	strcpy( chanid[9]  , "GX_COLORNULL" );
	
	for ( o = 0 ; o < 4 ; o++ )
                memset( tevswapsel[o] , 0 , 20 );
        strcpy( tevswapsel[0]  , "GX_TEV_SWAP0" );
        strcpy( tevswapsel[1]  , "GX_TEV_SWAP1" );
        strcpy( tevswapsel[2]  , "GX_TEV_SWAP2" );
        strcpy( tevswapsel[3]  , "GX_TEV_SWAP3" );
	
        materials = (char*)GC_MALLOC(12);
        numberOfMaterials = 0;
        lengthOfMaterials = 0;
        textures = (char*)GC_MALLOC(12);
        numberOfTextures = 0;
        lengthOfTextures = 0;
}

static int FourCCsMatch(fourcc cc1, fourcc cc2)
{
	int ret = 1;
	ret &= (cc1[0] == cc2[0]);
	ret &= (cc1[1] == cc2[1]);
	ret &= (cc1[2] == cc2[2]);
	ret &= (cc1[3] == cc2[3]);
	return ret;
}

static void BRLYT_ReadDataFromMemoryX(void* destination, void* input, size_t size)
{
	u8* out = (u8*)destination;
	u8* in = ((u8*)input) + BRLYT_fileoffset;
	memcpy(out, in, size);
}

static void BRLYT_ReadDataFromMemory(void* destination, void* input, size_t size)
{
	BRLYT_ReadDataFromMemoryX(destination, input, size);
	BRLYT_fileoffset += size;
}

float float_swap_bytes(float float1)
{
	u8 *float1c; float1c = (u8*)&float1;

	u8 charTemp = 0x00;
	charTemp = float1c[0]; float1c[0] = float1c[3]; float1c[3] = charTemp;
	charTemp = 0x00;
	charTemp = float1c[1]; float1c[1] = float1c[2]; float1c[2] = charTemp;

	float *newFloat; newFloat = (float*)float1c;
	return *newFloat;
}

u32 bit_extract(u32 num, u32 start, u32 end)
{
	if (end == 100) end = start;
	u32 mask;
	int first;
	int firstMask = 1;
	for (first=0;first<31-start+1;first++)
	{
		firstMask *= 2;
	}
	firstMask -= 1;
	int secondMask = 1;

	for (first=0;first< 31-end;first++)
	{
		secondMask *=2;
	}
	secondMask -= 1;
	mask = firstMask - secondMask;
	int ret = (num & mask) >> (31 - end);
	return ret;
}

char* getMaterial(u16 offset)
{

	if (offset == 0) return materials;
	char *mat = materials + strlen(materials) + 1;

	if (offset > 1)
	{
		int n;
		for (n=1; n<offset;n++)
			mat = mat + strlen(mat) + 1;
	}
	return mat;
}

char* getTexture(u16 offset)
{

	if (offset == 0) return textures;
	char *tex = textures + strlen(textures) + 1;

	if (offset > 1)
	{
		int n;
		for (n=1; n<offset;n++)
			tex = tex + strlen(tex) + 1;
	}
	return tex;
}

u16 findMatOffset(char *mats)
{
	int isEqual = 1;
	char* mat = materials;
	int i;
	for (i = 0; isEqual != 0; )
	{
		isEqual = strcmp(mats, mat);
		if (isEqual != 0) i++;
		mat = mat + strlen(mat) + 1;
		if( i == numberOfMaterials )
		{
			printf("ERROR: material not found: %s\n", mats);
			exit(-1);
		}
	}
	return i;
}

u16 findTexOffset(char *tex)
{
	int isEqual = 1;
	char* texs = textures;
	int i;
	for (i = 0; isEqual != 0; )
	{
		isEqual = strcmp(tex, texs);
		if (isEqual != 0) i++;
		texs = texs + strlen(texs) + 1;
		if( i == numberOfTextures )
		{
			printf("ERROR: texture not found: %s\n", tex);
			exit(-1);
		}
	}
	return i;
}

int BRLYT_ReadEntries(u8* brlyt_file, size_t file_size, brlyt_header header, brlyt_entry* entries)
{
	return 0;
}

void BRLYT_CheckHeaderSanity(brlyt_header header, size_t filesize)
{
	if((header.magic[0] != 'R') || (header.magic[1] != 'L') || (header.magic[2] != 'Y') || (header.magic[3] != 'T')) {
		printf("BRLYT magic doesn't match! %.4s\n", header.magic);
		printf("magic: %.4s\n", header.magic);
		exit(1);
	}
	if(header.endian != short_swap_bytes(0xFEFF)){
		printf("BRLYT endian is not set for big\n");
		printf("endian: %04x\n", short_swap_bytes(header.endian) );
		exit(1);
	}
	if((header.version != short_swap_bytes(0x0008) )&&(header.version != short_swap_bytes(0x000a))){
		printf("BRLYT version not right\n");
		printf("version: %04x\n", short_swap_bytes(header.version) );
		exit(1);
	}
	if(filesize != be32(header.filesize)) {
		printf("BRLYT filesize doesn't match!\n");
		exit(1);
	}
}

void PrintBRLYTEntry_lyt1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxml_node_t *a;
	mxml_node_t *size;
	mxml_node_t *width;
	mxml_node_t *height;
	brlyt_lytheader_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_lytheader_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	a = mxmlNewElement(tag, "drawnFromMiddle"); mxmlNewTextf(a, 0, "%02x", data.drawnFromMiddle);
	size = mxmlNewElement(tag, "size");
	width = mxmlNewElement(size, "width"); mxmlNewTextf(width, 0, "%f", float_swap_bytes(data.width));
	height = mxmlNewElement(size, "height"); mxmlNewTextf(height, 0,"%f", float_swap_bytes(data.height));
}

void PrintBRLYTEntry_grp1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxml_node_t *subs, *sub;
	brlyt_group_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_group_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	mxmlElementSetAttrf(tag, "name", "%s", data.name);

	if (short_swap_bytes(data.numsubs) > 0) subs = mxmlNewElement(tag, "subs");

	int i;
	for (i=0;i<short_swap_bytes(data.numsubs);i++)
	{
		char subb[16];
		memset(subb, 0x00, 16);
		BRLYT_ReadDataFromMemory(subb, brlyt_file, sizeof(subb));
		sub = mxmlNewElement(subs, "sub"); mxmlNewTextf(sub, 0, "%.16s", subb);
	}
}

void PrintBRLYTEntry_txl1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxml_node_t *entries;
	mxml_node_t *naame;
	brlyt_numoffs_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	entries = mxmlNewElement(tag, "entries");
	int pos = 4;
	pos += data.offs;
	int bpos = pos;
	int n;
	for (n=0;n<short_swap_bytes(data.num);n++)
	{
		brlyt_offsunk_chunk data2;
		BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_offsunk_chunk));
		int tempLocation = BRLYT_fileoffset;
		BRLYT_fileoffset = entry.data_location + bpos + be32(data2.offset);
		int toRead = (be32(entry.length) + entry.data_location - 8) - BRLYT_fileoffset;
		char nameRead[toRead];
		BRLYT_ReadDataFromMemory(nameRead, brlyt_file, sizeof(nameRead));

		char tpl = 0;
		char *ending = memchr(nameRead, tpl, toRead);
		int end = ending - nameRead;
		char name[end+1];
		memcpy(name, nameRead, sizeof(name));
		naame = mxmlNewElement(entries, "name"); mxmlNewTextf(naame, 0, "%s", name);
		BRLYT_fileoffset = tempLocation;
		int newSize = lengthOfTextures+sizeof(name);
        textures = GC_REALLOC(textures, newSize);
		numberOfTextures += 1;
		memcpy(textures+lengthOfTextures, name, sizeof(name));
		lengthOfTextures = newSize;
	}
}

void PrintBRLYTEntry_fnl1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxml_node_t *entries;
	mxml_node_t *naame;
	brlyt_numoffs_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	entries = mxmlNewElement(tag, "entries");
	int pos = 4;
	pos += data.offs;
	int bpos = pos;
	int n;
	for (n=0;n<short_swap_bytes(data.num);n++)
	{
		brlyt_offsunk_chunk data2;
		BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_offsunk_chunk));
		int tempLocation = BRLYT_fileoffset;
		BRLYT_fileoffset = entry.data_location + bpos + be32(data2.offset);
		int toRead = (be32(entry.length) + entry.data_location - 8) - BRLYT_fileoffset;
		char nameRead[toRead];
		BRLYT_ReadDataFromMemory(nameRead, brlyt_file, sizeof(nameRead));

		char tpl = 0;
		char *ending = memchr(nameRead, tpl, toRead);
		int end = ending - nameRead;
		char name[end+1];
		memcpy(name, nameRead, sizeof(name));
		naame = mxmlNewElement(entries, "name"); mxmlNewTextf(naame, 0, "%s", name);
		BRLYT_fileoffset = tempLocation;
	}
}

void PrintPane( brlyt_entry entry , u8* brlyt_file , mxml_node_t * tag )
{
	mxml_node_t *flags, *origin, *alpha, *padding;
	mxml_node_t *translate, *x, *y, *z;
	mxml_node_t *rotate, *scale;
	mxml_node_t *size, *width, *height;
	brlyt_pane_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_pane_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	mxmlElementSetAttrf(tag, "name", "%s", data.name);
	mxmlElementSetAttrf(tag, "userdata", "%s", data.userdata);
	flags = mxmlNewElement(tag, "visible"); mxmlNewTextf(flags, 0, "%02x", data.flag1 & 0x1);
	flags = mxmlNewElement(tag, "WidescreenAffected"); mxmlNewTextf(flags, 0, "%02x", (data.flag1 & 0x2)>>1);
	flags = mxmlNewElement(tag, "flag"); mxmlNewTextf(flags, 0, "%02x", (data.flag1 & 0x4)>>2);
	origin  = mxmlNewElement(tag, "origin");
		mxmlElementSetAttrf(origin, "x", "%s", originX[data.origin%3]);
		mxmlElementSetAttrf(origin, "y", "%s", originY[data.origin/3]);
	alpha   = mxmlNewElement(tag, "alpha");   mxmlNewTextf(alpha, 0, "%02x", data.alpha);
	padding = mxmlNewElement(tag, "padding"); mxmlNewTextf(padding, 0, "%02x", data.pad);
	translate = mxmlNewElement(tag, "translate");
	x = mxmlNewElement(translate, "x"); mxmlNewTextf(x, 0, "%.25f", float_swap_bytes(data.XTrans));
	y = mxmlNewElement(translate, "y"); mxmlNewTextf(y, 0, "%.25f", float_swap_bytes(data.YTrans));
	z = mxmlNewElement(translate, "z"); mxmlNewTextf(z, 0, "%.25f", float_swap_bytes(data.ZTrans));
	rotate = mxmlNewElement(tag, "rotate");
	x = mxmlNewElement(rotate, "x"); mxmlNewTextf(x, 0, "%.20f", float_swap_bytes(data.XRotate));
	y = mxmlNewElement(rotate, "y"); mxmlNewTextf(y, 0, "%.20f", float_swap_bytes(data.YRotate));
	z = mxmlNewElement(rotate, "z"); mxmlNewTextf(z, 0, "%.20f", float_swap_bytes(data.ZRotate));
	scale = mxmlNewElement(tag, "scale");
	x = mxmlNewElement(scale, "x"); mxmlNewTextf(x, 0, "%.10f", float_swap_bytes(data.XScale));
	y = mxmlNewElement(scale, "y"); mxmlNewTextf(y, 0, "%.10f", float_swap_bytes(data.YScale));
	size = mxmlNewElement(tag, "size");
	width = mxmlNewElement(size, "width"); mxmlNewTextf(width, 0, "%f", float_swap_bytes(data.width));
	height = mxmlNewElement(size, "height"); mxmlNewTextf(height, 0, "%f", float_swap_bytes(data.height));
}

void PrintBRLYTEntry_pan1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	PrintPane( entry , brlyt_file , tag );
}

void PrintBRLYTEntry_wnd1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	PrintPane( entry , brlyt_file , tag );

	mxml_node_t *wndd, *unkk, *count, *offset1, *offset2, *padding;
	int i, j;
	brlyt_wnd wndy;
	BRLYT_ReadDataFromMemory(&wndy, brlyt_file, sizeof(brlyt_wnd));
	wndd = mxmlNewElement(tag, "wnd");
	for(i=0;i<4;i++)
	{
		unkk = mxmlNewElement(wndd, "coordinate"); mxmlNewTextf(unkk, 0, "%f", float_swap_bytes(wndy.coords[i]));
	}
	count = mxmlNewElement(wndd, "FrameCount"); mxmlNewTextf(count, 0, "%02x", wndy.frame_count);
	offset1 = mxmlNewElement(wndd, "offset1"); mxmlNewTextf(offset1, 0, "%08x", be32(wndy.offset1));
	offset2 = mxmlNewElement(wndd, "offset2"); mxmlNewTextf(offset2, 0, "%08x", be32(wndy.offset2));

	mxml_node_t *wnddd, *colors, *material;
	brlyt_wnd1 wndy1;
	BRLYT_ReadDataFromMemory(&wndy1, brlyt_file, sizeof(brlyt_wnd1));
	wnddd = mxmlNewElement(tag, "wnd1");
	for(i=0;i<4;i++)
	{
		colors = mxmlNewElement(wnddd, "color"); mxmlNewTextf(colors, 0, "%08x", int_swap_bytes(wndy1.colors[i]));
	}

	material = mxmlNewElement(wnddd, "material"); mxmlElementSetAttrf(material, "name", "%s", getMaterial(short_swap_bytes(wndy1.material)));
	count = mxmlNewElement(wnddd, "CoordinateCount"); mxmlNewTextf(count, 0, "%02x", short_swap_bytes(wndy1.coordinate_count));
	padding = mxmlNewElement(wnddd, "padding"); mxmlNewTextf(padding, 0, "%02x", short_swap_bytes(wndy1.padding));

	mxml_node_t *wnddddd, *texcoord;
	for ( i = 0 ; i < wndy1.coordinate_count ; i++ ){
		brlyt_wnd3 wndy3;
		BRLYT_ReadDataFromMemory(&wndy3, brlyt_file, sizeof(brlyt_wnd3));
		wnddddd = mxmlNewElement(tag, "Coords");
		for ( j = 0 ; j < 8 ; j++ ) {
			texcoord = mxmlNewElement(wnddddd, "texcoord"); mxmlNewTextf(texcoord, 0, "%f", float_swap_bytes(wndy3.texcoords[j]));
		}

	}

	mxml_node_t *wndddddd, *offsett;
	brlyt_wnd4 wndy4;
	for(i=0;i<wndy.frame_count;i++)
	{
		BRLYT_ReadDataFromMemory(&wndy4, brlyt_file, sizeof(brlyt_wnd4));
		wndddddd = mxmlNewElement(tag, "wnd4");
		offsett = mxmlNewElement(wndddddd, "offset"); mxmlNewTextf(offsett, 0, "%08x", be32(wndy4.offset));
	}

	mxml_node_t *wndmat, *matrl;
	brlyt_wnd4_mat wndy4mat;
	for(i=0;i<wndy.frame_count;i++)
	{
		BRLYT_ReadDataFromMemory(&wndy4mat, brlyt_file, sizeof(brlyt_wnd4_mat));
		wndmat = mxmlNewElement(tag, "wnd4mat");
		matrl = mxmlNewElement(wndmat, "material"); mxmlElementSetAttrf(matrl, "name", "%s", getMaterial(short_swap_bytes(wndy4mat.material)));
		matrl = mxmlNewElement(wndmat, "index"); mxmlNewTextf(matrl, 0, "%02x", wndy4mat.index);
		matrl = mxmlNewElement(wndmat, "padding"); mxmlNewTextf(matrl, 0, "%02x", wndy4mat.padding);
	}
}

void PrintBRLYTEntry_usd1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	// DO SOMETHING COOL
	mxml_node_t *usd, *val, *isworking, *stringg;
	brlyt_usdstart_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	int n = 0;
	for (n=0;n<short_swap_bytes(data.string_count);n++)
	{
		u32 tempOffset = BRLYT_fileoffset;
		brlyt_usdmain_chunk data2;
		BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_usdmain_chunk));
		usd = mxmlNewElement(tag, "usdentry");
		val = mxmlNewElement(usd, "unk5"); mxmlNewTextf(val, 0, "%04x", short_swap_bytes(data2.unk5));
		val = mxmlNewElement(usd, "unk6"); mxmlNewTextf(val, 0, "%02x", data2.unk6);
		val = mxmlNewElement(usd, "unk7"); mxmlNewTextf(val, 0, "%02x", data2.unk7);
		BRLYT_fileoffset = tempOffset + be32(data2.is_working_offset);
		u32 working;
		BRLYT_ReadDataFromMemoryX(&working, brlyt_file, sizeof(u32));
		isworking = mxmlNewElement(usd, "isWorking"); mxmlNewTextf(isworking, 0, "%08x", be32(working));
		BRLYT_fileoffset = tempOffset + be32(data2.string_offset);
		char string_name[8];
		BRLYT_ReadDataFromMemoryX(string_name, brlyt_file, sizeof(char) * 8);
		stringg = mxmlNewElement(usd, "string"); mxmlNewTextf(stringg, 0, "%s", string_name);
		BRLYT_fileoffset = tempOffset + 0xC;

	}
}

void PrintBRLYTEntry_bnd1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	PrintPane( entry , brlyt_file , tag );
}

void PrintBRLYTEntry_pic1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	PrintPane( entry , brlyt_file , tag );

	brlyt_pic_chunk data2;
	BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_pic_chunk));
	mxml_node_t *material, *color, *vtx, *coordinates, *set, *coord;
	material = mxmlNewElement(tag, "material"); mxmlElementSetAttrf(material, "name", "%s", getMaterial(short_swap_bytes(data2.mat_num)));
	color = mxmlNewElement(tag, "colors");

	vtx = mxmlNewElement(color, "vtxColorTL");
	mxmlElementSetAttrf( vtx , "r" , "%02x" , ( be32(data2.vtx_colors[0]) >> 24 ) & 0xff );
	mxmlElementSetAttrf( vtx , "g" , "%02x" , ( be32(data2.vtx_colors[0]) >> 16 ) & 0xff );
	mxmlElementSetAttrf( vtx , "b" , "%02x" , ( be32(data2.vtx_colors[0]) >>  8 ) & 0xff );
	mxmlElementSetAttrf( vtx , "a" , "%02x" , ( be32(data2.vtx_colors[0]) >>  0 ) & 0xff );

	vtx = mxmlNewElement(color, "vtxColorTR");
	mxmlElementSetAttrf( vtx , "r" , "%02x" , ( be32(data2.vtx_colors[1]) >> 24 ) & 0xff );
	mxmlElementSetAttrf( vtx , "g" , "%02x" , ( be32(data2.vtx_colors[1]) >> 16 ) & 0xff );
	mxmlElementSetAttrf( vtx , "b" , "%02x" , ( be32(data2.vtx_colors[1]) >>  8 ) & 0xff );
	mxmlElementSetAttrf( vtx , "a" , "%02x" , ( be32(data2.vtx_colors[1]) >>  0 ) & 0xff );

	vtx = mxmlNewElement(color, "vtxColorBL");
	mxmlElementSetAttrf( vtx , "r" , "%02x" , ( be32(data2.vtx_colors[2]) >> 24 ) & 0xff );
	mxmlElementSetAttrf( vtx , "g" , "%02x" , ( be32(data2.vtx_colors[2]) >> 16 ) & 0xff );
	mxmlElementSetAttrf( vtx , "b" , "%02x" , ( be32(data2.vtx_colors[2]) >>  8 ) & 0xff );
	mxmlElementSetAttrf( vtx , "a" , "%02x" , ( be32(data2.vtx_colors[2]) >>  0 ) & 0xff );

	vtx = mxmlNewElement(color, "vtxColorBR");
	mxmlElementSetAttrf( vtx , "r" , "%02x" , ( be32(data2.vtx_colors[3]) >> 24 ) & 0xff );
	mxmlElementSetAttrf( vtx , "g" , "%02x" , ( be32(data2.vtx_colors[3]) >> 16 ) & 0xff );
	mxmlElementSetAttrf( vtx , "b" , "%02x" , ( be32(data2.vtx_colors[3]) >>  8 ) & 0xff );
	mxmlElementSetAttrf( vtx , "a" , "%02x" , ( be32(data2.vtx_colors[3]) >>  0 ) & 0xff );

	coordinates = mxmlNewElement(tag, "coordinates");
	int n;
	for (n=0;n<data2.num_texcoords;n++)
	{
		float texcoords[8];
		BRLYT_ReadDataFromMemory(texcoords, brlyt_file, sizeof(texcoords));

		set = mxmlNewElement(coordinates, "set");

		coord = mxmlNewElement(set, "coordTL");
		mxmlElementSetAttrf( coord , "s" ,  "%f" , float_swap_bytes(texcoords[0]) );
		mxmlElementSetAttrf( coord , "t" ,  "%f" , float_swap_bytes(texcoords[1]) );

		coord = mxmlNewElement(set, "coordTR");
		mxmlElementSetAttrf( coord , "s" ,  "%f" , float_swap_bytes(texcoords[2]) );
		mxmlElementSetAttrf( coord , "t" ,  "%f" , float_swap_bytes(texcoords[3]) );

		coord = mxmlNewElement(set, "coordBL");
		mxmlElementSetAttrf( coord , "s" ,  "%f" , float_swap_bytes(texcoords[4]) );
		mxmlElementSetAttrf( coord , "t" ,  "%f" , float_swap_bytes(texcoords[5]) );

		coord = mxmlNewElement(set, "coordBR");
		mxmlElementSetAttrf( coord , "s" ,  "%f" , float_swap_bytes(texcoords[6]) );
		mxmlElementSetAttrf( coord , "t" ,  "%f" , float_swap_bytes(texcoords[7]) );

	}
}

void PrintBRLYTEntry_txt1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	PrintPane( entry , brlyt_file , tag );

	mxml_node_t *length, *material, *font, *xsize, *ysize, *charsize, *linesize, *alignment, *unkk, *color1, *color2, *text;
	brlyt_text_chunk data2;
	BRLYT_ReadDataFromMemory(&data2, brlyt_file, sizeof(brlyt_text_chunk));
	u8 texty[short_swap_bytes(data2.len2)];
	memcpy(texty, &brlyt_file[BRLYT_fileoffset], short_swap_bytes(data2.len2));
	length = mxmlNewElement(tag, "length"); mxmlNewTextf(length, 0, "%04x-%04x", short_swap_bytes(data2.len2), short_swap_bytes(data2.len2));
	material = mxmlNewElement(tag, "material"); mxmlElementSetAttrf(material, "name", "%s", getMaterial(short_swap_bytes(data2.mat_num)));
	font = mxmlNewElement(tag, "font"); mxmlElementSetAttrf(font, "index", "%d", short_swap_bytes(data2.font_idx));
	xsize = mxmlNewElement(font, "xsize"); mxmlNewTextf(xsize, 0, "%f", float_swap_bytes(data2.font_size_x));
	ysize = mxmlNewElement(font, "ysize"); mxmlNewTextf(ysize, 0, "%f", float_swap_bytes(data2.font_size_y));
	charsize = mxmlNewElement(font, "charsize"); mxmlNewTextf(charsize, 0, "%f", float_swap_bytes(data2.char_space));
	linesize = mxmlNewElement(font, "linesize"); mxmlNewTextf(linesize, 0, "%f", float_swap_bytes(data2.line_space));
	alignment = mxmlNewElement(font, "alignment");
		mxmlElementSetAttrf(alignment, "x", "%s", originX[data2.alignment%3]);
		mxmlElementSetAttrf(alignment, "y", "%s", originY[data2.alignment/3]);
	unkk = mxmlNewElement(font, "whatAmI"); mxmlNewTextf(unkk, 0, "%02x", data2.unk_char);
	color1 = mxmlNewElement(tag, "topcolor");// mxmlNewTextf(color1, 0, "%08x", be32(data2.color1) );
		mxmlElementSetAttrf(color1, "r", "%02x", ( be32(data2.color1) >> 24 ) & 0xff );
		mxmlElementSetAttrf(color1, "g", "%02x", ( be32(data2.color1) >> 16 ) & 0xff );
		mxmlElementSetAttrf(color1, "b", "%02x", ( be32(data2.color1) >>  8 ) & 0xff );
		mxmlElementSetAttrf(color1, "a", "%02x", ( be32(data2.color1) >>  0 ) & 0xff );
	color2 = mxmlNewElement(tag, "bottomcolor");// mxmlNewTextf(color2, 0, "%08x", be32(data2.color2) );
		mxmlElementSetAttrf(color2, "r", "%02x", ( be32(data2.color2) >> 24 ) & 0xff );
		mxmlElementSetAttrf(color2, "g", "%02x", ( be32(data2.color2) >> 16 ) & 0xff );
		mxmlElementSetAttrf(color2, "b", "%02x", ( be32(data2.color2) >>  8 ) & 0xff );
		mxmlElementSetAttrf(color2, "a", "%02x", ( be32(data2.color2) >>  0 ) & 0xff );
//	int q; for(q=0;q<short_swap_bytes(data2.len2);q++) printf("%02x", texty[q]);	// S T U P I D   U T F 1 6	T E X T
	u8 textbuffer[4096];
	int q; for(q=0;q<short_swap_bytes(data2.len2);q++) sprintf((char*)&textbuffer[q*2], "%02x", texty[q]);
	text = mxmlNewElement(tag, "text"); mxmlNewTextf(text, 0, "%s", textbuffer);
}

void PrintBRLYTEntry_mat1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxml_node_t *entries, *colors, *forecolor, *backcolor, *unk2, *tev_k;
	// mxml_node_t * flags;
	brlyt_numoffs_chunk data;
	BRLYT_fileoffset = entry.data_location;
	BRLYT_ReadDataFromMemory(&data, brlyt_file, sizeof(brlyt_numoffs_chunk));
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
	int n = 0;
	for (n=0;n<short_swap_bytes(data.num);n++)
	{
		int offset;
		BRLYT_ReadDataFromMemory(&offset, brlyt_file, sizeof(offset));
		int tempDataLocation = BRLYT_fileoffset;
		BRLYT_fileoffset = entry.data_location + be32(offset) - 8;
		brlyt_material_chunk data3;
		BRLYT_ReadDataFromMemory(&data3, brlyt_file, sizeof(brlyt_material_chunk));

		u32 flaggs = be32(data3.flags);
		entries = mxmlNewElement(tag, "entries"); mxmlElementSetAttrf(entries, "name",  "%s", data3.name);
		colors = mxmlNewElement(entries, "colors");

		forecolor = mxmlNewElement(colors, "forecolor");
		mxmlElementSetAttrf(forecolor, "r", "%d", short_swap_bytes(data3.forecolor[0]));
		mxmlElementSetAttrf(forecolor, "g", "%d", short_swap_bytes(data3.forecolor[1]));
		mxmlElementSetAttrf(forecolor, "b", "%d", short_swap_bytes(data3.forecolor[2]));
		mxmlElementSetAttrf(forecolor, "a", "%d", short_swap_bytes(data3.forecolor[3]));

		backcolor = mxmlNewElement(colors, "backcolor");
		mxmlElementSetAttrf(backcolor, "r", "%d", short_swap_bytes(data3.backcolor[0]));
		mxmlElementSetAttrf(backcolor, "g", "%d", short_swap_bytes(data3.backcolor[1]));
		mxmlElementSetAttrf(backcolor, "b", "%d", short_swap_bytes(data3.backcolor[2]));
		mxmlElementSetAttrf(backcolor, "a", "%d", short_swap_bytes(data3.backcolor[3]));

		unk2 = mxmlNewElement(colors, "colorReg3");
		mxmlElementSetAttrf(unk2, "r", "%d", short_swap_bytes(data3.colorReg3[0]));
		mxmlElementSetAttrf(unk2, "g", "%d", short_swap_bytes(data3.colorReg3[1]));
		mxmlElementSetAttrf(unk2, "b", "%d", short_swap_bytes(data3.colorReg3[2]));
		mxmlElementSetAttrf(unk2, "a", "%d", short_swap_bytes(data3.colorReg3[3]));

		tev_k = mxmlNewElement(colors, "tev_k1");
		mxmlElementSetAttrf(tev_k, "r", "%02x", ( be32(data3.tev_kcolor[0]) >> 24 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "g", "%02x", ( be32(data3.tev_kcolor[0]) >> 16 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "b", "%02x", ( be32(data3.tev_kcolor[0]) >>  8 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "a", "%02x", ( be32(data3.tev_kcolor[0]) >>  0 ) & 0xff );

		tev_k = mxmlNewElement(colors, "tev_k2");
		mxmlElementSetAttrf(tev_k, "r", "%02x", ( be32(data3.tev_kcolor[1]) >> 24 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "g", "%02x", ( be32(data3.tev_kcolor[1]) >> 16 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "b", "%02x", ( be32(data3.tev_kcolor[1]) >>  8 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "a", "%02x", ( be32(data3.tev_kcolor[1]) >>  0 ) & 0xff );

		tev_k = mxmlNewElement(colors, "tev_k3");
		mxmlElementSetAttrf(tev_k, "r", "%02x", ( be32(data3.tev_kcolor[2]) >> 24 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "g", "%02x", ( be32(data3.tev_kcolor[2]) >> 16 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "b", "%02x", ( be32(data3.tev_kcolor[2]) >>  8 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "a", "%02x", ( be32(data3.tev_kcolor[2]) >>  0 ) & 0xff );

		tev_k = mxmlNewElement(colors, "tev_k4");
		mxmlElementSetAttrf(tev_k, "r", "%02x", ( be32(data3.tev_kcolor[3]) >> 24 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "g", "%02x", ( be32(data3.tev_kcolor[3]) >> 16 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "b", "%02x", ( be32(data3.tev_kcolor[3]) >>  8 ) & 0xff );
		mxmlElementSetAttrf(tev_k, "a", "%02x", ( be32(data3.tev_kcolor[3]) >>  0 ) & 0xff );

		//flags = mxmlNewElement(entries, "flags"); mxmlNewTextf(flags, 0, "%08x", be32(data3.flags));

		int newSize = lengthOfMaterials+strlen(data3.name)+1;
        materials = GC_REALLOC(materials, newSize);
		numberOfMaterials += 1;
		memcpy(materials+lengthOfMaterials, data3.name, 1+strlen(data3.name));
		lengthOfMaterials = newSize;

		mxml_node_t *texture, *wrap_t, *wrap_s;
		int n;
		for (n=0;n<bit_extract(flaggs, 28,31);n++)
		{
			brlyt_texref_chunk data4;
			BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_texref_chunk));
			int tplOffset = short_swap_bytes(data4.tex_offs);
			texture = mxmlNewElement(entries, "texture"); mxmlElementSetAttrf(texture, "name", "%s", getTexture(tplOffset));

			wrap_s = mxmlNewElement(texture, "wrap_s");
			mxmlNewTextf(wrap_s, 0, "%s", wraps[data4.wrap_s]);
			wrap_t = mxmlNewElement(texture, "wrap_t");
			mxmlNewTextf(wrap_t, 0, "%s", wraps[data4.wrap_t]);
		}

		mxml_node_t *tex_coords, *dataa;
//		# 0x14 * flags[24-27], followed by
				for (n=0;n<bit_extract(flaggs, 24,27);n++)
				{
						brlyt_tex_srt data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_tex_srt));
						tex_coords = mxmlNewElement(entries, "TextureSRT");
						dataa = mxmlNewElement(tex_coords, "XTrans"); mxmlNewTextf(dataa, 0, "%.10f", float_swap_bytes(data4.XTrans));
						dataa = mxmlNewElement(tex_coords, "YTrans"); mxmlNewTextf(dataa, 0, "%.10f", float_swap_bytes(data4.YTrans));
						dataa = mxmlNewElement(tex_coords, "Rotate"); mxmlNewTextf(dataa, 0, "%.10f", float_swap_bytes(data4.Rotate));
						dataa = mxmlNewElement(tex_coords, "XScale"); mxmlNewTextf(dataa, 0, "%.10f", float_swap_bytes(data4.XScale));
						dataa = mxmlNewElement(tex_coords, "YScale"); mxmlNewTextf(dataa, 0, "%.10f", float_swap_bytes(data4.YScale));
				}

				mxml_node_t *ua3;
		//# 4*flags[20-23], followed by
				for (n=0;n<bit_extract(flaggs, 20,23);n++)
				{
						brlyt_tex_coordgen data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_tex_coordgen));
						ua3 = mxmlNewElement(entries, "CoordGen");

						dataa = mxmlNewElement(ua3, "tgen_type");
						mxmlNewTextf(dataa, 0, "%s", tgen_types[data4.tgen_type]);
						dataa = mxmlNewElement(ua3, "tgen_src");
						mxmlNewTextf(dataa, 0, "%s", tgen_src[data4.tgen_src]);
						dataa = mxmlNewElement(ua3, "MatrixSource");
						mxmlNewTextf(dataa, 0, "%s", mtxsrc[data4.mtxsrc/3]);
						dataa = mxmlNewElement(ua3, "padding"); mxmlNewTextf(dataa, 0, "%02x", data4.padding);
				}

				mxml_node_t *ua4;
		//# 4 * flags[6]
				for (n=0;n<bit_extract(flaggs, 6,100);n++)
				{
						brlyt_tex_chanctrl data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_tex_chanctrl));
						ua4 = mxmlNewElement(entries, "ChanControl");

						dataa = mxmlNewElement(ua4, "ColorMatSource");
						mxmlNewTextf(dataa, 0, "%s", matsrc[data4.color_matsrc]);
						dataa = mxmlNewElement(ua4, "AlphaMatSource");
						mxmlNewTextf(dataa, 0, "%s", matsrc[data4.alpha_matsrc]);
						dataa = mxmlNewElement(ua4, "Padding1");
						mxmlNewTextf(dataa, 0, "%02x", data4.padding1);
						dataa = mxmlNewElement(ua4, "Padding2");
						mxmlNewTextf(dataa, 0, "%02x", data4.padding2);

				}

				mxml_node_t *ua5;
		//# 4 * flags[4]
				for (n=0;n<bit_extract(flaggs, 4,100);n++)
				{
						brlyt_tex_matcol data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_tex_matcol));
						ua5 = mxmlNewElement(entries, "MaterialColor");

						dataa = mxmlNewElement(ua5, "red"); mxmlNewTextf(dataa, 0, "%02x", data4.red);
						dataa = mxmlNewElement(ua5, "green"); mxmlNewTextf(dataa, 0, "%02x", data4.green);
						dataa = mxmlNewElement(ua5, "blue"); mxmlNewTextf(dataa, 0, "%02x", data4.blue);
						dataa = mxmlNewElement(ua5, "alpha"); mxmlNewTextf(dataa, 0, "%02x", data4.alpha);
				}

				mxml_node_t *ua6;
		//# 4 * flags[19]
				for (n=0;n<bit_extract(flaggs, 19,100);n++)
				{
						brlyt_tev_swapmodetable data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_tev_swapmodetable));

						ua6 = mxmlNewElement(entries, "TevSwapModeTable");

						dataa = mxmlNewElement(ua6, "OneRed");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.one & 0x03) >> 0] );
						dataa = mxmlNewElement(ua6, "OneGreen");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.one & 0x0C) >> 2] );
						dataa = mxmlNewElement(ua6, "OneBlue");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.one & 0x30) >> 4] );
						dataa = mxmlNewElement(ua6, "OneAlpha");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.one & 0xC0) >> 6] );

						dataa = mxmlNewElement(ua6, "TwoRed");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.two & 0x03) >> 0] );
						dataa = mxmlNewElement(ua6, "TwoGreen");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.two & 0x0C) >> 2] );
						dataa = mxmlNewElement(ua6, "TwoBlue");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.two & 0x30) >> 4] );
						dataa = mxmlNewElement(ua6, "TwoAlpha");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.two & 0xC0) >> 6] );

						dataa = mxmlNewElement(ua6, "ThreeRed");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.three & 0x03) >> 0] );
						dataa = mxmlNewElement(ua6, "ThreeGreen");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.three & 0x0C) >> 2] );
						dataa = mxmlNewElement(ua6, "ThreeBlue");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.three & 0x30) >> 4] );
						dataa = mxmlNewElement(ua6, "ThreeAlpha");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.three & 0xC0) >> 6] );

						dataa = mxmlNewElement(ua6, "FourRed");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.four & 0x03) >> 0] );
						dataa = mxmlNewElement(ua6, "FourGreen");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.four & 0x0C) >> 2] );
						dataa = mxmlNewElement(ua6, "FourBlue");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.four & 0x30) >> 4] );
						dataa = mxmlNewElement(ua6, "FourAlpha");
						mxmlNewTextf(dataa, 0, "%s", tevcolor[(data4.four & 0xC0) >> 6] );

				}

				mxml_node_t *ua7, *a, *b, *c, *d, *e;
				for (n=0;n<bit_extract(flaggs, 17,18);n++)
				{
						brlyt_indtex_srt data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_indtex_srt));
						ua7 = mxmlNewElement(entries, "IndTextureSRT");
						a = mxmlNewElement(ua7, "XTrans"); mxmlNewTextf(a, 0, "%f", float_swap_bytes(data4.XTrans));
						b = mxmlNewElement(ua7, "YTrans"); mxmlNewTextf(b, 0, "%f", float_swap_bytes(data4.YTrans));
						c = mxmlNewElement(ua7, "Rotate"); mxmlNewTextf(c, 0, "%f", float_swap_bytes(data4.Rotate));
						d = mxmlNewElement(ua7, "XScale"); mxmlNewTextf(d, 0, "%f", float_swap_bytes(data4.XScale));
						e = mxmlNewElement(ua7, "YScale"); mxmlNewTextf(e, 0, "%f", float_swap_bytes(data4.YScale));
				}

				mxml_node_t *ua8;
		//# 4 * flags[14-16]
				for (n=0;n<bit_extract(flaggs, 14,16);n++)
				{
						brlyt_indtex_order data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_indtex_order));
						ua8 = mxmlNewElement(entries, "IndTextureOrder");

						dataa = mxmlNewElement(ua8, "TextureCoordinate");
						mxmlNewTextf(dataa, 0, "%s", texcoord[(data4.tex_coord == 0xff) ? 9 : data4.tex_coord]);
						dataa = mxmlNewElement(ua8, "TextureMap");
						mxmlNewTextf(dataa, 0, "%s", texmap[(data4.tex_map > 8) ? (data4.tex_map == 0xff) ? 9 : 10 : data4.tex_map ]);
						dataa = mxmlNewElement(ua8, "ScaleS");
						mxmlNewTextf(dataa, 0, "%s", scale[data4.scale_s]);
						dataa = mxmlNewElement(ua8, "ScaleT");
						mxmlNewTextf(dataa, 0, "%s", scale[data4.scale_t]);
				}

				mxml_node_t *ua9;
		//# 0x10 * flags[9-13]
				for (n=0;n<bit_extract(flaggs, 9,13);n++)
				{
						TevStages data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(TevStages));
						ua9 = mxmlNewElement(entries, "TevStage");

						dataa = mxmlNewElement(ua9, "TextureCoordinate");
						mxmlNewTextf(dataa, 0, "%s", texcoord[(data4.texcoord == 0xff) ? 9 : data4.texcoord]);

						dataa = mxmlNewElement(ua9, "TextureMap");
						int texmapind = ((data4.texmaptop << 8) | data4.texmapbot);
						mxmlNewTextf(dataa, 0, "%s",  texmap[(texmapind > 8) ? (texmapind == 0xff) ? 9 : 10 
: texmapind ] );

						dataa = mxmlNewElement(ua9, "Color");
						mxmlNewTextf(dataa, 0, "%s", chanid[(data4.color != 255) ? data4.color : 9]);

						dataa = mxmlNewElement(ua9, "Padding1");
						mxmlNewTextf(dataa, 0, "%01x", data4.empty1);

						dataa = mxmlNewElement(ua9, "RasSelect");
						mxmlNewTextf(dataa, 0, "%s", tevswapsel[data4.ras_sel]);

						dataa = mxmlNewElement(ua9, "TexSelect");
						mxmlNewTextf(dataa, 0, "%s", tevswapsel[data4.tex_sel]);

						dataa = mxmlNewElement(ua9, "TevColorCombinerA");
						mxmlNewTextf(dataa, 0, "%01x", data4.aC);

						dataa = mxmlNewElement(ua9, "TevColorCombinerB");
						mxmlNewTextf(dataa, 0, "%01x", data4.bC);

						dataa = mxmlNewElement(ua9, "TevColorCombinerC");
						mxmlNewTextf(dataa, 0, "%01x", data4.cC);

						dataa = mxmlNewElement(ua9, "TevColorCombinerD");
						mxmlNewTextf(dataa, 0, "%01x", data4.dC);

						dataa = mxmlNewElement(ua9, "TevColorScale");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevscaleC);

						dataa = mxmlNewElement(ua9, "TevColorBias");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevbiasC);

						dataa = mxmlNewElement(ua9, "TevColorOp");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevopC);

						dataa = mxmlNewElement(ua9, "TevColorClamp");
						mxmlNewTextf(dataa, 0, "%01x", data4.clampC);

						dataa = mxmlNewElement(ua9, "TevColorRegisterID");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevregidC);

						dataa = mxmlNewElement(ua9, "TevKColorSelect");
						mxmlNewTextf(dataa, 0, "%02x", data4.selC);

						dataa = mxmlNewElement(ua9, "TevAlphaCombinerA");
						mxmlNewTextf(dataa, 0, "%01x", data4.aA);

						dataa = mxmlNewElement(ua9, "TevAlphaCombinerB");
						mxmlNewTextf(dataa, 0, "%01x", data4.bA);

						dataa = mxmlNewElement(ua9, "TevAlphaCombinerC");
						mxmlNewTextf(dataa, 0, "%01x", data4.cA);

						dataa = mxmlNewElement(ua9, "TevAlphaCombinerD");
						mxmlNewTextf(dataa, 0, "%01x", data4.dA);

						dataa = mxmlNewElement(ua9, "TevAlphaScale");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevscaleA);

						dataa = mxmlNewElement(ua9, "TevAlphaBias");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevbiasA);

						dataa = mxmlNewElement(ua9, "TevAlphaOp");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevopA);

						dataa = mxmlNewElement(ua9, "TevAlphaClamp");
						mxmlNewTextf(dataa, 0, "%01x", data4.clampA);

						dataa = mxmlNewElement(ua9, "TevAlphaRegisterID");
						mxmlNewTextf(dataa, 0, "%01x", data4.tevregidA);

						dataa = mxmlNewElement(ua9, "TevKAlphaSelect");
						mxmlNewTextf(dataa, 0, "%02x", data4.selA);

						dataa = mxmlNewElement(ua9, "IndirectTextureID");
						mxmlNewTextf(dataa, 0, "%02x", data4.indtexid);

						dataa = mxmlNewElement(ua9, "IndirectTextureFormat");
						mxmlNewTextf(dataa, 0, "%01x", data4.format);

						dataa = mxmlNewElement(ua9, "IndirectTextureBias");
						mxmlNewTextf(dataa, 0, "%01x", data4.bias);

						dataa = mxmlNewElement(ua9, "IndirectTextureMatrixID");
						mxmlNewTextf(dataa, 0, "%01x", data4.mtxid);

						dataa = mxmlNewElement(ua9, "IndirectTextureWrapS");
						mxmlNewTextf(dataa, 0, "%01x", data4.wrap_s);

						dataa = mxmlNewElement(ua9, "IndirectTextureWrapT");
						mxmlNewTextf(dataa, 0, "%01x", data4.wrap_t);

						dataa = mxmlNewElement(ua9, "IndirectTextureAddPrevious");
						mxmlNewTextf(dataa, 0, "%01x", data4.addprev);

						dataa = mxmlNewElement(ua9, "IndirectTextureUTClod");
						mxmlNewTextf(dataa, 0, "%01x", data4.utclod);

						dataa = mxmlNewElement(ua9, "IndirectTextureA");
						mxmlNewTextf(dataa, 0, "%01x", data4.aIND);

						dataa = mxmlNewElement(ua9, "Padding2");
						mxmlNewTextf(dataa, 0, "%01x", data4.empty2);

						dataa = mxmlNewElement(ua9, "Padding3");
						mxmlNewTextf(dataa, 0, "%01x", data4.empty3);

						dataa = mxmlNewElement(ua9, "Padding4");
						mxmlNewTextf(dataa, 0, "%01x", data4.empty4);

				}

				mxml_node_t *uaa;
		//# 4 * flags[8]
				for (n=0;n<bit_extract(flaggs, 8,8);n++)
				{
						brlyt_alpha_compare data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_alpha_compare));
						uaa = mxmlNewElement(entries, "AlphaCompare");
						dataa = mxmlNewElement(uaa, "comp0");
						mxmlNewTextf(dataa, 0, "%s", compare[data4.comp0]);
						dataa = mxmlNewElement(uaa, "comp1");
						mxmlNewTextf(dataa, 0, "%s", compare[data4.comp1]);
						dataa = mxmlNewElement(uaa, "aop");
						mxmlNewTextf(dataa, 0, "%s", aop[data4.aop]);
						dataa = mxmlNewElement(uaa, "ref0"); mxmlNewTextf(dataa, 0, "%02x", data4.ref0);
						dataa = mxmlNewElement(uaa, "ref1"); mxmlNewTextf(dataa, 0, "%02x", data4.ref1);
				}

				mxml_node_t *uab;
		//# 4 * flags[7]
				for (n=0;n<bit_extract(flaggs, 7,7);n++)
				{
						brlyt_blend_mode data4;
						BRLYT_ReadDataFromMemory(&data4, brlyt_file, sizeof(brlyt_blend_mode));
						uab = mxmlNewElement(entries, "BlendMode");

						dataa = mxmlNewElement(uab, "type");
						mxmlNewTextf(dataa, 0, "%s", blendmode[data4.type]);
						dataa = mxmlNewElement(uab, "src_fact");
						mxmlNewTextf(dataa, 0, "%s", blendfactor[data4.src_fact]);
						dataa = mxmlNewElement(uab, "dst_fact");
						mxmlNewTextf(dataa, 0, "%s", blendfactor[data4.dst_fact]);
						dataa = mxmlNewElement(uab, "op");
						mxmlNewTextf(dataa, 0, "%s", logicop[data4.op]);

				}
		BRLYT_fileoffset = tempDataLocation;
	}
}

void PrintBRLYTEntry_gre1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
}

void PrintBRLYTEntry_grs1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
}

void PrintBRLYTEntry_pae1(brlyt_entry entry, u8* brlyt_file, mxml_node_t *tag)
{
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
}

void PrintBRLYTEntry_pas1(brlyt_entry entry, u8* brlyt_file, mxml_node_t* tag)
{
	mxmlElementSetAttrf(tag, "type", "%.4s", entry.magic);
}

void PrintBRLYTEntries(brlyt_entry *entries, int entrycnt, u8* brlyt_file, mxml_node_t *xmlyt)
{
	int i;
	mxml_node_t *tag;
	for(i = 0; i < entrycnt; i++) {
		tag = mxmlNewElement(xmlyt, "tag");
		if((FourCCsMatch(entries[i].magic, pan1_magic) == 1)) {
			PrintBRLYTEntry_pan1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, txt1_magic) == 1)) {
			PrintBRLYTEntry_txt1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, pic1_magic) == 1)) {
			PrintBRLYTEntry_pic1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, wnd1_magic) == 1)) {
			PrintBRLYTEntry_wnd1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, bnd1_magic) == 1)) {
			PrintBRLYTEntry_bnd1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, usd1_magic) == 1)) {
			PrintBRLYTEntry_usd1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, lyt1_magic) == 1)) {
			PrintBRLYTEntry_lyt1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, grp1_magic) == 1)) {
			PrintBRLYTEntry_grp1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, txl1_magic) == 1)) {
			PrintBRLYTEntry_txl1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, fnl1_magic) == 1)) {
			PrintBRLYTEntry_fnl1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, mat1_magic) == 1)) {
			PrintBRLYTEntry_mat1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, gre1_magic) == 1)) {
			PrintBRLYTEntry_gre1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, grs1_magic) == 1)) {
			PrintBRLYTEntry_grs1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, pae1_magic) == 1)) {
			PrintBRLYTEntry_pae1(entries[i], brlyt_file, tag);
		}else if((FourCCsMatch(entries[i].magic, pas1_magic) == 1)) {
			PrintBRLYTEntry_pas1(entries[i], brlyt_file, tag);
		}else{
		}
	}
}

void parse_brlyt(char *filename, char *filenameout)
{
	SetupConstants();

	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Error! Couldn't open %s!\n", filename);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	size_t file_size = ftell(fp);
    u8* brlyt_file = (u8*)GC_MALLOC(file_size);
	fseek(fp, 0, SEEK_SET);
	fread(brlyt_file, file_size, 1, fp);
	fclose(fp);
	BRLYT_fileoffset = 0;
	brlyt_header header;
	BRLYT_ReadDataFromMemory(&header, brlyt_file, sizeof(brlyt_header));
	BRLYT_CheckHeaderSanity(header, file_size);
	brlyt_entry *entries;
	BRLYT_fileoffset = short_swap_bytes(header.lyt_offset);
	brlyt_entry_header tempentry;

	int i;
	for(i = 0; BRLYT_fileoffset < file_size; i++) {
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		BRLYT_fileoffset += be32(tempentry.length);
	}
	int entrycount = i;
    entries = (brlyt_entry*)GC_MALLOC(entrycount * sizeof(brlyt_entry));
	if(entries == NULL) {
		printf("Couldn't allocate for entries!\n");
		exit(1);
	}
	BRLYT_fileoffset = short_swap_bytes(header.lyt_offset);
	for(i = 0; i < entrycount; i++) {
		BRLYT_ReadDataFromMemoryX(&tempentry, brlyt_file, sizeof(brlyt_entry_header));
		memcpy(entries[i].magic, tempentry.magic, 4);
		entries[i].length = tempentry.length;
		entries[i].data_location = BRLYT_fileoffset + sizeof(brlyt_entry_header);
		BRLYT_fileoffset += be32(tempentry.length);
	}


	FILE *xmlFile;
	xmlFile = fopen(filenameout, "w");
	mxml_node_t *xml;
	mxml_node_t *xmlyt;
	mxmlSetWrapMargin(0);
	xml = mxmlNewXML("1.0");
	xmlyt = mxmlNewElement(xml, "xmlyt");
	mxmlElementSetAttrf(xmlyt, "version", "%d.%d.%d%s", BENZIN_VERSION_MAJOR, BENZIN_VERSION_MINOR, BENZIN_VERSION_BUILD, BENZIN_VERSION_OTHER);
	mxmlElementSetAttrf(xmlyt, "brlyt_version", "%04x", short_swap_bytes(header.version));
	PrintBRLYTEntries(entries, entrycount, brlyt_file, xmlyt);
	mxmlSaveFile(xml, xmlFile, whitespace_cb);
	fclose(xmlFile);
    mxmlDelete(xml);
}

void WritePane( mxml_node_t * tree , mxml_node_t * node , u8** tagblob , u32 * blobsize , FILE * fp , u32 * fileOffset )
{
		brlyt_pane_chunk chunk;
		char temp[256];
		memset(temp, 0, 256);
		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		memset(chunk.name, 0, 16);
		strcpy(chunk.name, temp);
		memset(temp, 0, 256);
		if(mxmlElementGetAttr(node, "userdata") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "userdata"));
		else{
			printf("No userdata attribute found!\nQuitting!\n");
			exit(1);
		}
		memset(chunk.userdata, 0, 8);
		strcpy(chunk.userdata, temp);

		mxml_node_t *subnode = mxmlFindElement(node, node, "translate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.XTrans = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.YTrans = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ZTrans = float_swap_bytes(atof(tempCoord));
			}
		}
		subnode = mxmlFindElement(node, node, "rotate", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.XRotate = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.YRotate = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "z", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.ZRotate = float_swap_bytes(atof(tempCoord));
			}
		}
		subnode = mxmlFindElement(node, node, "scale", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "x", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.XScale = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "y", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.YScale = float_swap_bytes(atof(tempCoord));
			}
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.width = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk.height = float_swap_bytes(atof(tempCoord));
			}
		}

		u8 flag1 = 0;
		subnode = mxmlFindElement(node, node, "visible", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			flag1 |= strtol(tempCoord, NULL, 16);
		}
		subnode = mxmlFindElement(node, node, "WidescreenAffected", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			flag1 |= ( strtol(tempCoord, NULL, 16) << 1 );
		}
		subnode = mxmlFindElement(node, node, "flag", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			flag1 |= ( strtol(tempCoord, NULL, 16) << 2 );
		}
		chunk.flag1 = flag1;
		subnode = mxmlFindElement(node, node, "origin", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			int i;
			char originL , originH;
			char temp[256];
			memset( temp , 0 , 256 );
			if(mxmlElementGetAttr(subnode, "x") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "x"));
			else{
				printf("No x attribute found!\nQuitting!\n");
				exit(1);
			}
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( strncmp( originX[i] , temp , 15 ) == 0 ) {
					originL = i;
					break;
				}
			}
			memset( temp , 0 , 256 );
			if(mxmlElementGetAttr(subnode, "y") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "y"));
			else{
				printf("No y attribute found!\nQuitting!\n");
				exit(1);
			}
			for ( i = 0 ; i < 3 ; i++ ) {
				if ( strncmp( originY[i] , temp , 15 ) == 0 ) {
					originH = i;
					break;
				}
			}
			chunk.origin = ( originH * 3 ) + ( originL );
		}
		subnode = mxmlFindElement(node, node, "alpha", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.alpha = strtol(tempCoord, NULL, 16);
		}
		subnode = mxmlFindElement(node, node, "padding", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk.pad = strtol(tempCoord, NULL, 16);
		}
		fwrite(&chunk, sizeof(chunk), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk);
}

void WriteBRLYTEntry( mxml_node_t * tree , mxml_node_t * node , u8** tagblob , u32 * blobsize , char temp[4] , FILE * fp , u32 * fileOffset )
{
	u32 startOfChunk = *fileOffset;

	char lyt1[4] = {'l', 'y', 't', '1'};
	char txl1[4] = {'t', 'x', 'l', '1'};
	char fnl1[4] = {'f', 'n', 'l', '1'};
	char mat1[4] = {'m', 'a', 't', '1'};
	char pan1[4] = {'p', 'a', 'n', '1'};
	char wnd1[4] = {'w', 'n', 'd', '1'};
	char bnd1[4] = {'b', 'n', 'd', '1'};
	char pic1[4] = {'p', 'i', 'c', '1'};
	char txt1[4] = {'t', 'x', 't', '1'};
	char grp1[4] = {'g', 'r', 'p', '1'};
	char grs1[4] = {'g', 'r', 's', '1'};
	char gre1[4] = {'g', 'r', 'e', '1'};
	char pas1[4] = {'p', 'a', 's', '1'};
	char pae1[4] = {'p', 'a', 'e', '1'};
	char usd1[4] = {'u', 's', 'd', '1'};

	if ( memcmp(temp, lyt1, sizeof(lyt1)) == 0)
	{
		brlyt_lytheader_chunk lytheader;

		mxml_node_t *subnode = mxmlFindElement(node , node , "drawnFromMiddle", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempChar[256];
			get_value(subnode, tempChar, 256);
			lytheader.drawnFromMiddle = atoi(tempChar);
			lytheader.pad[0]=0;lytheader.pad[1]=0;lytheader.pad[2]=0;
		}
		subnode = mxmlFindElement(node, node, "size", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode = mxmlFindElement(subnode , subnode , "width", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				get_value(valnode, tempChar, 256);
				*(float*)(&(lytheader.width)) = atof(tempChar);
			}
			valnode = mxmlFindElement(subnode , subnode ,"height", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				get_value(valnode, tempChar, 256);

				*(float*)(&(lytheader.height)) = atof(tempChar);
			}
		}
		lytheader.pad[0] = 0; lytheader.pad[1] = 0; lytheader.pad[2] = 0;
		lytheader.width = float_swap_bytes(lytheader.width);
		lytheader.height = float_swap_bytes(lytheader.height);
		fwrite(&lytheader, sizeof(brlyt_header), 1, fp);
		*fileOffset = *fileOffset + sizeof(lytheader);
	}
	if ( memcmp(temp, txl1, sizeof(txl1)) == 0)
	{
		int numoffsOffset = ftell(fp);

		brlyt_numoffs_chunk chunk;
		chunk.num = 0;
		chunk.offs = 0;
		fwrite(&chunk, sizeof(chunk), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk);

		char *names;
        names = GC_MALLOC(sizeof(names));
		int lengthOfNames = 0;
		numberOfTextures = 0;
		mxml_node_t *subnode = mxmlFindElement(node, node, "entries", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			int numEntries = 0;

			int oldNameLength;
			int baseOffset = 0;

			u32 *offsunks;

            offsunks = GC_MALLOC(sizeof(u32));

			brlyt_offsunk_chunk chunk2;
			mxml_node_t *valnode;
			for(valnode = mxmlFindElement(subnode, subnode, "name", NULL, NULL, MXML_DESCEND); valnode != NULL; valnode = mxmlFindElement(valnode, subnode, "name", NULL, NULL, MXML_DESCEND)) {
				if (valnode != NULL)
				{
					char tempSub[256];
					get_value(valnode, tempSub, 256);

					oldNameLength = lengthOfNames;
					lengthOfNames += strlen(tempSub);

                    offsunks = GC_REALLOC(offsunks, (numEntries + 1)*(2 * sizeof(u32)));
                    if (offsunks == NULL) printf("NULLed by a GC_REALLOC\n");

					offsunks[(numEntries*2)+0] = baseOffset;
					offsunks[(numEntries*2)+1] = 0;

					*fileOffset = *fileOffset + sizeof(chunk2);
					numEntries++;
					numberOfTextures++;

					baseOffset += (strlen(tempSub) + 1);

                    names = GC_REALLOC(names, 1 + sizeof(char) * lengthOfNames);
					strcpy(&names[oldNameLength], tempSub);
					char nuller = '\0';
					memcpy(&names[lengthOfNames], &nuller, sizeof(char));
					lengthOfNames += 1;
				}
			}
			chunk.num = short_swap_bytes(numEntries);
			chunk.offs = 0;
			fseek(fp, numoffsOffset, SEEK_SET);
			fwrite(&chunk, sizeof(chunk), 1, fp);

			int i;
			for(i=0;i<numEntries;i++) {
				offsunks[(i*2)+0] = be32(offsunks[(i*2)+0] + (numEntries * 8));
				offsunks[i*2+1] = 0;

				fwrite(&offsunks[i*2], sizeof(u32), 1, fp);
				fwrite(&offsunks[i*2+1], sizeof(u32), 1, fp);
            }
		}
		fseek(fp, *fileOffset, SEEK_SET);
		fwrite(names, lengthOfNames, 1, fp);
		*fileOffset = *fileOffset + lengthOfNames;
		if ((*fileOffset % 4) > 0)
		{
			u8 toAdd = 4-(*fileOffset % 4);
			char nuller[3] = {'\0', '\0', '\0'};
			fwrite(&nuller, sizeof(char), toAdd, fp);
			*fileOffset = *fileOffset + toAdd;
		}
        textures = GC_MALLOC(lengthOfNames * sizeof(char));
        memcpy(textures, names, lengthOfNames * sizeof(char));
	}
	if ( memcmp(temp, fnl1, sizeof(fnl1)) == 0)
	{
		brlyt_numoffs_chunk chunk;
		chunk.num = 0;
		chunk.offs = 0;
		fwrite(&chunk, sizeof(chunk), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk);

		char *names;
        names = GC_MALLOC(sizeof(char));
		int lengthOfNames = 0;
		mxml_node_t *subnode = mxmlFindElement(node, node, "entries", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			int numEntries = 0;

			int oldNameLength;
			int baseOffset = 0;
			int numoffsOffset = *fileOffset - 4;

			u32 *offsunks;
            offsunks = GC_MALLOC(sizeof(u32));

			brlyt_offsunk_chunk chunk2;
			mxml_node_t *valnode;
			for(valnode = mxmlFindElement(subnode, subnode, "name", NULL, NULL, MXML_DESCEND); valnode != NULL; valnode = mxmlFindElement(valnode, subnode, "name", NULL, NULL, MXML_DESCEND)) {
				if (valnode != NULL)
				{
					char tempSub[256];
					get_value(valnode, tempSub, 256);

					oldNameLength = lengthOfNames;
					lengthOfNames += strlen(tempSub);

                    offsunks = GC_REALLOC(offsunks, (numEntries + 1)*(2 * sizeof(u32)));
					offsunks[(numEntries*2)+0] = baseOffset;
					offsunks[(numEntries*2)+1] = 0;

					*fileOffset = *fileOffset + sizeof(chunk2);
					numEntries++;

					baseOffset += (strlen(tempSub) + 1);
                    names = GC_REALLOC(names, 1 + sizeof(char) * lengthOfNames);
					memcpy(&names[oldNameLength], tempSub, strlen(tempSub));
					char nuller = '\0';
					memcpy(&names[lengthOfNames], &nuller, sizeof(char));
					lengthOfNames += 1;
				}
			}
			chunk.num = short_swap_bytes(numEntries);
			chunk.offs = 0;
			fseek(fp, numoffsOffset, SEEK_SET);
			fwrite(&chunk, sizeof(chunk), 1, fp);

			int i;
			for(i=0;i<numEntries;i++) {
				offsunks[(i*2)+0] = be32(offsunks[(i*2)+0] + (numEntries * 8));
				offsunks[i*2+1] = 0;

				fwrite(&offsunks[i*2], sizeof(u32), 1, fp);
				fwrite(&offsunks[i*2+1], sizeof(u32), 1, fp);
            }
		}
		fseek(fp, *fileOffset, SEEK_SET);
		fwrite(names, lengthOfNames, 1, fp);
		*fileOffset = *fileOffset + lengthOfNames;
		if ((*fileOffset % 4) > 0)
		{
			u8 toAdd = 4-(*fileOffset % 4);
			char nuller[3] = {'\0', '\0', '\0'};
			fwrite(&nuller, sizeof(char), toAdd, fp);
			*fileOffset = *fileOffset + toAdd;
        }
	}
	if ( memcmp(temp, mat1, sizeof(mat1)) == 0)
	{
		numberOfMaterials = 0;
		int numberOfEntries = 0;
		int actualNumber = 0;
		brlyt_numoffs_chunk numchunk;
		brlyt_material_chunk chunk;
		mxml_node_t *subnode;
		for(subnode=mxmlFindElement(node, node, "entries", NULL, NULL, MXML_DESCEND);subnode!=NULL;subnode=mxmlFindElement(subnode, node, "entries", NULL, NULL, MXML_DESCEND))
			numberOfEntries++;

		numchunk.num = short_swap_bytes(numberOfEntries);
		numchunk.offs = 0;
		fwrite(&numchunk, sizeof(numchunk), 1, fp);
		*fileOffset = *fileOffset + sizeof(numchunk);

        int *offsets = GC_MALLOC(numberOfEntries * sizeof(int));
		int offsetsOffset = ftell(fp);
		int materialOffset = 0;
		actualNumber = numberOfEntries;
		numberOfEntries = 0;
		int matSize = 0;
		int initialOffset = sizeof(numchunk) + 8 + (actualNumber * sizeof(int));

		offsets[numberOfEntries] = be32(matSize + initialOffset);
		fwrite(&offsets[0], sizeof(int), actualNumber, fp);
		*fileOffset = *fileOffset + (actualNumber * 4);

		for(subnode=mxmlFindElement(node,node,"entries",NULL,NULL,MXML_DESCEND);subnode!=NULL;subnode=mxmlFindElement(subnode,node,"entries",NULL,NULL,MXML_DESCEND))
		{
			numberOfEntries += 1;
			numberOfMaterials++;
			char temp[256];
			if(mxmlElementGetAttr(subnode, "name") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "name"));
			else{
				printf("No name attribute found!\nQuitting!\n");
				exit(1);
			}
			memset(chunk.name, 0, 20);
			strcpy(chunk.name, temp);

            materials = GC_REALLOC(materials, lengthOfMaterials + strlen(chunk.name) + 1);
			memcpy(&materials[lengthOfMaterials], chunk.name, strlen(chunk.name) + 1);
			lengthOfMaterials += (1 + strlen(chunk.name));
			numberOfMaterials += 1;

			mxml_node_t *setnode;
			setnode = mxmlFindElement(subnode, subnode, "colors", NULL, NULL, MXML_DESCEND);
			if (setnode != NULL)
			{
				mxml_node_t *valnode;
				valnode=mxmlFindElement(setnode, setnode, "forecolor", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					char tempCoord[256];
					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "r") );
					chunk.forecolor[0] = strtol(tempCoord, NULL, 10);
					chunk.forecolor[0] = short_swap_bytes(chunk.forecolor[0]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "g") );
					chunk.forecolor[1] = strtol(tempCoord, NULL, 10);
					chunk.forecolor[1] = short_swap_bytes(chunk.forecolor[1]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "b") );
					chunk.forecolor[2] = strtol(tempCoord, NULL, 10);
					chunk.forecolor[2] = short_swap_bytes(chunk.forecolor[2]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "a") );
					chunk.forecolor[3] = strtol(tempCoord, NULL, 10);
					chunk.forecolor[3] = short_swap_bytes(chunk.forecolor[3]);
				}
				valnode=mxmlFindElement(setnode, setnode, "backcolor", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					char tempCoord[256];
					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "r") );
					chunk.backcolor[0] = strtol(tempCoord, NULL, 10);
					chunk.backcolor[0] = short_swap_bytes(chunk.backcolor[0]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "g") );
					chunk.backcolor[1] = strtol(tempCoord, NULL, 10);
					chunk.backcolor[1] = short_swap_bytes(chunk.backcolor[1]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "b") );
					chunk.backcolor[2] = strtol(tempCoord, NULL, 10);
					chunk.backcolor[2] = short_swap_bytes(chunk.backcolor[2]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "a") );
					chunk.backcolor[3] = strtol(tempCoord, NULL, 10);
					chunk.backcolor[3] = short_swap_bytes(chunk.backcolor[3]);
				}
				valnode=mxmlFindElement(setnode, setnode, "colorReg3", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					char tempCoord[256];
					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "r") );
					chunk.colorReg3[0] = strtol(tempCoord, NULL, 10);
					chunk.colorReg3[0] = short_swap_bytes(chunk.colorReg3[0]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "g") );
					chunk.colorReg3[1] = strtol(tempCoord, NULL, 10);
					chunk.colorReg3[1] = short_swap_bytes(chunk.colorReg3[1]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "b") );
					chunk.colorReg3[2] = strtol(tempCoord, NULL, 10);
					chunk.colorReg3[2] = short_swap_bytes(chunk.colorReg3[2]);

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "a") );
					chunk.colorReg3[3] = strtol(tempCoord, NULL, 10);
					chunk.colorReg3[3] = short_swap_bytes(chunk.colorReg3[3]);
				}

				valnode=mxmlFindElement(setnode, setnode, "tev_k1", NULL, NULL, MXML_DESCEND);
				{
					u32 red, green, blue, alpha;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
					red = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
					green = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
					blue = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
					alpha = strtoul(tempCoord, NULL, 16);

					chunk.tev_kcolor[0] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
					chunk.tev_kcolor[0] = int_swap_bytes( chunk.tev_kcolor[0] );
				}
				valnode=mxmlFindElement(setnode, setnode, "tev_k2", NULL, NULL, MXML_DESCEND);
				{
					u32 red, green, blue, alpha;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
					red = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
					green = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
					blue = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
					alpha = strtoul(tempCoord, NULL, 16);

					chunk.tev_kcolor[1] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
					chunk.tev_kcolor[1] = int_swap_bytes( chunk.tev_kcolor[1] );
				}
				valnode=mxmlFindElement(setnode, setnode, "tev_k3", NULL, NULL, MXML_DESCEND);
				{
					u32 red, green, blue, alpha;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
					red = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
					green = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
					blue = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
					alpha = strtoul(tempCoord, NULL, 16);

					chunk.tev_kcolor[2] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
					chunk.tev_kcolor[2] = int_swap_bytes( chunk.tev_kcolor[2] );
				}
				valnode=mxmlFindElement(setnode, setnode, "tev_k4", NULL, NULL, MXML_DESCEND);
				{
					u32 red, green, blue, alpha;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
					red = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
					green = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
					blue = strtoul(tempCoord, NULL, 16);

					memset( tempCoord , 0 , 256 );
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
					alpha = strtoul(tempCoord, NULL, 16);

					chunk.tev_kcolor[3] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
					chunk.tev_kcolor[3] = int_swap_bytes( chunk.tev_kcolor[3] );
				}
			}
			/*
			setnode = mxmlFindElement(subnode,subnode,"flags",NULL,NULL,MXML_DESCEND);
			if (setnode != NULL)
			{
				char tempCoord[256];
				get_value(setnode, tempCoord, 256);
				chunk.flags = be32(strtoul(tempCoord, NULL, 16));
			}
			*/

			u32 tempFileOffset = ftell(fp);
			mat1_flags flags;
			flags.flags = 0;
			chunk.flags = 0;

			fwrite(&chunk, sizeof(chunk), 1, fp);
			*fileOffset = *fileOffset + sizeof(chunk);
			matSize += sizeof(chunk);

			brlyt_texref_chunk chunkTexRef;
			for(setnode = mxmlFindElement(subnode, subnode, "texture", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "texture", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char temp[256];
				if(mxmlElementGetAttr(setnode, "name") != NULL)
					strcpy(temp, mxmlElementGetAttr(setnode, "name"));
				else{
					printf("No name attribute found!\nQuitting!\n");
					exit(1);
				}
				chunkTexRef.tex_offs = short_swap_bytes(findTexOffset(temp));

				valnode=mxmlFindElement(setnode, setnode, "wrap_s", NULL, NULL, MXML_DESCEND);

				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunkTexRef.wrap_s = strtoul(tempCoord, NULL, 16);
				for ( l = 0 ; l < 4 ; l++ ) {
					if ( strncmp( wraps[l] , tempCoord , 17 ) == 0 ) {
						chunkTexRef.wrap_s = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "wrap_t", NULL, NULL, MXML_DESCEND);

				get_value(valnode, tempCoord, 256);
				chunkTexRef.wrap_t = strtoul(tempCoord, NULL, 16);
				for ( l = 0 ; l < 4 ; l++ ) {
					if ( strncmp( wraps[l] , tempCoord , 17 ) == 0 ) {
						chunkTexRef.wrap_t = l;
						break;
					}
				}

				flags.flag.Texture++;

				fwrite(&chunkTexRef, sizeof(chunkTexRef), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkTexRef);
				matSize += sizeof(chunkTexRef);
			}

			brlyt_tex_srt chunkTexCoords;
			for(setnode = mxmlFindElement(subnode, subnode, "TextureSRT", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "TextureSRT", NULL, NULL, MXML_DESCEND))
			{
				char tempCoord[256];
				mxml_node_t *valnode;
				valnode=mxmlFindElement(setnode, setnode, "XTrans", NULL, NULL, MXML_DESCEND);
		memset(tempCoord, 0, 256);
				get_value(valnode, tempCoord, 256);
				chunkTexCoords.XTrans = float_swap_bytes(atof(tempCoord));
				valnode=mxmlFindElement(setnode, setnode, "YTrans", NULL, NULL, MXML_DESCEND);
		memset(tempCoord, 0, 256);
				get_value(valnode, tempCoord, 256);
				chunkTexCoords.YTrans = float_swap_bytes(atof(tempCoord));
				valnode=mxmlFindElement(setnode, setnode, "Rotate", NULL, NULL, MXML_DESCEND);
		memset(tempCoord, 0, 256);
				get_value(valnode, tempCoord, 256);
				chunkTexCoords.Rotate = float_swap_bytes(atof(tempCoord));
				valnode=mxmlFindElement(setnode, setnode, "XScale", NULL, NULL, MXML_DESCEND);
		memset(tempCoord, 0, 256);
				get_value(valnode, tempCoord, 256);
				chunkTexCoords.XScale = float_swap_bytes(atof(tempCoord));
				valnode=mxmlFindElement(setnode, setnode, "YScale", NULL, NULL, MXML_DESCEND);
		memset(tempCoord, 0, 256);
				get_value(valnode, tempCoord, 256);
				chunkTexCoords.YScale = float_swap_bytes(atof(tempCoord));

				flags.flag.TexSRT++;

				fwrite(&chunkTexCoords, sizeof(chunkTexCoords), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkTexCoords);
				matSize += sizeof(chunkTexCoords);
			}

			brlyt_tex_coordgen chunkUa3;
			for(setnode = mxmlFindElement(subnode, subnode, "CoordGen", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "CoordGen", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "tgen_type", NULL, NULL, MXML_DESCEND);
				memset(tempCoord,0,256);
				get_value( valnode , tempCoord , 256 );
				chunkUa3.tgen_type = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 11 ; l++ ){
					if ( strncmp( tgen_types[l] , tempCoord , 14 ) == 0 ) {
						chunkUa3.tgen_type = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "tgen_src", NULL, NULL, MXML_DESCEND);
				memset(tempCoord,0,256);
				get_value( valnode , tempCoord , 256 );
				chunkUa3.tgen_src = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 21 ; l++ ){
					if ( strncmp( tgen_src[l] , tempCoord , 20 ) == 0 ) {
						chunkUa3.tgen_src = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "MatrixSource", NULL, NULL, MXML_DESCEND);
				memset(tempCoord,0,256);
				get_value( valnode , tempCoord , 256 );
				chunkUa3.mtxsrc = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 42 ; l++ ){
					if ( strncmp( mtxsrc[l] , tempCoord , 14 ) == 0 ) {
						chunkUa3.mtxsrc = l*3;
						break;
					}
				}


				valnode=mxmlFindElement(setnode, setnode, "padding", NULL, NULL, MXML_DESCEND);
				memset(tempCoord,0,256);
				get_value( valnode , tempCoord , 256 );
				chunkUa3.padding = strtoul( tempCoord , NULL , 16 );
				// padding should be 0
				chunkUa3.padding = 0;

				flags.flag.TexCoord++;

				fwrite(&chunkUa3, sizeof(chunkUa3), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa3);
				matSize += sizeof(chunkUa3);
			}

			brlyt_tex_chanctrl chunkUa4;
			for(setnode = mxmlFindElement(subnode, subnode, "ChanControl", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "ChanControl", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "ColorMatSource", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa4.color_matsrc = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 2 ; l++ ){
					if ( strncmp( matsrc[l] , tempCoord , 14 ) == 0 ) {
						chunkUa4.color_matsrc = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "AlphaMatSource", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa4.alpha_matsrc = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 2 ; l++ ){
					if ( strncmp( matsrc[l] , tempCoord , 14 ) == 0 ) {
						chunkUa4.alpha_matsrc = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "Padding1", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa4.padding1 = strtoul( tempCoord , NULL , 16 );
				// padding should be 0
				chunkUa4.padding1 = 0;

				valnode=mxmlFindElement(setnode, setnode, "Padding2", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa4.padding2 = strtoul( tempCoord , NULL , 16 );
				// padding should be 0
				chunkUa4.padding2 = 0;

				flags.flag.ua4++;

				fwrite(&chunkUa4, sizeof(chunkUa4), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa4);
				matSize += sizeof(chunkUa4);
			}

			brlyt_tex_matcol chunkUa5;
			for(setnode = mxmlFindElement(subnode, subnode, "MaterialColor", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "MaterialColor", NULL, NULL, MXML_DESCEND))
			{
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "red", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa5.red = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "green", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa5.green = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "blue", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa5.blue = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "alpha", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa5.alpha = strtoul( tempCoord , NULL , 16 );

				flags.flag.ua5++;

				fwrite(&chunkUa5, sizeof(chunkUa5), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa5);
				matSize += sizeof(chunkUa5);
			}
			brlyt_tev_swapmodetable chunkUa6;
			for(setnode = mxmlFindElement(subnode, subnode, "TevSwapModeTable", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "TevSwapModeTable", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "OneRed", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				char red = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						red = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "OneGreen", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				char green = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						green = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "OneBlue", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				char blue = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						blue = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "OneAlpha", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				char alpha = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						alpha = l;
						break;
					}
				}

				chunkUa6.one = ( red << 0 ) | ( green << 2 ) | ( blue << 4 ) | ( alpha << 6 );

				valnode=mxmlFindElement(setnode, setnode, "TwoRed", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				red = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						red = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "TwoGreen", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				green = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						green = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "TwoBlue", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				blue = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						blue = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "TwoAlpha", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				alpha = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						alpha = l;
						break;
					}
				}

				chunkUa6.two = ( red << 0 ) | ( green << 2 ) | ( blue << 4 ) | ( alpha << 6 );

				valnode=mxmlFindElement(setnode, setnode, "ThreeRed", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				red = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						red = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "ThreeGreen", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				green = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						green = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "ThreeBlue", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				blue = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						blue = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "ThreeAlpha", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				alpha = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						alpha = l;
						break;
					}
				}

				chunkUa6.three = ( red << 0 ) | ( green << 2 ) | ( blue << 4 ) | ( alpha << 6 );

				valnode=mxmlFindElement(setnode, setnode, "FourRed", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				red = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						red = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "FourGreen", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				green = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						green = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "FourBlue", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				blue = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						blue = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "FourAlpha", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				alpha = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
					if ( strncmp( tevcolor[l] , tempCoord , 14 ) == 0 ) {
						alpha = l;
						break;
					}
				}

				chunkUa6.four = ( red << 0 ) | ( green << 2 ) | ( blue << 4 ) | ( alpha << 6 );

				flags.flag.ua6++;

				fwrite(&chunkUa6, sizeof(chunkUa6), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa6);
				matSize += sizeof(chunkUa6);
			}

			brlyt_indtex_srt chunkUa7;
			for(setnode = mxmlFindElement(subnode, subnode, "IndTextureSRT", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "IndTextureSRT", NULL, NULL, MXML_DESCEND))
			{
				mxml_node_t *valnode;
				valnode=mxmlFindElement(setnode, setnode, "XTrans", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
						char tempCoord[256];
						get_value(valnode, tempCoord, 256);
						chunkUa7.XTrans = float_swap_bytes(atof(tempCoord));
				}
				valnode=mxmlFindElement(setnode, setnode, "YTrans", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
						char tempCoord[256];
						get_value(valnode, tempCoord, 256);
						chunkUa7.YTrans = float_swap_bytes(atof(tempCoord));
				}
				valnode=mxmlFindElement(setnode, setnode, "Rotate", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
						char tempCoord[256];
						get_value(valnode, tempCoord, 256);
						chunkUa7.Rotate = float_swap_bytes(atof(tempCoord));
				}
				valnode=mxmlFindElement(setnode, setnode, "XScale", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
						char tempCoord[256];
						get_value(valnode, tempCoord, 256);
						chunkUa7.XScale = float_swap_bytes(atof(tempCoord));
				}
				valnode=mxmlFindElement(setnode, setnode, "YScale", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
						char tempCoord[256];
						get_value(valnode, tempCoord, 256);
						chunkUa7.YScale = float_swap_bytes(atof(tempCoord));
				}

				flags.flag.ua7++;

				fwrite(&chunkUa7, sizeof(chunkUa7), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa7);
				matSize += sizeof(chunkUa7);
			}
			brlyt_indtex_order chunkUa8;
			for(setnode = mxmlFindElement(subnode, subnode, "IndTextureOrder", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "IndTextureOrder", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "TextureCoordinate", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa8.tex_coord = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 10 ; l++ ){
                                        if ( strncmp( texcoord[l] , tempCoord , 20 ) == 0 ) {
                                                chunkUa8.tex_coord = (l == 9) ? 0xff : l ;
                                                break;
                                        }
                                }


				valnode=mxmlFindElement(setnode, setnode, "TextureMap", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa8.tex_map = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 11 ; l++ ){
                                        if ( strncmp( texmap[l] , tempCoord , 20 ) == 0 ) {
                                                if(l == 9)
							l = 0xff;
						if(l == 10)
							l = 0x100;
						chunkUa8.tex_map = l;
                                                break;
                                        }
                                }

				
				valnode=mxmlFindElement(setnode, setnode, "ScaleS", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa8.scale_s = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 10 ; l++ ){
					if ( strncmp( scale[l] , tempCoord , 14 ) == 0 ) {
						chunkUa8.scale_s = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "ScaleT", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa8.scale_t = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 10 ; l++ ){
					if ( strncmp( scale[l] , tempCoord , 14 ) == 0 ) {
						chunkUa8.scale_t = l;
						break;
					}
				}

				flags.flag.ua8++;

				fwrite(&chunkUa8, sizeof(chunkUa8), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa8);
				matSize += sizeof(chunkUa8);
			}

			TevStages chunkUa9;
			for (setnode = mxmlFindElement(subnode, subnode, "TevStage", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "TevStage", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "TextureCoordinate", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.texcoord = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 10 ; l++ ){
                                        if ( strncmp( texcoord[l] , tempCoord , 20 ) == 0 ) {
                                                chunkUa9.texcoord = (l == 9) ? 0xff : l;
                                                break;
                                        }
                                }


				valnode=mxmlFindElement(setnode, setnode, "TextureMap", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				u32 tempval = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 11 ; l++ ){
                                        if ( strncmp( texmap[l] , tempCoord , 20 ) == 0 ) {
                                                if(l == 9)
                                                        l = 0xff;
                                                if(l == 10)
                                                        l = 0x100;
                                                tempval = l;
                                                break;
                                        }
                                }

				chunkUa9.texmapbot = tempval & 0xff;
				chunkUa9.texmaptop = (tempval & 0x100) >> 8;

				valnode=mxmlFindElement(setnode, setnode, "Color", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.color = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 10 ; l++ ){
					if ( strncmp( chanid[l] , tempCoord , 14 ) == 0 ) {
						chunkUa9.color = (l != 9) ? l : 255;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "Padding1", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.empty1 = strtoul( tempCoord , NULL , 16 );
				chunkUa9.empty1 = 0;

				valnode=mxmlFindElement(setnode, setnode, "RasSelect", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.ras_sel = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
                                        if ( strncmp( tevswapsel[l] , tempCoord , 20 ) == 0 ) {
                                                chunkUa9.ras_sel = l;
                                                break;
                                        }
                                }

				
				valnode=mxmlFindElement(setnode, setnode, "TexSelect", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tex_sel = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 4 ; l++ ){
                                        if ( strncmp( tevswapsel[l] , tempCoord , 20 ) == 0 ) {
                                                chunkUa9.tex_sel = l;
                                                break;
                                        }
                                }
				
				valnode=mxmlFindElement(setnode, setnode, "TevColorCombinerA", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.aC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorCombinerB", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.bC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorCombinerC", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.cC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorCombinerD", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.dC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorOp", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevopC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorBias", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevbiasC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorScale", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevscaleC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorClamp", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.clampC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevColorRegisterID", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevregidC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevKColorSelect", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.selC = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaCombinerA", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.aA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaCombinerB", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.bA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaCombinerC", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.cA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaCombinerD", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.dA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaOp", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevopA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaBias", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevbiasA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaScale", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevscaleA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaClamp", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.clampA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevAlphaRegisterID", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.tevregidA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "TevKAlphaSelect", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.selA = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureID", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.indtexid = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureFormat", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.format = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureBias", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.bias = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureMatrixID", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.mtxid = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureWrapS", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.wrap_s = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureWrapT", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.wrap_t = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureAddPrevious", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.addprev = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureUTClod", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.utclod = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "IndirectTextureA", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.aIND = strtoul( tempCoord , NULL , 16 );

				valnode=mxmlFindElement(setnode, setnode, "Padding2", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.empty2 = strtoul( tempCoord , NULL , 16 );
				chunkUa9.empty2 = 0;

				valnode=mxmlFindElement(setnode, setnode, "Padding3", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.empty3 = strtoul( tempCoord , NULL , 16 );
				chunkUa9.empty3 = 0;

				valnode=mxmlFindElement(setnode, setnode, "Padding4", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUa9.empty4 = strtoul( tempCoord , NULL , 16 );
				chunkUa9.empty4 = 0;

				flags.flag.ua9++;

				fwrite(&chunkUa9, sizeof(chunkUa9), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUa9);
				matSize += sizeof(chunkUa9);
			}

			brlyt_alpha_compare chunkUaa;
			for(setnode = mxmlFindElement(subnode, subnode, "AlphaCompare", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "AlphaCompare", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "comp0", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value(valnode, tempCoord, 256);
				chunkUaa.comp0 = strtoul(tempCoord, NULL, 16);
				for ( l = 0 ; l < 8 ; l++ ){
					if ( strncmp( compare[l] , tempCoord , 14 ) == 0 ) {
						chunkUaa.comp0 = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "comp1", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value(valnode, tempCoord, 256);
				chunkUaa.comp1 = strtoul(tempCoord, NULL, 16);
				for ( l = 0 ; l < 8 ; l++ ){
					if ( strncmp( compare[l] , tempCoord , 14 ) == 0 ) {
						chunkUaa.comp1 = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "aop", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value(valnode, tempCoord, 256);
				chunkUaa.aop = strtoul(tempCoord, NULL, 16);
				for ( l = 0 ; l < 5 ; l++ ){
					if ( strncmp( aop[l] , tempCoord , 14 ) == 0 ) {
						chunkUaa.aop = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "ref0", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value(valnode, tempCoord, 256);
				chunkUaa.ref0 = strtoul(tempCoord, NULL, 16);

				valnode=mxmlFindElement(setnode, setnode, "ref1", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value(valnode, tempCoord, 256);
				chunkUaa.ref1 = strtoul(tempCoord, NULL, 16);

				flags.flag.uaa++;

				fwrite(&chunkUaa, sizeof(chunkUaa), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUaa);
				matSize += sizeof(chunkUaa);
			}

			brlyt_blend_mode chunkUab;
			for(setnode = mxmlFindElement(subnode, subnode, "BlendMode", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "BlendMode", NULL, NULL, MXML_DESCEND))
			{
				int l;
				mxml_node_t *valnode;
				char tempCoord[256];

				valnode=mxmlFindElement(setnode, setnode, "type", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUab.type = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 5 ; l++ ){
					if ( strncmp( blendmode[l] , tempCoord , 14 ) == 0 ) {
						chunkUab.type = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "src_fact", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUab.src_fact = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 8 ; l++ ){
					if ( strncmp( blendfactor[l] , tempCoord , 14 ) == 0 ) {
						chunkUab.src_fact = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "dst_fact", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUab.dst_fact = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 8 ; l++ ){
					if ( strncmp( blendfactor[l] , tempCoord , 14 ) == 0 ) {
						chunkUab.dst_fact = l;
						break;
					}
				}

				valnode=mxmlFindElement(setnode, setnode, "op", NULL, NULL, MXML_DESCEND);
				memset( tempCoord , 0 , 256 );
				get_value( valnode , tempCoord , 256 );
				chunkUab.op = strtoul( tempCoord , NULL , 16 );
				for ( l = 0 ; l < 16 ; l++ ){
					if ( strncmp( logicop[l] , tempCoord , 14 ) == 0 ) {
						chunkUab.op = l;
						break;
					}
				}

				flags.flag.uab++;

				fwrite(&chunkUab, sizeof(chunkUab), 1, fp);
				*fileOffset = *fileOffset + sizeof(chunkUab);
				matSize += sizeof(chunkUab);
			}

			// write mat1_header with flags
			chunk.flags = be32( flags.flags );
			u32 end = ftell(fp);
			fseek( fp , tempFileOffset , SEEK_SET );
			fwrite(&chunk, sizeof(chunk), 1, fp);
			fseek( fp , end , SEEK_SET );

			if (numberOfEntries < actualNumber)
			{
				offsets[numberOfEntries] = be32(matSize + initialOffset);
				materialOffset = ftell(fp);
				fseek(fp, offsetsOffset+(numberOfEntries * sizeof(int)), SEEK_SET);
				fwrite(&offsets[numberOfEntries], sizeof(int), 1, fp);
				fseek(fp, materialOffset, SEEK_SET);
				initialOffset += matSize;
				matSize = 0;
			}
        }
	}
	if ( memcmp(temp, pan1, sizeof(pan1)) == 0)
	{
		WritePane( tree, node, tagblob, blobsize, fp, fileOffset );
	}
	if ( memcmp(temp, bnd1, sizeof(bnd1)) == 0)
	{
		WritePane( tree, node, tagblob, blobsize, fp, fileOffset );
	}
	if ( memcmp(temp, wnd1, sizeof(wnd1)) == 0)
	{
		WritePane( tree, node, tagblob, blobsize, fp, fileOffset );

		int i;
		brlyt_wnd wndy;
		mxml_node_t * subnode;
		subnode = mxmlFindElement(node, node, "wnd", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			i = 0;
			mxml_node_t *subsubnode;
			for(subsubnode = mxmlFindElement(subnode, subnode, "coordinate", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "coordinate", NULL, NULL, MXML_DESCEND))
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy.coords[i] = float_swap_bytes(atof(tempCoord));
				i++;
			}
			subsubnode = mxmlFindElement(subnode, subnode, "FrameCount", NULL, NULL, MXML_DESCEND);
			if(subsubnode != NULL)
			{

				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy.frame_count = (u8)(strtoul(tempCoord, NULL, 16));
			}
			wndy.padding[0] = 0x0;
			wndy.padding[1] = 0x0;
			wndy.padding[2] = 0x0;

			subsubnode = mxmlFindElement(subnode, subnode, "offset1", NULL, NULL, MXML_DESCEND);
			if(subsubnode != NULL)
			{

				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy.offset1 = be32(strtoul(tempCoord, NULL, 16));
			}
			subsubnode = mxmlFindElement(subnode, subnode, "offset2", NULL, NULL, MXML_DESCEND);
			if(subsubnode != NULL)
			{

				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy.offset2 = be32(strtoul(tempCoord, NULL, 16));
			}
			fwrite(&wndy, sizeof(brlyt_wnd), 1, fp);
			*fileOffset = *fileOffset + sizeof(brlyt_wnd);
		}

		brlyt_wnd1 wndy1;
		subnode = mxmlFindElement(node, node, "wnd1", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			i=0;
			mxml_node_t *subsubnode;
			for(subsubnode = mxmlFindElement(subnode, subnode, "color", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "color", NULL, NULL, MXML_DESCEND))
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy1.colors[i] = be32(strtoul(tempCoord, NULL, 16));
				i++;
			}
			subsubnode = mxmlFindElement(subnode, subnode, "material", NULL, NULL, MXML_DESCEND);
			if (subnode != NULL)
			{
				char temp[256];
				if(mxmlElementGetAttr(subsubnode, "name") != NULL)
					strcpy(temp, mxmlElementGetAttr(subsubnode, "name"));
				else{
					printf("No name attribute found!\nQuitting!\n");
					exit(1);
				}
				wndy1.material = short_swap_bytes(findMatOffset(temp));
			}
			subsubnode = mxmlFindElement(subnode, subnode, "CoordinateCount", NULL, NULL, MXML_DESCEND);
			if (subsubnode != NULL)
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy1.coordinate_count = short_swap_bytes(strtoul(tempCoord, NULL, 16));
			}
			subsubnode = mxmlFindElement(subnode, subnode, "padding", NULL, NULL, MXML_DESCEND);
			if (subsubnode != NULL)
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy1.padding = short_swap_bytes(strtoul(tempCoord, NULL, 16));
			}
			fwrite(&wndy1, sizeof(brlyt_wnd1), 1, fp);
			*fileOffset = *fileOffset + sizeof(brlyt_wnd1);
		}

		brlyt_wnd3 wndy3;
		subnode = mxmlFindElement(node, node, "Coords", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			i=0;
			mxml_node_t *subsubnode;
			for(subsubnode = mxmlFindElement(subnode, subnode, "texcoord", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "texcoord", NULL, NULL, MXML_DESCEND))
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy3.texcoords[i] = float_swap_bytes(atof(tempCoord));
				i++;
			}
			fwrite(&wndy3, sizeof(brlyt_wnd3), 1, fp);
			*fileOffset = *fileOffset + sizeof(brlyt_wnd3);
		}

		brlyt_wnd4 wndy4;
		for(subnode = mxmlFindElement(node, node, "wnd4", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "wnd4", NULL, NULL, MXML_DESCEND))
		{
			i=0;
			mxml_node_t *subsubnode;
			subsubnode = mxmlFindElement(subnode, subnode, "offset", NULL, NULL, MXML_DESCEND);
			if (subsubnode != NULL)
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy4.offset = be32(strtoul(tempCoord, NULL, 16));
				i++;
			}
			fwrite(&wndy4, sizeof(brlyt_wnd4), 1, fp);
			*fileOffset = *fileOffset + sizeof(brlyt_wnd4);
		}
		brlyt_wnd4_mat wndy4mat;
		for(subnode = mxmlFindElement(node, node, "wnd4mat", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "wnd4mat", NULL, NULL, MXML_DESCEND))
		{
			i=0;
			mxml_node_t *subsubnode;
			subsubnode = mxmlFindElement(subnode, subnode, "material", NULL, NULL, MXML_DESCEND);
			if (subnode != NULL)
			{
				char temp[256];
				if(mxmlElementGetAttr(subsubnode, "name") != NULL)
					strcpy(temp, mxmlElementGetAttr(subsubnode, "name"));
				else{
					printf("No name attribute found!\nQuitting!\n");
					exit(1);
				}
				wndy4mat.material = short_swap_bytes(findMatOffset(temp));
			}
			subsubnode = mxmlFindElement(subnode, subnode, "index", NULL, NULL, MXML_DESCEND);
			if (subsubnode != NULL)
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy4mat.index = strtoul(tempCoord, NULL, 16);
			}
			subsubnode = mxmlFindElement(subnode, subnode, "padding", NULL, NULL, MXML_DESCEND);
			if (subsubnode != NULL)
			{
				char tempCoord[256];
				get_value(subsubnode, tempCoord, 256);
				wndy4mat.padding = strtoul(tempCoord, NULL, 16);
			}
			fwrite(&wndy4mat, sizeof(brlyt_wnd4_mat), 1, fp);
			*fileOffset = *fileOffset + sizeof(brlyt_wnd4_mat);
		}
	}
	if ( memcmp(temp, txt1, sizeof(txt1)) == 0)
	{
		WritePane( tree, node, tagblob, blobsize, fp, fileOffset );

		brlyt_text_chunk chunk2;
		mxml_node_t * subnode;
		subnode = mxmlFindElement(node, node, "material", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char temp[256];
			if(mxmlElementGetAttr(subnode, "name") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "name"));
			else{
				printf("No name attribute found!\nQuitting!\n");
				exit(1);
			}
			chunk2.mat_num = short_swap_bytes(findMatOffset(temp));
		}
		subnode = mxmlFindElement(node, node, "font", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char temp[256];
			if(mxmlElementGetAttr(subnode, "index") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "index"));
			else{
				printf("No index attribute found!\nQuitting!\n");
				exit(1);
			}
			chunk2.font_idx = atoi(temp);
			chunk2.font_idx = short_swap_bytes(chunk2.font_idx);

			mxml_node_t *valnode;
			valnode = mxmlFindElement(subnode, subnode, "xsize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.font_size_x = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "ysize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.font_size_y = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "charsize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.char_space = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "linesize", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.line_space = float_swap_bytes(atof(tempCoord));
			}
			valnode = mxmlFindElement(subnode, subnode, "alignment", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				int i;
				char originL , originH;
				char temp[256];
				memset( temp , 0 , 256 );
				if(mxmlElementGetAttr(valnode, "x") != NULL)
					strcpy(temp, mxmlElementGetAttr(valnode, "x"));
				else{
					printf("No x attribute found!\nQuitting!\n");
					exit(1);
				}
				for ( i = 0 ; i < 3 ; i++ ) {
					if ( strncmp( originX[i] , temp , 15 ) == 0 ) {
						originL = i;
						break;
					}
				}
				memset( temp , 0 , 256 );
				if(mxmlElementGetAttr(valnode, "y") != NULL)
					strcpy(temp, mxmlElementGetAttr(valnode, "y"));
				else{
					printf("No y attribute found!\nQuitting!\n");
					exit(1);
				}
				for ( i = 0 ; i < 3 ; i++ ) {
					if ( strncmp( originY[i] , temp , 15 ) == 0 ) {
						originH = i;
						break;
					}
				}
				chunk2.alignment = ( originH * 3 ) + ( originL );
			}
			valnode = mxmlFindElement(subnode, subnode, "whatAmI", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempCoord[256];
				get_value(valnode, tempCoord, 256);
				chunk2.unk_char = strtoul(tempCoord, NULL,16);
			}
		}

		subnode = mxmlFindElement(node, node, "length", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char tempCoord[256];
			get_value(subnode, tempCoord, 256);
			chunk2.len1 = strtoul(tempCoord, NULL, 16);
			chunk2.len1 = short_swap_bytes(chunk2.len1);
			chunk2.len2 = strtoul(tempCoord+5, NULL, 16);
			chunk2.len2 = short_swap_bytes(chunk2.len2);
		}
		subnode = mxmlFindElement(node, node, "topcolor", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			u32 red, green, blue, alpha;
			char tempCoord[256];
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "r") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "r"));
			else {
				printf("No r attribute found!\nQuitting!\n");
				exit(1);
			}
			red = strtoul(tempCoord, NULL, 16);
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "g") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "g"));
			else {
				printf("No g attribute found!\nQuitting!\n");
				exit(1);
			}
			green = strtoul(tempCoord, NULL, 16);
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "b") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "b"));
			else {
				printf("No b attribute found!\nQuitting!\n");
				exit(1);
			}
			blue = strtoul(tempCoord, NULL, 16);
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "a") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "a"));
			else {
				printf("No a attribute found!\nQuitting!\n");
				exit(1);
			}
			alpha = strtoul(tempCoord, NULL, 16);
			chunk2.color1 = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
			chunk2.color1 = be32( chunk2.color1 );
		}
		subnode = mxmlFindElement(node, node, "bottomcolor", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			u32 red, green, blue, alpha;
			char tempCoord[256];
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "r") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "r"));
			else {
				printf("No r attribute found!\nQuitting!\n");
				exit(1);
			}
			red = strtoul(tempCoord, NULL, 16);
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "g") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "g"));
			else {
				printf("No g attribute found!\nQuitting!\n");
				exit(1);
			}
			green = strtoul(tempCoord, NULL, 16);
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "b") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "b"));
			else {
				printf("No b attribute found!\nQuitting!\n");
				exit(1);
			}
			blue = strtoul(tempCoord, NULL, 16);
			memset( tempCoord , 0 , 256 );
			if ( mxmlElementGetAttr(subnode, "a") != NULL)
				strcpy(tempCoord, mxmlElementGetAttr(subnode, "a"));
			else {
				printf("No a attribute found!\nQuitting!\n");
				exit(1);
			}
			alpha = strtoul(tempCoord, NULL, 16);
			chunk2.color2 = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
			chunk2.color2 = be32( chunk2.color2 );
		}
		u16 textyLength = short_swap_bytes(chunk2.len2);
		if ((textyLength % 4) != 0)
			textyLength += (4 - (textyLength % 4));
		u8 texty[textyLength + 1];
		u8 tempy[textyLength*2];
		subnode = mxmlFindElement(node, node, "text", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			u8 tempCoord[8184];
			get_value(subnode, tempCoord, 8184);
			memcpy(tempy, tempCoord, short_swap_bytes(chunk2.len2)*2);
			u32 w; for (w=0;w<short_swap_bytes(chunk2.len2);w++)
			{
				u8 temp[3];
				temp[0] = tempy[w*2+0];
				temp[1] = tempy[w*2+1];
				temp[2] = 0x0;
				//printf("w: %d, temp: %s\n", w, temp);
				texty[w] = (u8)strtoul((char*)temp, NULL, 16);
			}
		}

		chunk2.name_offs=be32(0x74);
		chunk2.pad[0]= 0;chunk2.pad[1]=0;
		fwrite(&chunk2, sizeof(chunk2), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk2);

	int q;for (q=short_swap_bytes(chunk2.len2);q<textyLength;q++)
		{
			q++;
			texty[q] = 0x00;
		}
		fwrite(texty, textyLength, 1, fp);
		*fileOffset = *fileOffset + textyLength;
	}
	if ( memcmp(temp, pic1, sizeof(pic1)) == 0)
	{
		WritePane( tree, node, tagblob, blobsize, fp, fileOffset );

		brlyt_pic_chunk chunk2;
		mxml_node_t * subnode;
		subnode = mxmlFindElement(node, node, "material", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			char temp[256];
			if(mxmlElementGetAttr(subnode, "name") != NULL)
				strcpy(temp, mxmlElementGetAttr(subnode, "name"));
			else{
				printf("No name attribute found!\nQuitting!\n");
				exit(1);
			}
			chunk2.mat_num = short_swap_bytes(findMatOffset(temp));
		}
		subnode = mxmlFindElement(node, node, "colors", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			int i=0;
			valnode = mxmlFindElement(subnode, subnode, "vtxColorTL", NULL, NULL, MXML_DESCEND);
			if ( valnode != NULL )
			{
				u32 red, green, blue, alpha;
				char tempCoord[256];
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "r") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
				else {
					printf("No r attribute found!\nQuitting!\n");
					exit(1);
				}
				red = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "g") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
				else {
					printf("No g attribute found!\nQuitting!\n");
					exit(1);
				}
				green = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "b") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
				else {
					printf("No b attribute found!\nQuitting!\n");
					exit(1);
				}
				blue = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "a") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
				else {
					printf("No a attribute found!\nQuitting!\n");
					exit(1);
				}
				alpha = strtoul(tempCoord, NULL, 16);
				chunk2.vtx_colors[i] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
				chunk2.vtx_colors[i] = int_swap_bytes( chunk2.vtx_colors[i] );
				i++;
			}
			valnode = mxmlFindElement(subnode, subnode, "vtxColorTR", NULL, NULL, MXML_DESCEND);
			if ( valnode != NULL )
			{
				u32 red, green, blue, alpha;
				char tempCoord[256];
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "r") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
				else {
					printf("No r attribute found!\nQuitting!\n");
					exit(1);
				}
				red = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "g") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
				else {
					printf("No g attribute found!\nQuitting!\n");
					exit(1);
				}
				green = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "b") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
				else {
					printf("No b attribute found!\nQuitting!\n");
					exit(1);
				}
				blue = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "a") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
				else {
					printf("No a attribute found!\nQuitting!\n");
					exit(1);
				}
				alpha = strtoul(tempCoord, NULL, 16);
				chunk2.vtx_colors[i] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
				chunk2.vtx_colors[i] = int_swap_bytes( chunk2.vtx_colors[i] );
				i++;
			}
			valnode = mxmlFindElement(subnode, subnode, "vtxColorBL", NULL, NULL, MXML_DESCEND);
			if ( valnode != NULL )
			{
				u32 red, green, blue, alpha;
				char tempCoord[256];
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "r") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
				else {
					printf("No r attribute found!\nQuitting!\n");
					exit(1);
				}
				red = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "g") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
				else {
					printf("No g attribute found!\nQuitting!\n");
					exit(1);
				}
				green = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "b") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
				else {
					printf("No b attribute found!\nQuitting!\n");
					exit(1);
				}
				blue = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "a") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
				else {
					printf("No a attribute found!\nQuitting!\n");
					exit(1);
				}
				alpha = strtoul(tempCoord, NULL, 16);
				chunk2.vtx_colors[i] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
				chunk2.vtx_colors[i] = int_swap_bytes( chunk2.vtx_colors[i] );
				i++;
			}
			valnode = mxmlFindElement(subnode, subnode, "vtxColorBR", NULL, NULL, MXML_DESCEND);
			if ( valnode != NULL )
			{
				u32 red, green, blue, alpha;
				char tempCoord[256];
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "r") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "r"));
				else {
					printf("No r attribute found!\nQuitting!\n");
					exit(1);
				}
				red = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "g") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "g"));
				else {
					printf("No g attribute found!\nQuitting!\n");
					exit(1);
				}
				green = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "b") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "b"));
				else {
					printf("No b attribute found!\nQuitting!\n");
					exit(1);
				}
				blue = strtoul(tempCoord, NULL, 16);
				memset( tempCoord , 0 , 256 );
				if ( mxmlElementGetAttr(valnode, "a") != NULL)
					strcpy(tempCoord, mxmlElementGetAttr(valnode, "a"));
				else {
					printf("No a attribute found!\nQuitting!\n");
					exit(1);
				}
				alpha = strtoul(tempCoord, NULL, 16);
				chunk2.vtx_colors[i] = ( red << 24 ) | ( green << 16 ) | ( blue << 8 ) | alpha;
				chunk2.vtx_colors[i] = int_swap_bytes( chunk2.vtx_colors[i] );
				i++;
			}
		}
		u32 numberOfPicCoords = 0;
		u32 sets = 1;
        float* picCoords = GC_MALLOC(8 * sizeof(float));
		subnode = mxmlFindElement(node, node, "coordinates", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *setnode;
			for (setnode = mxmlFindElement(subnode, subnode, "set", NULL, NULL, MXML_DESCEND); setnode != NULL; setnode = mxmlFindElement(setnode, subnode, "set", NULL, NULL, MXML_DESCEND))
			{
                picCoords = GC_REALLOC(picCoords, sizeof(float) * 8 * sets);
				mxml_node_t *valnode;
				valnode=mxmlFindElement(setnode, setnode, "coordTL", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					float tempCoordGotten , coordGotten;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "s" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;

					strcpy( tempCoord , mxmlElementGetAttr( valnode , "t" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;
				}
				valnode=mxmlFindElement(setnode, setnode, "coordTR", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					float tempCoordGotten , coordGotten;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "s" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;

					strcpy( tempCoord , mxmlElementGetAttr( valnode , "t" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;
				}
				valnode=mxmlFindElement(setnode, setnode, "coordBL", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					float tempCoordGotten , coordGotten;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "s" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;

					strcpy( tempCoord , mxmlElementGetAttr( valnode , "t" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;
				}
				valnode=mxmlFindElement(setnode, setnode, "coordBR", NULL, NULL, MXML_DESCEND);
				if ( valnode != NULL )
				{
					float tempCoordGotten , coordGotten;
					char tempCoord[256];

					memset( tempCoord , 0 , 256 );
					strcpy( tempCoord , mxmlElementGetAttr( valnode , "s" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;

					strcpy( tempCoord , mxmlElementGetAttr( valnode , "t" ) );
					tempCoordGotten = atof(tempCoord);
					coordGotten = float_swap_bytes(tempCoordGotten);
					memcpy(&picCoords[numberOfPicCoords], &coordGotten, sizeof(float));
					numberOfPicCoords++;
				}
				sets++;
			}
		}
		chunk2.num_texcoords = (sets - 1);
		chunk2.padding = 0;
		fwrite(&chunk2, sizeof(chunk2), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk2);
		fwrite(picCoords, numberOfPicCoords * sizeof(float), 1, fp);
		*fileOffset = *fileOffset + (sizeof(float) * numberOfPicCoords);
	}
	if ( memcmp(temp, usd1, sizeof(usd1)) == 0)
	{
		// DO SOMETHING COOL
		brlyt_usdstart_chunk chunk;
		chunk.string_count = 0;
		chunk.unk2 = 0;		// offset = 0

		brlyt_usdmain_chunk chunk2;
		chunk2.string_offset = 0x10;
		chunk2.is_working_offset = 0xC;
		chunk2.unk5 = 0;
		chunk2.unk6 = 0;
		chunk2.unk7 = 0;

		mxml_node_t* subnode;
		for (subnode = mxmlFindElement(node, node, "usdentry", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "usdentry", NULL, NULL, MXML_DESCEND))
		{
			chunk.string_count++;
		}
		chunk.string_count = short_swap_bytes(chunk.string_count);
		fwrite(&chunk, sizeof(brlyt_usdstart_chunk), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk);

        u32 * working = GC_MALLOC(sizeof(u32) * short_swap_bytes(chunk.string_count));
        char * stringg = GC_MALLOC(sizeof(char) * 8 * short_swap_bytes(chunk.string_count));
		memset(stringg, 0, sizeof(char) * 8 * short_swap_bytes(chunk.string_count));
		char * stringg_temp = stringg;
		u32 index = 0;

		u32 big_offs = (0xC * short_swap_bytes(chunk.string_count)) + (0x4 * short_swap_bytes(chunk.string_count));
		u32 little_offs = (0xC * short_swap_bytes(chunk.string_count));

		for (subnode = mxmlFindElement(node, node, "usdentry", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "usdentry", NULL, NULL, MXML_DESCEND))
		{
			mxml_node_t *valnode = mxmlFindElement(subnode , subnode , "unk5", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				get_value(valnode, tempChar, 256);
				chunk2.unk5 = strtoul(tempChar, NULL, 16);
				chunk2.unk5 = short_swap_bytes(chunk2.unk5);
			}
			valnode = mxmlFindElement(subnode , subnode , "unk6", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				get_value(valnode, tempChar, 256);
				chunk2.unk6 = strtoul(tempChar, NULL, 16);
			}
			valnode = mxmlFindElement(subnode , subnode , "unk7", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				get_value(valnode, tempChar, 256);
				chunk2.unk7 = strtoul(tempChar, NULL, 16);
			}
			valnode = mxmlFindElement(subnode , subnode , "isWorking", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				get_value(valnode, tempChar, 256);
				working[index] = strtoul(tempChar, NULL, 16);
				working[index] = be32(working[index]);
			}
			valnode = mxmlFindElement(subnode , subnode , "string", NULL, NULL, MXML_DESCEND);
			if (valnode != NULL)
			{
				char tempChar[256];
				memset(tempChar, 0, 256);
				get_value(valnode, tempChar, 256);
				memcpy(stringg_temp, tempChar, 8);
			}
			chunk2.string_offset = be32(big_offs);
			chunk2.is_working_offset = be32(little_offs);
			fwrite(&chunk2, sizeof(brlyt_usdmain_chunk),1,fp);
			*fileOffset = *fileOffset + sizeof(chunk2);
			big_offs -= 0x4; // -0xC +0x8
			little_offs -=8; // -0xC +0x4
			index++;
			stringg_temp+=8;
		}
		fwrite(working, sizeof(u32) * short_swap_bytes(chunk.string_count), 1, fp);
		*fileOffset = *fileOffset + (sizeof(u32) * short_swap_bytes(chunk.string_count));
		fwrite(stringg, sizeof(char) * 8 * short_swap_bytes(chunk.string_count), 1, fp);
		*fileOffset = *fileOffset + (sizeof(char) * 8 * short_swap_bytes(chunk.string_count));
	}
	if ( memcmp(temp, grp1, sizeof(grp1)) == 0)
	{
		brlyt_group_chunk chunk;
		chunk.unk = 0;
		char temp[256];
		if(mxmlElementGetAttr(node, "name") != NULL)
			strcpy(temp, mxmlElementGetAttr(node, "name"));
		else{
			printf("No name attribute found!\nQuitting!\n");
			exit(1);
		}
		memset(chunk.name, 0, 16 * sizeof(char));
		strcpy(chunk.name, temp);
		int numSubs = 0;
		char *subs;
    subs = GC_MALLOC(0 * sizeof(char));
		u32 subsLength = 0;

		mxml_node_t *subnode = mxmlFindElement(node, node, "subs", NULL, NULL, MXML_DESCEND);
		if (subnode != NULL)
		{
			mxml_node_t *valnode;
			for(valnode = mxmlFindElement(subnode, subnode, "sub", NULL, NULL, MXML_DESCEND); valnode != NULL; valnode = mxmlFindElement(valnode, subnode, "sub", NULL, NULL, MXML_DESCEND)) {
			if (valnode != NULL)
			{
					u32 oldSubsLength = subsLength;
					char tempSub[256];
					get_value(valnode, tempSub, 256);
					subsLength += 16;
                    subs = GC_REALLOC(subs, 1 + sizeof(char) * subsLength);
					memset(&subs[oldSubsLength], 0x00, 16);
					strcpy(&subs[oldSubsLength], tempSub);
					numSubs++;
				}
			}
		}
		chunk.numsubs = short_swap_bytes(numSubs);
		fwrite(&chunk, sizeof(chunk), 1, fp);
		*fileOffset = *fileOffset + sizeof(chunk);
		fwrite(subs, sizeof(char) * subsLength, 1, fp);
        *fileOffset = *fileOffset + subsLength;
	}
	if ( memcmp(temp, grs1, sizeof(grs1)) == 0)
	{

	}
	if ( memcmp(temp, gre1, sizeof(gre1)) == 0)
	{

	}
	if ( memcmp(temp, pas1, sizeof(pas1)) == 0)
	{

	}
	if ( memcmp(temp, pae1, sizeof(pae1)) == 0)
	{

	}

	*blobsize = *fileOffset - startOfChunk;
}

void WriteBRLYTHeader(brlyt_header rlythead, FILE* fp)
{
	brlyt_header writehead;
	writehead.magic[0] = rlythead.magic[0];
	writehead.magic[1] = rlythead.magic[1];
	writehead.magic[2] = rlythead.magic[2];
	writehead.magic[3] = rlythead.magic[3];
	writehead.endian = short_swap_bytes(rlythead.endian);
	writehead.version = short_swap_bytes(rlythead.version);
	writehead.filesize = be32(rlythead.filesize);
	writehead.lyt_offset = short_swap_bytes(rlythead.lyt_offset);
	writehead.sections = short_swap_bytes(rlythead.sections);

	fwrite(&writehead, sizeof(brlyt_header), 1, fp);
}

void write_brlyt(char *infile, char *outfile)
{
	SetupConstants();

	u32 fileOffset = 0;
	FILE* fpx = fopen(infile, "r");
	if(fpx == NULL) {
		printf("xmlyt couldn't be opened!\n");
		exit(1);
	}
	mxml_node_t *hightree = mxmlLoadFile(NULL, fpx, MXML_TEXT_CALLBACK);
	if(hightree == NULL) {
		printf("Couldn't open hightree!\n");
		exit(1);
	}
	mxml_node_t *tree = mxmlFindElement(hightree, hightree, "xmlyt", NULL, NULL, MXML_DESCEND);
	if(tree == NULL) {
		printf("Couldn't get tree!\n");
		exit(1);
	}
	char tempVersion[9];
	if(mxmlElementGetAttr(tree, "brlyt_version") != NULL)
		strcpy(tempVersion, mxmlElementGetAttr(tree, "brlyt_version"));
	else{
		printf("No brlyt_version attribute found!\n");
		exit(1);
	}
	u32 versionn = strtoul(tempVersion, NULL, 16);

	mxml_node_t *node;
	FILE* fp = fopen(outfile, "wb+");
	if(fp == NULL) {
		printf("destination brlyt couldn't be opened!\n");
		exit(1);
	}

	u8* tagblob;
	u32 blobsize;
	u16 blobcount = 0;
	brlyt_header rlythead;
	rlythead.magic[0] = 'R';
	rlythead.magic[1] = 'L';
	rlythead.magic[2] = 'Y';
	rlythead.magic[3] = 'T';
	//rlythead.version = 0xFEFF0008;
	rlythead.endian = 0xFEFF;
	rlythead.version = versionn;
	rlythead.filesize = 0;
	rlythead.lyt_offset = sizeof(brlyt_header);
	rlythead.sections = 1;
	WriteBRLYTHeader(rlythead, fp);

	fileOffset += sizeof(brlyt_header);

	u32 numberOfEntries;

	for(node = mxmlFindElement(tree, tree, "tag", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, tree, "tag", NULL, NULL, MXML_DESCEND)) {

		char tempType[5] = {0};
		if(mxmlElementGetAttr(node, "type") != NULL)
			strncpy(tempType, mxmlElementGetAttr(node, "type"), 5);
		else{
			printf("No type attribute found!\n");
			exit(1);
		}
		//fputs(tempType, stdout);

		u32 chunkHeaderOffset = fileOffset;

		brlyt_entry_header chunk;
		chunk.magic[0]=tempType[0];
		chunk.magic[1]=tempType[1];
		chunk.magic[2]=tempType[2];
		chunk.magic[3]=tempType[3];
		chunk.length = 0;

		fwrite(&chunk, sizeof(chunk), 1, fp);

		fileOffset += sizeof(chunk);

		WriteBRLYTEntry(tree, node, &tagblob, &blobsize, tempType, fp, &fileOffset);

		chunk.length = be32(blobsize + sizeof(chunk));

		fseek(fp, chunkHeaderOffset, SEEK_SET);
		fwrite(&chunk, sizeof(chunk), 1, fp);
		fseek(fp, fileOffset, SEEK_SET);

		numberOfEntries++;

		blobcount++;

	}

	rlythead.filesize = fileOffset;
	rlythead.sections = blobcount;
	fseek(fp, 0, SEEK_SET);
	WriteBRLYTHeader(rlythead, fp);

	fclose(fpx);
	fclose(fp);
}

void make_brlyt(char* infile, char* outfile)
{
	printf("\x1b[33mParsing XMLYT @ \x1b[0m%s.\n", infile);
	write_brlyt(infile, outfile);
	printf("\x1b[34mParsing XMLYT @ \x1b[0m%s complete.\n", infile);
}

