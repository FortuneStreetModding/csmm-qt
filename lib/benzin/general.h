/******************************************************************************
 *  general.h                                                                 *
 *  Part of Benzin                                                            *
 *  Handles general stuff.                                                    *
 *  Copyright (C)2009 SquidMan (Alex Marshall)        <SquidMan72@gmail.com>  *
 *  Copyright (C)2009 megazig  (Stephen Simpson)       <megazig@hotmail.com>  *
 *  Copyright (C)2009 Matt_P   (Matthew Parlane)                              *
 *  Copyright (C)2009 comex                                                   *
 *  Copyright (C)2009 booto                                                   *
 *  All Rights Reserved, HACKERCHANNEL.                                       *
 ******************************************************************************/

#define BENZIN_VERSION_MAJOR        2
#define BENZIN_VERSION_MINOR        1
#define BENZIN_VERSION_BUILD        11
#define BENZIN_VERSION_OTHER        "BETA"

#define INFORMATION_TEXT        \
"Benzin %d.%d.%d%s.\n" \
"Written by SquidMan (Alex Marshall), comex, and megazig.\n" \
"(c) 2009 HACKERCHANNEL\n", BENZIN_VERSION_MAJOR, BENZIN_VERSION_MINOR, BENZIN_VERSION_BUILD, BENZIN_VERSION_OTHER

#define fatal(x)    printf(x); exit(1)

