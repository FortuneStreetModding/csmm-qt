/******************************************************************************
 *  brlyt.h                                                                   *
 *  Part of Benzin                                                            *
 *  Handles BRLYT banner datas.                                               *
 *  Copyright (C)2009 SquidMan (Alex Marshall)        <SquidMan72@gmail.com>  *
 *  Copyright (C)2009 megazig  (Stephen Simpson)       <megazig@hotmail.com>  *
 *  Copyright (C)2009 Matt_P   (Matthew Parlane)                              *
 *  Copyright (C)2009 comex                                                   *
 *  Copyright (C)2009 booto                                                   *
 *  All Rights Reserved, HACKERCHANNEL.                                       *
 ******************************************************************************/

#ifndef BRLYT_H_
#define BRLYT_H_

#include "types.h"

typedef struct
{
    fourcc		magic;				// RLYT
	u16			endian;				// 0xFEFF
	u16			version;			// 0x0008
	u32			filesize;			// The filesize of the brlyt.
	u16			lyt_offset;			// Offset to the lyt1 section.
	u16			sections;			// Number of sections.
} brlyt_header;

typedef struct
{
	fourcc		magic;				// The magic.
	u32			length;				// How long the entry is.
} brlyt_entry_header;

typedef struct
{
	fourcc		magic;				// The magic.
	u32			length;				// How long the entry is.
	u32			data_location;		// The location of the data in the BRLYT file.
} brlyt_entry;

typedef struct
{
	u8			flag1;				// bits: 31-visibility 30-aspect_affected 29-??
	u8			origin;
	u8			alpha;
	u8			pad;
	char		name[0x10];
	char		userdata[0x08];
	float		XTrans;
	float		YTrans;
	float		ZTrans;
	float		XRotate;
	float		YRotate;
	float		ZRotate;
	float		XScale;
	float		YScale;
	float		width;
	float		height;
} brlyt_pane_chunk;

typedef struct
{
	u16			len1;
	u16			len2;
	u16			mat_num;
	u16			font_idx;
	u8			alignment;
	u8			unk_char;
	u8			pad[2];
	u32			name_offs;
	u32			color1;
	u32			color2;
	float		font_size_x;
	float		font_size_y;
	float		char_space;
	float		line_space;
} brlyt_text_chunk;

typedef struct						// starts @ 0x4c
{
	u32			vtx_colors[4];		// [4294967295L, 4294967295L, 4294967295L, 4294967295L]
	u16			mat_num;
	u8			num_texcoords;
	u8			padding;			// 0
} brlyt_pic_chunk;

typedef struct
{
	float		coords[4];			// all 0x00000000
	u8			frame_count;		// 0x01
	u8			padding[3];			// 0x00 0x00 0x00
	u32			offset1;			// 0x00000068   offset to brlyt_wnd1 colors
	u32			offset2;			// 0x0000007c	offset to frame offset list
} brlyt_wnd;

typedef struct						// pointed at by offset1
{
	u32			colors[4];			// all 0xffffffff
	u16			material;			// 0x0000
	u8			coordinate_count;	// 0x01
	u8			padding;			// 0x00
} brlyt_wnd1;

typedef struct
{
	float		texcoords[8];		// 0x00 0x00 3f800000 0x00 0x00 3f800000 3f800000 3f800000
} brlyt_wnd3;

typedef struct						// pointed to by offset2
{
	u32			offset;				// offset to something
} brlyt_wnd4;

typedef struct
{
	u16			material;			// material number
	u8			index;
	u8			padding;
} brlyt_wnd4_mat;

typedef struct
{
	char		drawnFromMiddle;// related to whether is drawn from middle // 1 = TRUE 2 = FALSE
	char		pad[3];
	float		width;
	float		height;
} brlyt_lytheader_chunk;

typedef struct
{
	char		name[16];
	u16			numsubs;
	u16			unk;
} brlyt_group_chunk;

typedef struct
{
	/* possible a numoffs */
	u16			string_count;
	u16			unk2;
} brlyt_usdstart_chunk;

typedef struct
{
	u32			string_offset;
	u32			is_working_offset;
	u16			unk5;
	u8			unk6;
	u8			unk7;
} brlyt_usdmain_chunk;

typedef struct
{
	u16			num;
	u16			offs;
} brlyt_numoffs_chunk;

typedef struct
{
	char		name[20];
	s16			forecolor[4];
	s16			backcolor[4];
	s16			colorReg3[4];
	u32			tev_kcolor[4];
	u32			flags;
} brlyt_material_chunk;

typedef struct
{
	int			offset;
	int			unk;
} brlyt_offsunk_chunk;

typedef struct
{
	u16			tex_offs;
	u8			wrap_s;
	u8			wrap_t;
} brlyt_texref_chunk;

typedef struct
{
	float		XTrans;
	float		YTrans;
	float		Rotate;
	float		XScale;
	float		YScale;
} brlyt_tex_srt;

typedef struct
{
	char		tgen_type;
	char		tgen_src;
	char		mtxsrc;
	char		padding;
} brlyt_tex_coordgen;

typedef struct
{
	char		color_matsrc;
	char		alpha_matsrc;
	char		padding1;
	char		padding2;
} brlyt_tex_chanctrl;

typedef struct
{
	u8			red;
	u8			green;
	u8			blue;
	u8			alpha;
} brlyt_tex_matcol;

typedef struct
{
	unsigned	red		: 2;
	unsigned	green	: 2;
	unsigned	blue	: 2;
	unsigned	alpha	: 2;
} brlyt_tev_swapmode;

typedef struct
{
	unsigned	Texture		: 4;
	unsigned	TexSRT		: 4;
	unsigned	TexCoord	: 4;
	unsigned	ua6			: 1;
	unsigned	ua7			: 2;
	unsigned	ua8			: 3;
	unsigned	ua9			: 5;
	unsigned	uaa			: 1;
	unsigned	uab			: 1;
	unsigned	ua4			: 1;
	unsigned	padding1	: 1;
	unsigned	ua5			: 1;
	unsigned	padding2	: 4;
} mat_flags;

// 0000 0000  0000 0000  0000 0000  0000 0000
// xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
// ---- 5-4b  a999 9988  8776 3333  2222 1111

typedef struct
{
	union
	{
		mat_flags	flag;
		u32			flags;
	};
} mat1_flags;

typedef struct
{
	u8			one;
	u8			two;
	u8			three;
	u8			four;
} brlyt_tev_swapmodetable;

typedef struct
{
	float		XTrans;
	float		YTrans;
	float		Rotate;
	float		XScale;
	float		YScale;
} brlyt_indtex_srt;

typedef struct
{
	u8			tex_coord;
	u8			tex_map;
	u8			scale_s;
	u8			scale_t;
} brlyt_indtex_order;

typedef struct
{
	unsigned texcoord  : 8;	// GX_SetTevOrder( i , this , *         , *    );
	unsigned color     : 8;	// GX_SetTevOrder( i , *    , *         , this );
	unsigned texmapbot : 8;	// GX_SetTevOrder( i , *    , this | *2 , *    );	*1
	unsigned texmaptop : 1;	// GX_SetTevOrder( i , *     , *1 | this , *    );	*2
	unsigned ras_sel   : 2;	// GX_SetTevSwapMode( i , this , *    );
	unsigned tex_sel   : 2;	// GX_SetTevSwapMode( i , *    , this );
	unsigned empty1    : 3;//pad
	unsigned aC        : 4;	// GX_SetTevColorIn( i , this , *    , *    , *    );
	unsigned bC        : 4;	// GX_SetTevColorIn( i , *    , this , *    , *    );
	unsigned cC        : 4;	// GX_SetTevColorIn( i , *    , *    , this , *    );
	unsigned dC        : 4;	// GX_SetTevColorIn( i , *    , *    , *    , this );
	unsigned tevscaleC : 2;	// GX_SetTevColorOp( i , * , * , this , * , * );
	unsigned tevbiasC  : 2; // GX_SetTevColorOp( i , * , this , * , * , * );
	unsigned tevopC    : 4; // GX_SetTevColorOp( i , this , * , * , * , * );
	unsigned tevregidC : 1; // GX_SetTevColorOp( i , * , * , * , * , this );
	unsigned clampC    : 2; // GX_SetTecColorOp( i , * , * , * , this , * );
	unsigned selC      : 5; // GX_SetTevKColorSel( i , this );
	unsigned aA        : 4;	// GX_SetTevAlphaIn( i , this , *    , *    , *    );
	unsigned bA        : 4;	// GX_SetTevAlphaIn( i , *    , this , *    , *    );
	unsigned cA        : 4;	// GX_SetTevAlphaIn( i , *    , *    , this , *    );
	unsigned dA        : 4;	// GX_SetTevAlphaIn( i , *    , *    , *    , this );
	unsigned tevscaleA : 2;	// GX_SetTevAlphaOp( i , * , * , this , * , * );
	unsigned tevbiasA  : 2; // GX_SetTevAlphaOp( i , * , this , * , * , * );
	unsigned tevopA    : 4; // GX_SetTevAlphaOp( i , this , * , * , * , * );
	unsigned tevregidA : 1; // GX_SetTevAlphaOp( i , * , * , * , * , this );
	unsigned clampA    : 2; // GX_SetTecAlphaOp( i , * , * , * , this , * );
	unsigned selA      : 5; // GX_SetTevKAlphaSel( i , this );
	unsigned indtexid  : 8; // GX_SetTevIndirect( i , this , * , * , *    , * , * , * , * , * );
	unsigned bias      : 3; // GX_SetTevIndirect( i , *    , * , this , * , * , * , * , * , * );
	unsigned mtxid     : 4; // GX_SetTevIndirect( i , *    , * , * , this , * , * , * , * , * );
	unsigned empty2    : 1;//pad
	unsigned wrap_s    : 3; // GX_SetTevIndirect( i , * , * , * , * , this , * , * , * , * );
	unsigned wrap_t    : 3; // GX_SetTevIndirect( i , * , * , * , * , * , this , * , * , * );
	unsigned empty3    : 2;//pad
	unsigned format    : 2; // GX_SetTevIndirect( i , * , this , * , * , * , * , * , * , * );
	unsigned addprev   : 1; // GX_SetTevIndirect( i , * , * , * , * , * , * , this , * , * );
	unsigned utclod    : 1; // GX_SetTevIndirect( i , * , * , * , * , * , * , * , this , * );
	unsigned aIND      : 2; // GX_SetTevIndirect( i , * , * , * , * , * , * , * , * , this );
	unsigned empty4    : 2;//pad
} TevStages;

typedef struct
{
	unsigned	comp0	: 4;
	unsigned	comp1	: 4;	// had to switch
	unsigned	aop		: 8;
	unsigned	ref0	: 8;
	unsigned	ref1	: 8;
} brlyt_alpha_compare;

typedef struct
{
	u8			type;
	u8			src_fact;
	u8			dst_fact;
	u8			op;
} brlyt_blend_mode;

u8 swapBits( u8 char1 );
u32 bitExtraction(u32 num, u32 start, u32 end);

void parse_brlyt(char *filename, char *filenameout);
void make_brlyt(char* infile, char* outfile);

#endif //BRLYT_H_

