/******************************************************************************
 *  brlan.h                                                                   *
 *  Part of Benzin                                                            *
 *  Handles BRLAN banner datas.                                               *
 *  Copyright (C)2009 SquidMan (Alex Marshall)        <SquidMan72@gmail.com>  *
 *  Copyright (C)2009 megazig  (Stephen Simpson)       <megazig@hotmail.com>  *
 *  Copyright (C)2009 Matt_P   (Matthew Parlane)                              *
 *  Copyright (C)2009 comex                                                   *
 *  Copyright (C)2009 booto                                                   *
 *  All Rights Reserved, HACKERCHANNEL.                                       *
 ******************************************************************************/

#ifndef BRLAN_H_
#define BRLAN_H_

#include "types.h"

typedef enum
{
	RLPA_ENTRY		= 0,		// Pane Animation .aka. Pane variable animation
								//		byte 1 not used byte 2 is which variable
								//		*( &pane.XTrans + type2<<2 ) = result
	RLTS_ENTRY		= 1,		// Texture ScaleRotateTranslate Mapping
								//		byte 1 is which SRT and byte 2 is which elem.
								//		*( GetTexSRTArray()[type1] + (type2*4) ) = result
	RLVI_ENTRY		= 2,		// Visibility	// stores data2 to bottom of flag1
								//		bytes 1 and 2 arent' used 
								//		( pane.flag1 & 0xfe ) |= ( result & 0x01 )
	RLVC_ENTRY		= 3,		// Vertex Color	// byte2 is passed to Pane::SetColorElement();
								//		byte 1 not used byte 2 is which vertex and color
								//		16-19 goes to unknown vars
								//		20-23 goes to textureCoordinate pointer :D
								//		Pane::SetColorElement( type2 , result );
	RLMC_ENTRY		= 4,		// Material Color
								//		byte1 is not used byte 2 is which color
								//		Material::SetColorElement( type2 , color );
	RLTP_ENTRY		= 5,		// TPL Pallete
								//		byte 1 is which Texture byte 2 must be 0
								//		SetTextureNoWrap( type1 , (return_val<<2) + anim_basic.pointer2 );
	RLIM_ENTRY					// IndTexSRTAry Material
								//		byte 1 is which SRT and  byte 2 is which elem.
								//		*( GetIndTexSRTArray()[type1] + (type2*4) ) = result
} brlan_entry_type;

typedef struct
{
	fourcc		magic;				// "pai1" in ASCII.
	u32			size;				// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u16			framesize;			// Framesize
	u8			flags;				// Flags		// isLoopData
	u8			padding;			// Padding
	u16			num_timgs;			// Number of timgs?
	u16			num_entries;        // Number of tags in the brlan.
	u32			padding2;			// Only if bit 25 of flags is set.
	u32			entry_offset;		// Offset to entries. (Relative to start of pai1 header.)
} brlan_pai1_header_type2;

typedef struct
{
	fourcc		magic;				// "RLAN" in ASCII.
	u16			endian;				// Always 0xFEFF. Tells endian.
	u16			version;			// Always 0x0008. Version of brlan format
	u32			file_size;			// Size of whole file, including the header.
	u16			pai1_offset;		// The offset to the pa*1 header from the start of file.
	u16			pai1_count;			// How many pa*1 sections there are
} brlan_header;

typedef struct
{
	fourcc		magic;						// "pah1" in ASCII.
	u32			size;
	u32			animationShareInfo_offset;	// from pah1
	u16			animationShareInfoNum;		// 802b5c80
} brlan_pah1_universal;

typedef struct
{
	char pane_name[0x10];			// for FindPaneByName()
	char pad1;						// to fill out hole
	char group_name[0x10];			// GroupName
	char pad2[0x3];					// to fill out hole
} brlan_pah1_animShareInfoArray;	// 0x24 bytes long

typedef struct
{
	fourcc		magic;				// "pat1" in ASCII.
	u32			size;
	u16			unk1;
	u16			group_num;			// 802b5c20
							// number of second strings
	u32			unk3_offset;		// offs from pat1
							// offset to first string
	u32			group_array_offset;	// 802b5c40 offs from pat1
							// offset to second string
							// every 0x14 from here
	u16			unk5a;
	u16			unk5b;
	u8			isDecendingBind;	// 802b5c60
							// bit31 - IsDecendingBind
	u8			padding;
} brlan_pat1_universal;

typedef struct
{
	fourcc		magic;				// "pai1" in ASCII.
	u32			size;				// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u16			framesize;			// Framesize
	u8			flags;				// Flags
	u8			padding;			// Padding
	u16			num_timgs;			// Number of timgs?
	u16			num_entries;		// Number of tags in the brlan.
} brlan_pai1_universal;

typedef struct
{
	fourcc		magic;				// "pai1" in ASCII.
	u32			size;				// Size of section, which is rest of the file. (header.file_size - header.offset_pai1)
	u16			framesize;			// Framesize
	u8			flags;				// Flags
	u8			padding;			// Padding
	u16			num_timgs;			// Number of timgs?
	u16			num_entries;		// Number of tags in the brlan.
	u32			entry_offset;		// Offset to entries. (Relative to start of pai1 header.)
} brlan_pai1_header_type1;			// followed by offset list

typedef struct						// pointed at by offset list + pai1_header
{
	char		name[20];			// Name of the BRLAN entry. (Must be defined in the BRLYT)
	u8			num_tags;
	u8			is_material;		// if 0 it's the name of a pane
	u8			pad[2];
} brlan_entry;						// followed by offset list

typedef struct						// pointed at by offset list + brlan_entry
{
	fourcc		magic;
	u8			entry_count;		// How many entries in this chunk.
	u8			pad1;				// All cases I've seen is zero.
	u8			pad2;				// All cases I've seen is zero.
	u8			pad3;				// All cases I've seen is zero.
} tag_header;

typedef struct
{
	u32			offset;				// Offset to the data pointed to by this entry.
									// Relative to the start of the RLPA header.
} tag_entry;

typedef struct
{
	//u16			type;				// Type (look at animtypes)
	u8			type1;
	u8			type2;
	u16			data_type;			// Every case has been 0x0200 // 0x0100 for pairs
	u16			coord_count;		// How many coordinates.
	u16			pad1;				// All cases I've seen is zero.
	u32			offset;				// Offset to tag data
} tag_entryinfo;

typedef struct
{									// Bits not listed here are currently unknown.
	u32			part1;				// If Bit 9 is set in flags, this is an f32, with a coordinate. (Bit 17 seems to act the same)
	u32			part2;				// If Bit 16 is set in flags, this is an f32, with another coordinate. (Bit 17 seems to act the same)
	u32			part3;				// With Bit 16 set in flags, this seems to be yet another coordinate. (Bit 17 seems to act the same)
} tag_data;

typedef struct
{
	u32			part1;
	u16			part2;
	u16			padding;
} tag_data2;


void parse_brlan(char* filename, char *filenameout);
void make_brlan(char* infile, char* outfile);

#endif //BRLAN_H_

