/******************************************************************************
 *  endian.c                                                                  *
 *  Part of Benzin                                                            *
 *  Handles endianness.                                                       *
 *  Copyright (C)2009 SquidMan (Alex Marshall)        <SquidMan72@gmail.com>  *
 *  Copyright (C)2009 megazig  (Stephen Simpson)       <megazig@hotmail.com>  *
 *  Copyright (C)2009 Matt_P   (Matthew Parlane)                              *
 *  Copyright (C)2009 comex                                                   *
 *  Copyright (C)2009 booto                                                   *
 *  All Rights Reserved, HACKERCHANNEL.                                       *
 ******************************************************************************/

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include "types.h"
#include "endian.h"

//#define be16(x)        ((x>>8)|(x<<8))
//#define be32(x)        ((x>>24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x<<24))
//#define be64(x)        ((x>>56)|((x<<40)&(u64)0x00FF000000000000)|((x<<24)&(u64)0x0000FF0000000000)|((x<<8)&(u64)0x000000FF00000000)|((x>>8)&(u64)0x00000000FF000000)|((x>>24)&(u64)0x0000000000FF0000)|((x<<40)&(u64)0x000000000000FF00)|(x<<56))

u16 be16(u16 x)
{
    return (u16)(((x>>8)) | 
        ((x&0xff)<<8));
}

u32 be32(u32 x)
{
    return (x>>24) | 
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
        (x<<24);
}

u32 int_swap_bytes(u32 int1)
{
        unsigned char *int1c; int1c = (unsigned char*)&int1;

        unsigned char charTemp = 0x00;
        charTemp = int1c[0]; int1c[0] = int1c[3]; int1c[3] = charTemp;
        charTemp = 0x00;
        charTemp = int1c[1]; int1c[1] = int1c[2]; int1c[2] = charTemp;

        u32 *newInt; newInt = (unsigned int*)int1c;
        return *newInt;
}

u16 short_swap_bytes(u16 short1)
{
        unsigned char* short1c; short1c = (unsigned char*)&short1;
        unsigned char charTemp = 0x00;
        charTemp = short1c[0]; short1c[0] = short1c[1]; short1c[1] = charTemp;

        unsigned short *newShort; newShort = (unsigned short*)short1c;
        return *newShort;
}

// __int64 for MSVC, "long long" for gcc
u64 be64(u64 x)
{
    return (x>>56) | 
        ((x<<40) & (u64)0x00FF000000000000LL) |
        ((x<<24) & (u64)0x0000FF0000000000LL) |
        ((x<<8)  & (u64)0x000000FF00000000LL) |
        ((x>>8)  & (u64)0x00000000FF000000LL) |
        ((x>>24) & (u64)0x0000000000FF0000LL) |
        ((x>>40) & (u64)0x000000000000FF00LL) |
        (x<<56);
}

#endif //_ENDIAN_H_

