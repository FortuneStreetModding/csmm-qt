/******************************************************************************
 *  xml.h                                                                     *
 *  Part of Benzin                                                            *
 *  Handles some XML stuff.                                                   *
 *  Copyright (C)2009 SquidMan (Alex Marshall)        <SquidMan72@gmail.com>  *
 *  Copyright (C)2009 megazig  (Stephen Simpson)       <megazig@hotmail.com>  *
 *  Copyright (C)2009 Matt_P   (Matthew Parlane)                              *
 *  Copyright (C)2009 comex                                                   *
 *  Copyright (C)2009 booto                                                   *
 *  All Rights Reserved, HACKERCHANNEL.                                       *
 ******************************************************************************/

#ifndef _XML_H_
#define _XML_H_

char *                    /* O - Buffer */
get_value(mxml_node_t *node,        /* I - Node to get */
      void        *buffer,        /* I - Buffer */
      int         buflen);        /* I - Size of buffer */

const char *whitespace_cb(mxml_node_t *node, int where);

#endif //_XML_H_
