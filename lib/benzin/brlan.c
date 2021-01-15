/******************************************************************************
 *  brlan.c                                                                   *
 *  Part of Benzin                                                            *
 *  Handles BRLAN banner datas.                                               *
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
#include <ctype.h>
#include <mxml.h>

#include "general.h"
#include "memfile.h"
#include "types.h"
#include "brlan.h"
#include "xml.h"
#include "endian.h"

#if DEBUGBRLAN == 1
#define dbgprintf	printf
#else
#define dbgprintf	//
#endif //DEBUGBRLAN

#define MAXIMUM_TAGS_SIZE		(0xf000)
#define MAXIMUM_TIMGS_SIZE		(0x1000)

fourcc tag_FourCCs[] = { "RLPA", "RLTS", "RLVI", "RLVC", "RLMC", "RLTP" , "RLIM" };

char tag_types_list[16][24];
char tag_types_rlpa_list[16][24];
char tag_types_rlts_list[16][24];
char tag_types_rlvi_list[16][24];
char tag_types_rlvc_list[16][24];
char tag_types_rlmc_list[16][24];
char tag_types_rltp_list[16][24];
char tag_types_rlim_list[16][24];

static size_t BRLAN_fileoffset = 0;
FILE* xmlanout;

void CreateTagTypesLists( )
{
	int i;
	for(i = 0; i < 16; i++)
	{
		memset(tag_types_list[i], 0, 24);
		memset(tag_types_rlpa_list[i], 0, 24);
		memset(tag_types_rlvc_list[i], 0, 24);
		memset(tag_types_rltp_list[i], 0, 24);
	}
	for(i = 0; i < 5; i++)
	{
		memset(tag_types_rlts_list[i], 0, 24);
		memset(tag_types_rlim_list[i], 0, 24);
	}
	for(i = 0; i < 16; i++)
		memset(tag_types_rlmc_list[i], 0, 24);
	memset(tag_types_rlvi_list[i], 0, 24);

	strcpy(tag_types_list[0],  "X Translation");
	strcpy(tag_types_list[1],  "Y Translation");
	strcpy(tag_types_list[2],  "Z Translation");
	strcpy(tag_types_list[3],  "X Rotate");
	strcpy(tag_types_list[4],  "Y Rotate");
	strcpy(tag_types_list[5],  "Z Rotate");
	strcpy(tag_types_list[6],  "X Scale");
	strcpy(tag_types_list[7],  "Y Scale");
	strcpy(tag_types_list[8],  "Width");
	strcpy(tag_types_list[9],  "Height");
	strcpy(tag_types_list[10], "0x0A");				// modified x
	strcpy(tag_types_list[11], "0x0B");				// modified y
	strcpy(tag_types_list[12], "0x0C");				// modified z
	strcpy(tag_types_list[13], "0x0D");
	strcpy(tag_types_list[14], "0x0E");
	strcpy(tag_types_list[15], "0x0F");

	strcpy(tag_types_rlpa_list[0],  "X Translation");
	strcpy(tag_types_rlpa_list[1],  "Y Translation");
	strcpy(tag_types_rlpa_list[2],  "Z Translation");
	strcpy(tag_types_rlpa_list[3],  "X Rotate");
	strcpy(tag_types_rlpa_list[4],  "Y Rotate");
	strcpy(tag_types_rlpa_list[5],  "Z Rotate");
	strcpy(tag_types_rlpa_list[6],  "X Scale");
	strcpy(tag_types_rlpa_list[7],  "Y Scale");
	strcpy(tag_types_rlpa_list[8],  "Width");
	strcpy(tag_types_rlpa_list[9],  "Height");
	strcpy(tag_types_rlpa_list[10], "0x0A");				// modified x
	strcpy(tag_types_rlpa_list[11], "0x0B");				// modified y
	strcpy(tag_types_rlpa_list[12], "0x0C");				// modified z
	strcpy(tag_types_rlpa_list[13], "0x0D");
	strcpy(tag_types_rlpa_list[14], "0x0E");
	strcpy(tag_types_rlpa_list[15], "0x0F");

	strcpy(tag_types_rlts_list[0], "XTrans");
	strcpy(tag_types_rlts_list[1], "YTrans");
	strcpy(tag_types_rlts_list[2], "Rotate");
	strcpy(tag_types_rlts_list[3], "XScale");
	strcpy(tag_types_rlts_list[4], "YScale");

	strcpy(tag_types_rlvi_list[0], "Visibility");

	strcpy(tag_types_rlvc_list[0],  "Top Left R");
	strcpy(tag_types_rlvc_list[1],  "Top Left G");
	strcpy(tag_types_rlvc_list[2],  "Top Left B");
	strcpy(tag_types_rlvc_list[3],  "Top Left A");
	strcpy(tag_types_rlvc_list[4],  "Top Right R");
	strcpy(tag_types_rlvc_list[5],  "Top Right G");
	strcpy(tag_types_rlvc_list[6],  "Top Right B");
	strcpy(tag_types_rlvc_list[7],  "Top Right A");
	strcpy(tag_types_rlvc_list[8],  "Bottom Left R");
	strcpy(tag_types_rlvc_list[9],  "Bottom Left G");
	strcpy(tag_types_rlvc_list[10], "Bottom Left B");
	strcpy(tag_types_rlvc_list[11], "Bottom Left A");
	strcpy(tag_types_rlvc_list[12], "Bottom Right R");
	strcpy(tag_types_rlvc_list[13], "Bottom Right G");
	strcpy(tag_types_rlvc_list[14], "Bottom Right B");
	strcpy(tag_types_rlvc_list[15], "Bottom Right A");

	strcpy(tag_types_rlmc_list[0],  "MatColor Red");
	strcpy(tag_types_rlmc_list[1],  "MatColor Green");
	strcpy(tag_types_rlmc_list[2],  "MatColor Blue");
	strcpy(tag_types_rlmc_list[3],  "MatColor Alpha");
	strcpy(tag_types_rlmc_list[4],  "Forecolor Red");
	strcpy(tag_types_rlmc_list[5],  "Forecolor Green");
	strcpy(tag_types_rlmc_list[6],  "Forecolor Blue");
	strcpy(tag_types_rlmc_list[7],  "Forecolor Alpha");
	strcpy(tag_types_rlmc_list[8],  "Backcolor Red");
	strcpy(tag_types_rlmc_list[9],  "Backcolor Green");
	strcpy(tag_types_rlmc_list[10], "Backcolor Blue");
	strcpy(tag_types_rlmc_list[11], "Backcolor Alpha");
	strcpy(tag_types_rlmc_list[12], "TevReg3 Red");
	strcpy(tag_types_rlmc_list[13], "TevReg3 Green");
	strcpy(tag_types_rlmc_list[14], "TevReg3 Blue");
	strcpy(tag_types_rlmc_list[15], "TevReg3 Alpha");

	strcpy(tag_types_rltp_list[0],  "PaletteZero");
	strcpy(tag_types_rltp_list[1],  "PaletteOne");
	strcpy(tag_types_rltp_list[2],  "PaletteTwo");
	strcpy(tag_types_rltp_list[3],  "PaletteThree");
	strcpy(tag_types_rltp_list[4],  "PaletteFour");
	strcpy(tag_types_rltp_list[5],  "PaletteFive");
	strcpy(tag_types_rltp_list[6],  "PaletteSix");
	strcpy(tag_types_rltp_list[7],  "PaletteSeven");
	strcpy(tag_types_rltp_list[8],  "PaletteEight");
	strcpy(tag_types_rltp_list[9],  "PaletteNine");
	strcpy(tag_types_rltp_list[10], "PaletteTen");
	strcpy(tag_types_rltp_list[11], "PaletteEleven");
	strcpy(tag_types_rltp_list[12], "PaletteTwelve");
	strcpy(tag_types_rltp_list[13], "PaletteThirteen");
	strcpy(tag_types_rltp_list[14], "PaletteFourteen");
	strcpy(tag_types_rltp_list[15], "PaletteFifteen");

	strcpy(tag_types_rlim_list[0], "XTrans");
	strcpy(tag_types_rlim_list[1], "YTrans");
	strcpy(tag_types_rlim_list[2], "Rotate");
	strcpy(tag_types_rlim_list[3], "XScale");
	strcpy(tag_types_rlim_list[4], "YScale");
}

static void BRLAN_ReadDataFromMemoryX(void* destination, void* input, size_t size)
{
	u8* out = (u8*)destination;
	u8* in = ((u8*)input) + BRLAN_fileoffset;
	memcpy(out, in, size);
}

static void BRLAN_ReadDataFromMemory(void* destination, void* input, size_t size)
{
	BRLAN_ReadDataFromMemoryX(destination, input, size);
	BRLAN_fileoffset += size;
}

static void CreateGlobal_pai1(brlan_pai1_header_type2 *pai1_header, brlan_pai1_header_type1 pai1_header1, brlan_pai1_header_type2 pai1_header2, int pai1_header_type)
{
	if(pai1_header_type == 1) {
		pai1_header->magic[0]		= pai1_header1.magic[0];
		pai1_header->magic[1]		= pai1_header1.magic[1];
		pai1_header->magic[2]		= pai1_header1.magic[2];
		pai1_header->magic[3]		= pai1_header1.magic[3];
		pai1_header->size			= pai1_header1.size;
		pai1_header->framesize		= pai1_header1.framesize;
		pai1_header->flags			= pai1_header1.flags;
		pai1_header->padding		= pai1_header1.padding;
		pai1_header->num_timgs		= pai1_header1.num_timgs;
		pai1_header->num_entries	= pai1_header1.num_entries;
		pai1_header->padding2		= 0;
		pai1_header->entry_offset	= pai1_header1.entry_offset;
	}else
		memcpy(pai1_header, &pai1_header2, sizeof(brlan_pai1_header_type2));
}

static int FourCCsMatch(fourcc cc1, fourcc cc2)
{
	if((cc1[0] == cc2[0]) && (cc1[1] == cc2[1]) && (cc1[2] == cc2[2]) && (cc1[3] == cc2[3]))
		return 1;
	else
		return 0;
}

static int FourCCInList(fourcc cc)
{
	int i;
	for(i = 0; i < 7; i++)
		if(FourCCsMatch(cc, tag_FourCCs[i])) return 1;
	return 0;
}

void BRLAN_CreateXMLTag(tag_header tagHeader, void* data, u32 offset, mxml_node_t *pane)
{
	BRLAN_fileoffset = offset;
	BRLAN_ReadDataFromMemory(&tagHeader, data, sizeof(tag_header));

	mxml_node_t *tag;
	tag = mxmlNewElement(pane, "tag"); mxmlElementSetAttrf(tag, "type", "%c%c%c%c", tagHeader.magic[0], tagHeader.magic[1], tagHeader.magic[2], tagHeader.magic[3]);

	tag_entryinfo tagEntryInfo;
	u32 tagentrylocations[tagHeader.entry_count];
	BRLAN_ReadDataFromMemory(&tagentrylocations, data, tagHeader.entry_count * sizeof(u32));
	u32 i, j;
	for ( i = 0; i < tagHeader.entry_count; i++)
	{
		mxml_node_t *entry;
		BRLAN_fileoffset = offset + be32(tagentrylocations[i]);
		BRLAN_ReadDataFromMemory(&tagEntryInfo, data, sizeof(tag_entryinfo));
		entry = mxmlNewElement(tag, "entry");
		mxmlElementSetAttrf(entry, "type1", "%u", tagEntryInfo.type1);
		char type_rlpa[4] = {'R', 'L', 'P', 'A'};
		char type_rlts[4] = {'R', 'L', 'T', 'S'};
		char type_rlvi[4] = {'R', 'L', 'V', 'I'};
		char type_rlvc[4] = {'R', 'L', 'V', 'C'};
		char type_rlmc[4] = {'R', 'L', 'M', 'C'};
		char type_rltp[4] = {'R', 'L', 'T', 'P'};
		char type_rlim[4] = {'R', 'L', 'I', 'M'};

		if(memcmp(tagHeader.magic, type_rlpa, 4) == 0)
		{
			if(tagEntryInfo.type2 < 16)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rlpa_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else if (memcmp(tagHeader.magic, type_rlts, 4) == 0) {
			if(tagEntryInfo.type2 < 5)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rlts_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else if (memcmp(tagHeader.magic, type_rlvi, 4) == 0) {
			if(tagEntryInfo.type2 < 1)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rlvi_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else if (memcmp(tagHeader.magic, type_rlvc, 4) == 0) {
			if(tagEntryInfo.type2 < 16)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rlvc_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else if (memcmp(tagHeader.magic, type_rlmc, 4) == 0) {
			if(tagEntryInfo.type2 < 32)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rlmc_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else if (memcmp(tagHeader.magic, type_rltp, 4) == 0) {
			if(tagEntryInfo.type2 < 16)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rltp_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else if (memcmp(tagHeader.magic, type_rlim, 4) == 0) {
			if(tagEntryInfo.type2 < 5)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_rlim_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		} else {
			if(tagEntryInfo.type2 < 16)
			{
				mxmlElementSetAttrf(entry, "type2", "%s",tag_types_list[tagEntryInfo.type2]);
			} else {
				mxmlElementSetAttrf(entry, "type2", "%u", tagEntryInfo.type2);
			}
		}


		for( j = 0; j < short_swap_bytes(tagEntryInfo.coord_count); j++)
		{
			if ( tagEntryInfo.data_type == 0x2 )
			{
				mxml_node_t *triplet;
				mxml_node_t *frame;
				mxml_node_t *value;
				mxml_node_t *blend;
				tag_data tagData;
				BRLAN_ReadDataFromMemory(&tagData, data, sizeof(tag_data));

				u32 p1 = be32(tagData.part1);
				u32 p2 = be32(tagData.part2);
				u32 p3 = be32(tagData.part3);
				triplet = mxmlNewElement(entry, "triplet");
				frame = mxmlNewElement(triplet, "frame");
				mxmlNewTextf(frame, 0, "%.15f", *(f32*)(&p1));
				value = mxmlNewElement(triplet, "value");
				mxmlNewTextf(value, 0, "%.15f", *(f32*)(&p2));
				blend = mxmlNewElement(triplet, "blend");
				mxmlNewTextf(blend, 0, "%.15f", *(f32*)(&p3));
			} else {
				tag_data2 tagData2;
				BRLAN_ReadDataFromMemory(&tagData2, data, sizeof(tag_data2));

				mxml_node_t *pair;
				mxml_node_t *data1;
				mxml_node_t *data2;
				mxml_node_t *padding;
				u32 p1 = be32(tagData2.part1);
				u16 p2 = short_swap_bytes(tagData2.part2);
				u16 p3 = short_swap_bytes(tagData2.padding);
				pair = mxmlNewElement(entry, "pair");
				data1 = mxmlNewElement(pair, "data1");
				mxmlNewTextf(data1, 0, "%.15f", *(f32*)(&p1));
				data2 = mxmlNewElement(pair, "data2");
				mxmlNewTextf(data2, 0, "%04x", p2);
				padding = mxmlNewElement(pair, "padding");
				mxmlNewTextf(padding, 0, "%04x", p3);
			}
		}
	}
}

void BRLAN_CreatePat1Tag(brlan_pai1_universal pai1_header, void* data, u32 offset, mxml_node_t *pat1_tag)
{
}

void parse_brlan(char* filename, char *filenameout)
{
	int i, j;

	FILE* fp = fopen(filename, "rb");
	if(fp == NULL) {
		printf("Error! Couldn't open %s!\n", filename);
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	u32 lengthOfFile = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	u8 data[lengthOfFile];
	fread(data, lengthOfFile, 1, fp);

	CreateTagTypesLists();

	char pat1_tag[4] = {'p', 'a', 't', '1'};
	char pah1_tag[4] = {'p', 'a', 'h', '1'};
	char pai1_tag[4] = {'p', 'a', 'i', '1'};

	BRLAN_fileoffset = 0;
	brlan_header header;
	BRLAN_ReadDataFromMemory(&header, data, sizeof(brlan_header));
	BRLAN_fileoffset = short_swap_bytes(header.pai1_offset);

	FILE *xmlFile;
	xmlFile = fopen(filenameout, "w");
	mxml_node_t *xml;
	mxml_node_t *xmlan;
	mxmlSetWrapMargin(0);
	xml = mxmlNewXML("1.0");
	xmlan = mxmlNewElement(xml, "xmlan");
	mxmlElementSetAttrf(xmlan, "version", "%d.%d.%d%s", BENZIN_VERSION_MAJOR, BENZIN_VERSION_MINOR, BENZIN_VERSION_BUILD, BENZIN_VERSION_OTHER);
	mxmlElementSetAttrf(xmlan, "brlan_version", "%04x", short_swap_bytes(header.version) );

	u16 pa_counter = 0;
	for(pa_counter = 0; pa_counter < short_swap_bytes(header.pai1_count); pa_counter++)
	{
		dbgprintf("Loop\n");
		dbgprintf("%.4s\n", data+BRLAN_fileoffset);
		dbgprintf("length: %08x\n", be32(*(u32*)(data+BRLAN_fileoffset+4)));
		if (!memcmp(pat1_tag, data+BRLAN_fileoffset, 4)){
			u32 pat1_offset = BRLAN_fileoffset;
			brlan_pat1_universal pathead;
			BRLAN_ReadDataFromMemory(&pathead, data, sizeof(brlan_pat1_universal));

			mxml_node_t * pat_node;
			pat_node = mxmlNewElement(xmlan, "pat1");
			mxml_node_t *unk1, *unk2, *unk5, *unk6;
			unk1 = mxmlNewElement(pat_node, "unk1");
			mxmlNewTextf(unk1, 0, "%04x", short_swap_bytes(pathead.unk1));
			//unk2 = mxmlNewElement(pat_node, "unk2");
			//mxmlNewTextf(unk2, 0, "%04x", short_swap_bytes(pathead.unk2));
			unk5 = mxmlNewElement(pat_node, "unk5a");
			mxmlNewTextf(unk5, 0, "%04x", short_swap_bytes(pathead.unk5a));
			unk5 = mxmlNewElement(pat_node, "unk5b");
			mxmlNewTextf(unk5, 0, "%04x", short_swap_bytes(pathead.unk5b));
			// unk6 == bool lookAtChildren/isDecendingBind
			unk6 = mxmlNewElement(pat_node, "isDecendingBind");
			mxmlNewTextf(unk6, 0, "%02x", pathead.isDecendingBind);
			unk6 = mxmlNewElement(pat_node, "padding");
			mxmlNewTextf(unk6, 0, "%02x", pathead.padding);

			mxml_node_t *strngs1, *strngs2;

			u32 ofs = pat1_offset+ be32(pathead.unk3_offset);
			char str[0x40];
			memset(str, 0, 0x40);
			strncpy(str, data+ofs, 0x3f);
			strngs1 = mxmlNewElement(pat_node, "first");
			mxmlNewTextf(strngs1, 0, "%s", str);

			u32 offs = pat1_offset+ be32(pathead.group_array_offset);
			strngs2 = mxmlNewElement(pat_node, "seconds");
			u16 seconds = 0;
			for(seconds = 0; seconds < short_swap_bytes(pathead.group_num); seconds++)
			{
				mxml_node_t *info;
				char strng[0x14];
				memcpy(strng, data+offs, 0x14);
				info = mxmlNewElement(strngs2, "string");
				mxmlNewTextf(info, 0, "%s", strng);
				offs += 0x14;
			}

			BRLAN_fileoffset = pat1_offset+be32(pathead.size);

		}else if(!memcmp(pah1_tag, data+BRLAN_fileoffset, 4)){
			u32 pah1_offset = BRLAN_fileoffset;
			mxml_node_t * pah_node;
			pah_node = mxmlNewElement(xmlan, "pah1");
			BRLAN_fileoffset += be32(*(u32*)(data+BRLAN_fileoffset+4));
			//FIXME

		}else if(!memcmp(pai1_tag, data+BRLAN_fileoffset, 4)){
			u32 pai1_offset = BRLAN_fileoffset;
			dbgprintf("In pai1 tag\n");
			mxml_node_t * pai_node;
			pai_node = mxmlNewElement(xmlan, "pai1");

			brlan_pai1_universal universal;
			BRLAN_ReadDataFromMemoryX(&universal, data, sizeof(brlan_pai1_universal));

			int pai1_header_type;
			brlan_pai1_header_type1 pai1_header1;
			brlan_pai1_header_type2 pai1_header2;
			brlan_pai1_header_type2 pai1_header;

			if((be32(universal.flags) & (1 << 25)) >= 1) {
				pai1_header_type = 2;
				BRLAN_ReadDataFromMemory(&pai1_header2, data, sizeof(brlan_pai1_header_type2));
			} else {
				pai1_header_type = 1;
				BRLAN_ReadDataFromMemory(&pai1_header1, data, sizeof(brlan_pai1_header_type1));
			}

			CreateGlobal_pai1(&pai1_header, pai1_header1, pai1_header2, pai1_header_type);

			mxmlElementSetAttrf(pai_node, "framesize", "%lu", (long unsigned int)short_swap_bytes(pai1_header.framesize));
			mxmlElementSetAttrf(pai_node, "flags", "%02x", pai1_header.flags);

			int timgs = short_swap_bytes(pai1_header.num_timgs);

			BRLAN_fileoffset = pai1_offset + sizeof(brlan_pai1_header_type1);
			if ( pai1_header_type == 2 ) BRLAN_fileoffset += 4;
			int tableoff = BRLAN_fileoffset;
			int currtableoff = BRLAN_fileoffset;

			mxml_node_t *timg;
			for( i = 0; i < timgs; i++) {
				u32 curr_timg_off = 0;
				BRLAN_ReadDataFromMemory(&curr_timg_off, data, 4);
				char timgname[256];
				memset(timgname, 0, 256);
				int z = tableoff + be32(curr_timg_off);
				for( j = 0; data[z] != 0; timgname[j++] = data[z], z++);
				{
					timg = mxmlNewElement(pai_node, "timg");
					mxmlElementSetAttrf(timg, "name", "%s", timgname);
				}
				currtableoff += 4;
			}

			int tagcount = short_swap_bytes(pai1_header.num_entries);
			u32 taglocations[tagcount];
			BRLAN_fileoffset = pai1_offset + be32(pai1_header.entry_offset);
			BRLAN_ReadDataFromMemory(taglocations, data, tagcount * sizeof(u32));

			for( i = 0; i < tagcount; i++) {
				brlan_entry brlanEntry;
				tag_header tagHeader;
				BRLAN_fileoffset = be32(taglocations[i]) + pai1_offset;
				u32 brlanEntryOffset = BRLAN_fileoffset;
				BRLAN_ReadDataFromMemory(&brlanEntry, data, sizeof(brlan_entry));

				mxml_node_t *pane;
				pane = mxmlNewElement(pai_node, "pane");
				mxmlElementSetAttrf(pane, "name", "%s", brlanEntry.name);
				mxmlElementSetAttrf(pane, "type", "%u", brlanEntry.is_material);

				u32 entrylocations[brlanEntry.num_tags];
				BRLAN_ReadDataFromMemory(entrylocations, data, brlanEntry.num_tags * sizeof(u32));
				for ( j = 0; j < brlanEntry.num_tags; j++)
				{
					BRLAN_CreateXMLTag(tagHeader, data, brlanEntryOffset + be32(entrylocations[j]), pane);
				}
			}
		}
	}

	mxmlSaveFile(xml, xmlFile, whitespace_cb);
	mxmlDelete(xml);
	fclose(xmlFile);
	fclose(fp);
}

void WriteBRLANTagHeader(tag_header* head, FILE* fp)
{
	fwrite(head, sizeof(tag_header), 1, fp);
}

void WriteBRLANTagEntries(tag_entry* entry, u8 count, FILE* fp)
{
	tag_entry writeentry;
	int i;
	for(i = 0; i < count; i++) {
		writeentry.offset = be32(entry[i].offset);
		fwrite(&writeentry, sizeof(tag_entry), 1, fp);
	}
}

void WriteBRLANOffsets(u32* offsets, u32 number, FILE* fp)
{
	u32 tempoffsets[number];
	int i;
	for(i=0;i<number;i++) tempoffsets[i] = be32(offsets[i]);
	fwrite(tempoffsets, number * sizeof(u32), 1, fp);
}

void WriteBRLANTagEntryinfos(tag_entryinfo entryinfo, FILE* fp)
{
	tag_entryinfo writeentryinfo;
	writeentryinfo.type1 = entryinfo.type1;
	writeentryinfo.type2 = entryinfo.type2;
	writeentryinfo.data_type = short_swap_bytes(entryinfo.data_type);
	writeentryinfo.coord_count = short_swap_bytes(entryinfo.coord_count);
	writeentryinfo.pad1 = short_swap_bytes(entryinfo.pad1);
	writeentryinfo.offset = be32(entryinfo.offset);
	fwrite(&writeentryinfo, sizeof(tag_entryinfo), 1, fp);
}

void WriteBRLANTagData(tag_data* data, u16 count, FILE* fp)
{
	tag_data writedata;
	int i;
	for(i = 0; i < count; i++) {
		writedata.part1 = be32(data[i].part1);
		writedata.part2 = be32(data[i].part2);
		writedata.part3 = be32(data[i].part3);
		fwrite(&writedata, sizeof(tag_data), 1, fp);
	}
}

void WriteBRLANEntry(brlan_entry *entr, FILE* fp)
{
	brlan_entry writeentr;
	memset(writeentr.name, 0, 20);
	strncpy(writeentr.name, entr->name, 20);
	writeentr.num_tags = entr->num_tags;
	writeentr.is_material = entr->is_material;
	writeentr.pad[0] = 0x0;
	writeentr.pad[1] = 0x0;
	fwrite(&writeentr, sizeof(brlan_entry), 1, fp);
}

void create_entries_from_xml(mxml_node_t *pai1_node, mxml_node_t *node, brlan_entry *entr, tag_header* head, FILE* fp)
{
	tag_entry* entry = NULL;
	tag_entryinfo* entryinfo = NULL;
	tag_data** data = NULL;
	tag_data2** data2 = NULL;
	mxml_node_t *tempnode = NULL;
	mxml_node_t *subnode = NULL;
	mxml_node_t *subsubnode = NULL;
	char temp[256];
	char temp2[256];
	char temp3[32][24];
	int i, x;

	char tag_type[256];
	if(mxmlElementGetAttr(node, "type") != NULL)
		strcpy(tag_type, mxmlElementGetAttr(node, "type"));

	char rlpa_type[5] = {'R', 'L', 'P', 'A'};
	char rlts_type[5] = {'R', 'L', 'T', 'S'};
	char rlvi_type[5] = {'R', 'L', 'V', 'I'};
	char rlvc_type[5] = {'R', 'L', 'V', 'C'};
	char rlmc_type[5] = {'R', 'L', 'M', 'C'};
	char rltp_type[5] = {'R', 'L', 'T', 'P'};
	char rlim_type[5] = {'R', 'L', 'I', 'M'};

	for(i = 0; i < 32; i++)
		memset(temp3[i], 0, 24);
	for(x = 0; x < 16; x++)
	{
		if(memcmp(tag_type, rlpa_type, 4) == 0)
		{
			if ( x == 16 ) break;
			for(i = 0; i < strlen(tag_types_rlpa_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rlpa_list[x][i]);
			}
		} else if(memcmp(tag_type, rlts_type, 4) == 0) {
			if ( x == 5 ) break;
			for(i = 0; i < strlen(tag_types_rlts_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rlts_list[x][i]);
			}
		} else if(memcmp(tag_type, rlvi_type, 4) == 0) {
			if ( x == 1 ) break;
			for(i = 0; i < strlen(tag_types_rlvi_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rlvi_list[x][i]);
			}
		} else if(memcmp(tag_type, rlvc_type, 4) == 0) {
			if ( x == 16 ) break;
			for(i = 0; i < strlen(tag_types_rlvc_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rlvc_list[x][i]);
			}
		} else if(memcmp(tag_type, rlmc_type, 4) == 0) {
			if ( x == 16 ) break;
			for(i = 0; i < strlen(tag_types_rlmc_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rlmc_list[x][i]);
			}
		} else if(memcmp(tag_type, rltp_type, 4) == 0) {
			if ( x == 16 ) break;
			for(i = 0; i < strlen(tag_types_rltp_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rltp_list[x][i]);
			}
		} else if(memcmp(tag_type, rlim_type, 4) == 0) {
			if ( x == 5 ) break;
			for(i = 0; i < strlen(tag_types_rlim_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_rlim_list[x][i]);
			}
		} else {
			if ( x == 16 ) break;
			for(i = 0; i < strlen(tag_types_list[x]); i++)
			{
				temp3[x][i] = toupper(tag_types_list[x][i]);
			}
		}
	}
	head->entry_count = 0;
	subnode = node;
	for (x = 0, subnode = mxmlFindElement(subnode, node, "entry", NULL, NULL, MXML_DESCEND); subnode != NULL; subnode = mxmlFindElement(subnode, node, "entry", NULL, NULL, MXML_DESCEND), x++) {
		head->entry_count++;
		entry = realloc(entry, sizeof(tag_entry) * head->entry_count);
		entryinfo = realloc(entryinfo, sizeof(tag_entryinfo) * head->entry_count);
		if(data == NULL)
		{
			data = (tag_data**)malloc(sizeof(tag_data*) * head->entry_count);
			data2 = (tag_data2**)malloc(sizeof(tag_data2*) * head->entry_count);
		} else {
			data = (tag_data**)realloc(data, sizeof(tag_data*) * head->entry_count);
			data2 = (tag_data2**)realloc(data, sizeof(tag_data2*) * head->entry_count);
		}
		data[x] = NULL;
		data2[x] = NULL;
		memset(temp, 0, 256);
		memset(temp2, 0, 256);
		u8 type1;
		if(mxmlElementGetAttr(subnode, "type1") != NULL)
			type1 = strtoul( mxmlElementGetAttr(subnode, "type1") , NULL , 16 );
		else
			type1 = 0;
		if(mxmlElementGetAttr(subnode, "type2") != NULL)
			strcpy(temp, mxmlElementGetAttr(subnode, "type2"));
		else{
			printf("No type attribute found!\nSkipping this entry!\n");
			head->entry_count--;
			continue;
		}
		for(i = 0; i < strlen(temp); i++)
			temp2[i] = toupper(temp[i]);
		for(i = 0; (i < 17) && (strcmp(temp3[i - 1], temp2) != 0); i++);
		if(i == 17)
			i = atoi(temp2);
		else
			i--;

		entry[x].offset = 0;
		entryinfo[x].type1 = type1;
		entryinfo[x].type2 = i;
		entryinfo[x].data_type = 0x0200;
		entryinfo[x].pad1 = 0x0000;
		entryinfo[x].offset = 0x0000000C;
		entryinfo[x].coord_count = 0;
		subsubnode = subnode;
		for (i = 0, subsubnode = mxmlFindElement(subsubnode, subnode, "triplet", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "triplet", NULL, NULL, MXML_DESCEND), i++) {

			entryinfo[x].coord_count++;
			data[x] = realloc(data[x], sizeof(tag_data) * entryinfo[x].coord_count);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "frame", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"frame\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part1)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "value", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"value\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part2)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "blend", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"blend\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data[x][i].part3)) = atof(temp);
		}
		for (i = 0, subsubnode = mxmlFindElement(subnode, subnode, "pair", NULL, NULL, MXML_DESCEND); subsubnode != NULL; subsubnode = mxmlFindElement(subsubnode, subnode, "pair", NULL, NULL, MXML_DESCEND), i++) {
			entryinfo[x].data_type = 0x100;
			entryinfo[x].coord_count++;
			data2[x] = realloc(data2[x], sizeof(tag_data2) * entryinfo[x].coord_count);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "data1", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"data1\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			*(f32*)(&(data2[x][i].part1)) = atof(temp);
			tempnode = mxmlFindElement(subsubnode, subsubnode, "data2", NULL, NULL, MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"data2\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			data2[x][i].part2 = short_swap_bytes(strtoul(temp, NULL, 16));
			tempnode = mxmlFindElement(subsubnode, subsubnode, "padding", NULL,NULL,MXML_DESCEND);
			if(tempnode == NULL) {
				printf("Couldn't find attribute \"padding\"!\n");
				exit(1);
			}
			get_value(tempnode, temp, 256);
			data2[x][i].padding = short_swap_bytes(strtoul(temp, NULL, 16));
		}
	}

	head->pad1 = 0x0; head->pad2 = 0x0; head->pad3 = 0x0;
	WriteBRLANTagHeader(head, fp);
	u32 entryloc = ftell(fp);
	u32 animLen;
	WriteBRLANTagEntries(entry, head->entry_count, fp);
	u32* entryinfolocs = (u32*)calloc(head->entry_count, sizeof(u32));
	for(x = 0; x < head->entry_count; x++) {
		entryinfolocs[x] = ftell(fp);
		if (x>0)
		{
			if ( entryinfo[x].data_type == (0x200) )
			{
				entry[x].offset = entry[x-1].offset + sizeof(tag_entryinfo) + (entryinfo[x-1].coord_count * sizeof(tag_data));
			}
			if ( entryinfo[x].data_type == (0x100) )
			{
				entry[x].offset = entry[x-1].offset + sizeof(tag_entryinfo) + (entryinfo[x-1].coord_count * sizeof(tag_data2));
			}
		} else {
			entry[x].offset = x * (sizeof(tag_entryinfo) + sizeof(tag_data));
		}
		WriteBRLANTagEntryinfos(entryinfo[x], fp);
		if (x==0) animLen = ftell(fp);
		if ( entryinfo[x].data_type == 0x200 )
			WriteBRLANTagData(data[x], entryinfo[x].coord_count, fp);
		if ( entryinfo[x].data_type == 0x100 )
		{
			tag_data2 writedata;
			for(i = 0; i < entryinfo[x].coord_count; i++) {
				writedata.part1 = be32(data2[x][i].part1);
				writedata.part2 = (data2[x][i].part2);
				writedata.padding = (data2[x][i].padding);
				fwrite(&writedata, sizeof(tag_data2), 1, fp);
			}
		}
	}
	if ( entryinfo[0].data_type == 0x200 )
		free(data);
	if ( entryinfo[0].data_type == 0x100 )
		free(data2);
	u32 oldpos = ftell(fp);
	fseek(fp, entryloc, SEEK_SET);
	for( x = 0; x < head->entry_count; x++)
		entry[x].offset += sizeof(u32) * (head->entry_count) + 8;
	WriteBRLANTagEntries(entry, head->entry_count, fp);
	fseek(fp, oldpos, SEEK_SET);
	free(entry);
	free(entryinfo);
}

void create_tag_from_xml(mxml_node_t *pai1_node, mxml_node_t *node, FILE* fp)
{
	brlan_entry entr;
	entr.num_tags = 0x0;
	entr.is_material = 0x0;
	entr.pad[0] = 0x0;
	entr.pad[1] = 0x0;

	char temp[256];
	memset(entr.name, 0, 20);
	if(mxmlElementGetAttr(node, "name") != NULL)
		strcpy(entr.name, mxmlElementGetAttr(node, "name"));
	else{
	}
	if(mxmlElementGetAttr(node, "type") != NULL) {
		strcpy(temp, mxmlElementGetAttr(node, "type"));
		entr.is_material = (u8)atoi(temp);
	} else {
		// add nice safety
	}


	tag_header head;
	mxml_node_t *tagnode;
	for(tagnode = mxmlFindElement(node, node, "tag", NULL, NULL, MXML_DESCEND); tagnode != NULL; tagnode = mxmlFindElement(tagnode, node, "tag", NULL, NULL, MXML_DESCEND))
	{
		entr.num_tags++;
	}
	u32 brlanentryoffset = ftell(fp);
	WriteBRLANEntry(&entr, fp);

	u32 entrylocations[entr.num_tags];
	int i; for ( i = 0; i < entr.num_tags; i++) entrylocations[i] = brlanentryoffset;
	WriteBRLANOffsets(entrylocations, entr.num_tags, fp);

	entr.num_tags = 0x0;
	for(tagnode = mxmlFindElement(node, node, "tag", NULL, NULL, MXML_DESCEND); tagnode != NULL; tagnode = mxmlFindElement(tagnode, node, "tag", NULL, NULL, MXML_DESCEND))
	{
		entrylocations[entr.num_tags] = ftell(fp) - entrylocations[entr.num_tags];
		if(mxmlElementGetAttr(tagnode, "type") != NULL)
			strcpy(temp, mxmlElementGetAttr(tagnode, "type"));
		else {
			printf("No type attribute found!\nQuitting!\n");
			exit(1);
		}
		for ( i = 0; i < 4; i++) head.magic[i] = temp[i];
		if (FourCCInList(head.magic) == 0)
		{
			printf("animation type not recognized: %.4s", head.magic);
			exit(1);
		}
		create_entries_from_xml(node, tagnode, &entr, &head, fp);
		entr.num_tags++;
	}
	u32 tempOffset = ftell(fp);
	fseek(fp, brlanentryoffset + sizeof(brlan_entry), SEEK_SET);
	WriteBRLANOffsets(entrylocations, entr.num_tags, fp);
	fseek(fp, tempOffset, SEEK_SET);
}

void WriteBRLANHeader(brlan_header rlanhead, FILE* fp)
{
	brlan_header writehead;
	writehead.magic[0] = rlanhead.magic[0];
	writehead.magic[1] = rlanhead.magic[1];
	writehead.magic[2] = rlanhead.magic[2];
	writehead.magic[3] = rlanhead.magic[3];
	writehead.endian  = be16(rlanhead.endian);
	writehead.version = be16(rlanhead.version);
	writehead.file_size = be32(rlanhead.file_size);
	writehead.pai1_offset = short_swap_bytes(rlanhead.pai1_offset);
	writehead.pai1_count = short_swap_bytes(rlanhead.pai1_count);
	fwrite(&writehead, sizeof(brlan_header), 1, fp);
}

void WriteBRLANPaiHeader(brlan_pai1_header_type1 paihead, FILE* fp)
{
	brlan_pai1_header_type1 writehead;
	writehead.magic[0] = paihead.magic[0];
	writehead.magic[1] = paihead.magic[1];
	writehead.magic[2] = paihead.magic[2];
	writehead.magic[3] = paihead.magic[3];
	writehead.size = be32(paihead.size);
	writehead.framesize = short_swap_bytes(paihead.framesize);
	writehead.flags = paihead.flags;
	writehead.padding = paihead.padding;
	writehead.num_timgs = short_swap_bytes(paihead.num_timgs);
	writehead.num_entries = short_swap_bytes(paihead.num_entries);
	writehead.entry_offset = be32(paihead.entry_offset);
	fwrite(&writehead, sizeof(brlan_pai1_header_type1), 1, fp);
}

void write_brlan(char *infile, char* outfile)
{
	CreateTagTypesLists();
	int i;

	FILE* fpx = fopen(infile, "r");
	if(fpx == NULL) {
		printf("xmlan couldn't be opened!\n");
		exit(1);
	}
	mxml_node_t *hightree = mxmlLoadFile(NULL, fpx, MXML_TEXT_CALLBACK);
	if(hightree == NULL) {
		printf("Couldn't open hightree!\n");
		exit(1);
	}
	mxml_node_t *tree = mxmlFindElement(hightree, hightree, "xmlan", NULL, NULL, MXML_DESCEND);
	if(tree == NULL) {
		printf("Couldn't get tree!\n");
		exit(1);
	}
	char tempVersion[9];
	if(mxmlElementGetAttr(tree, "brlan_version") != NULL)
		strcpy(tempVersion, mxmlElementGetAttr(tree, "brlan_version"));
	else{
		printf("No brlan_version attribute found!\n");
		exit(1);
	}
	u32 versionn = strtoul(tempVersion, NULL, 16);

	mxml_node_t *node;
	FILE* fp = fopen(outfile, "wb+");
	if(fpx == NULL) {
		printf("destination brlan couldn't be opened!\n");
		exit(1);
	}
	u32 fileSize = 0;
	u16 blobcount = 0;
	brlan_header rlanhead;
	rlanhead.magic[0] = 'R';
	rlanhead.magic[1] = 'L';
	rlanhead.magic[2] = 'A';
	rlanhead.magic[3] = 'N';
	rlanhead.endian = 0xFEFF;
	//rlanhead.version = 0x0008;
	rlanhead.version = versionn;
	rlanhead.file_size = 0;
	rlanhead.pai1_offset = sizeof(brlan_header);
	rlanhead.pai1_count = 1;
	u32 chunk_count = 0;
	WriteBRLANHeader(rlanhead, fp);


	mxml_node_t * pat1_node;
	for(pat1_node = mxmlFindElement(tree, tree, "pat1", NULL, NULL, MXML_DESCEND); pat1_node != NULL; pat1_node = mxmlFindElement(pat1_node, tree, "pat1", NULL, NULL, MXML_DESCEND))
	{
		u32 pat1_offset = ftell(fp);
		brlan_pat1_universal pathead;
		pathead.magic[0] = 'p';
		pathead.magic[1] = 'a';
		pathead.magic[2] = 't';
		pathead.magic[3] = '1';
		pathead.size = 0;
		pathead.unk1 = 5;
		pathead.group_num = 4;
		pathead.unk3_offset = be32(0x1c);
		pathead.group_array_offset = be32(0x2c);
		pathead.unk5a = 0x008c;
		pathead.unk5b = 0x00a0;
		pathead.isDecendingBind = 0;
		pathead.padding = 0;
		fwrite(&pathead, sizeof(brlan_pat1_universal), 1,fp);

		mxml_node_t *unk1, *unk2, *unk5a, *unk5b, *unk6;
		mxml_node_t *first, *second, *pad;
		unk1 = mxmlFindElement(pat1_node, pat1_node, "unk1", NULL, NULL, MXML_DESCEND);
		if ( unk1 != NULL )
		{
			char temp[256];
			get_value(unk1, temp, 256);
			pathead.unk1 = strtoul(temp, NULL, 16);
			pathead.unk1 = short_swap_bytes(pathead.unk1);
		}
		/*
		unk2 = mxmlFindElement(pat1_node, pat1_node, "unk2", NULL, NULL, MXML_DESCEND);
		if ( unk2 != NULL )
		{
			char temp[256];
			get_value(unk2, temp, 256);
			pathead.unk2 = strtoul(temp, NULL, 16);
			pathead.unk2 = short_swap_bytes(pathead.unk2);
		}
		*/
		unk5a = mxmlFindElement(pat1_node, pat1_node, "unk5a", NULL, NULL, MXML_DESCEND);
		if ( unk5a != NULL )
		{
			char temp[256];
			get_value(unk5a, temp, 256);
			pathead.unk5a = strtoul(temp, NULL, 16);
			pathead.unk5a = short_swap_bytes(pathead.unk5a);
		}
		unk5b = mxmlFindElement(pat1_node, pat1_node, "unk5b", NULL, NULL, MXML_DESCEND);
		if ( unk5b != NULL )
		{
			char temp[256];
			get_value(unk5b, temp, 256);
			pathead.unk5b = strtoul(temp, NULL, 16);
			pathead.unk5b = short_swap_bytes(pathead.unk5b);
		}
		unk6 = mxmlFindElement(pat1_node, pat1_node, "isDecendingBind", NULL, NULL, MXML_DESCEND);
		if ( unk6 != NULL )
		{
			char temp[256];
			get_value(unk6, temp, 256);
			pathead.isDecendingBind = strtoul(temp, NULL, 16);
			//pathead.unk6 = short_swap_bytes(pathead.unk6);
		}
		pad = mxmlFindElement(pat1_node, pat1_node, "padding", NULL, NULL, MXML_DESCEND);
		if ( pad != NULL )
		{
			char temp[256];
			get_value(unk6, temp, 256);
			pathead.padding = strtoul(temp, NULL, 16);
			//pathead.padding = short_swap_bytes(pathead.padding);
		}

		u32 cnt1 = 1;
		char temp1[0x40];
		memset(temp1, 0, 0x40);
		first = mxmlFindElement(pat1_node, pat1_node, "first", NULL, NULL, MXML_DESCEND);
		if ( first != NULL )
			get_value(first, temp1, 0x10);
		u32 temp1_len = strlen(temp1);
		if ( temp1_len % 4 )
			temp1_len += (4 - (temp1_len % 4));
		fwrite(temp1, temp1_len, 1, fp);
		//pathead.group_array_offset = be32(be32(pathead.unk3_offset) + temp1_len);
		pathead.group_array_offset = be32(0x1C + temp1_len);

		u32 cnt2 = 0;
		second = mxmlFindElement(pat1_node, pat1_node, "seconds", NULL, NULL, MXML_DESCEND);
		if ( second != NULL )
		{
			dbgprintf("second\n");
			mxml_node_t *str_node;
			for(str_node = mxmlFindElement(second, second, "string", NULL, NULL, MXML_DESCEND); str_node != NULL; str_node = mxmlFindElement(str_node, second, "string", NULL, NULL, MXML_DESCEND))
			{
				cnt2++;
			}
			u32 tmp2_size = sizeof(char) * cnt2 * 0x14;
			dbgprintf("size: %d\n", tmp2_size);
			char * temp2 = malloc(sizeof(char) * cnt2 * 0x14);
			memset(temp2, 0, sizeof(char) * cnt2 * 0x14);
			char * ofs = temp2;
			for(str_node = mxmlFindElement(second, second, "string", NULL, NULL, MXML_DESCEND); str_node != NULL; str_node = mxmlFindElement(str_node, second, "string", NULL, NULL, MXML_DESCEND))
			{
				dbgprintf("loopy\n");
				char tempp[256];
				memset(tempp, 0, 256);
				get_value(str_node, tempp, 256);
				memcpy(ofs, tempp, 0x14);
				ofs += 0x14;
			}
			fwrite(temp2, tmp2_size, 1, fp);
			free(temp2);
		}
		pathead.group_num = cnt2;
		pathead.group_num = short_swap_bytes(pathead.group_num);
		u32 temp_offs = ftell(fp);
		pathead.size = temp_offs - pat1_offset;
		pathead.size = be32(pathead.size);
		fseek(fp, pat1_offset, SEEK_SET);
		fwrite(&pathead, sizeof(brlan_pat1_universal), 1,fp);
		fseek(fp, temp_offs, SEEK_SET);
		fileSize = ftell(fp);
		chunk_count++;
	}

	mxml_node_t * pai1_node;
	for(pai1_node = mxmlFindElement(tree, tree, "pai1", NULL, NULL, MXML_DESCEND); pai1_node != NULL; pai1_node = mxmlFindElement(pai1_node, tree, "pai1", NULL, NULL, MXML_DESCEND))
	{
		u32 pai1_offset = ftell(fp);

		brlan_pai1_header_type1 paihead;
		paihead.magic[0] = 'p';
		paihead.magic[1] = 'a';
		paihead.magic[2] = 'i';
		paihead.magic[3] = '1';
		paihead.size = 0;
		char temp[256];
		if(mxmlElementGetAttr(pai1_node, "framesize") != NULL)
			strcpy(temp, mxmlElementGetAttr(pai1_node, "framesize"));
		else{
			printf("No framesize attribute found!\nDefaulting to 20.");
			strcpy(temp, "20");
		}
		paihead.framesize = atoi(temp);
		memset(temp, 0, 256);
		if(mxmlElementGetAttr(pai1_node, "flags") != NULL)
		{
			strcpy(temp, mxmlElementGetAttr(pai1_node, "flags"));
		} else {
			printf("No flags attribute found!\nDefaulting to 1.");
			paihead.flags = 1;
		}
		paihead.flags = atoi(temp);
		paihead.padding = 0;
		paihead.num_timgs = 0;
		paihead.num_entries = 0;
		paihead.entry_offset = sizeof(brlan_pai1_header_type1);
		WriteBRLANPaiHeader(paihead, fp);

		u32 totaltagsize = 0;

		u16 timgcount = 0;
		u32 totaltimgize = 0;

		u32 timgnumber = 0x0;
		for(node = mxmlFindElement(pai1_node, pai1_node, "timg", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, pai1_node, "timg", NULL, NULL, MXML_DESCEND))
		{
			timgnumber++;
		}
		u32 imageoffsets[timgnumber];
		u32 imagefileoffset = ftell(fp);
		for( i = 0; i < timgnumber; i++) imageoffsets[i] = imagefileoffset;
		WriteBRLANOffsets(imageoffsets, timgnumber, fp);

		for(node = mxmlFindElement(pai1_node, pai1_node, "timg", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, pai1_node, "timg", NULL, NULL, MXML_DESCEND)) {
			timgcount++;
			imageoffsets[timgcount-1] = ftell(fp) - imageoffsets[timgcount-1];
			if(mxmlElementGetAttr(node, "name") != NULL)
				strcpy(temp, mxmlElementGetAttr(node, "name"));
			else{
				printf("No name attribute found!\n");
				exit(1);
			}
			fwrite(temp, strlen(temp) + 1, 1, fp);
			totaltimgize += strlen(temp) + 1;
		}
		if ((totaltimgize % 4) != 0)
		{
			u8 blank[3] = {0x0, 0x0, 0x0};
			fwrite(blank, (4 - (totaltimgize % 4)), 1, fp);
			totaltimgize += (4 - (totaltimgize % 4));
		}
		u32 tempoOffset = ftell(fp);
		fseek(fp, imagefileoffset, SEEK_SET);
		WriteBRLANOffsets(imageoffsets, timgnumber, fp);
		fseek(fp, tempoOffset, SEEK_SET);

		u32 panecount = 0x0;
		for(node = mxmlFindElement(pai1_node, pai1_node, "pane", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, pai1_node, "pane", NULL, NULL, MXML_DESCEND))
		{
			panecount++;
		}
		u32 paneoffsets[panecount];
		u32 tagoffset = ftell(fp);
		for(i = 0; i < panecount; i++)
			paneoffsets[i] = pai1_offset;
		WriteBRLANOffsets(paneoffsets, panecount, fp);

		for(node = mxmlFindElement(pai1_node, pai1_node, "pane", NULL, NULL, MXML_DESCEND); node != NULL; node = mxmlFindElement(node, pai1_node, "pane", NULL, NULL, MXML_DESCEND))
		{
			blobcount++;
			paneoffsets[blobcount-1] = ftell(fp) - paneoffsets[blobcount-1];
			create_tag_from_xml(pai1_node, node, fp);
			totaltagsize = ftell(fp) - tagoffset;
		}
		fileSize = ftell(fp);
		fseek(fp, tagoffset, SEEK_SET);
		WriteBRLANOffsets(paneoffsets, blobcount, fp);


		paihead.num_timgs = timgcount;
		paihead.entry_offset = sizeof(brlan_pai1_header_type1) +totaltimgize + (4*paihead.num_timgs);
		paihead.num_entries = blobcount;
		fseek(fp, 0, SEEK_END);
		paihead.size = fileSize - pai1_offset;

		u32 tempp = ftell(fp);
		fseek(fp, pai1_offset, SEEK_SET);
		WriteBRLANPaiHeader(paihead, fp);
		fseek(fp, tempp, SEEK_SET);

		chunk_count++;
	}



	rlanhead.file_size = fileSize;
	rlanhead.pai1_count = chunk_count;
	fseek(fp, 0, SEEK_SET);
	WriteBRLANHeader(rlanhead, fp);
}

void make_brlan(char* infile, char* outfile)
{
	printf("Parsing XMLAN @ %s.\n", infile);
	write_brlan(infile, outfile);
}
