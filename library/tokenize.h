/***************************************************************************
*
*   File    : tokenize.h
*   Purpose : Header file need for tokenize eml words
*
*
*   Author  : David Ruano Ordás
*   Date    : November  02, 2010
*
*****************************************************************************
*   LICENSING
*****************************************************************************
*
* WB4Spam: An ANSI C is an open source, highly extensible, high performance and
* multithread spam filtering platform. It takes concepts from SpamAssassin project
* improving distinct issues.
*
* Copyright (C) 2010, by Sing Research Group (http://sing.ei.uvigo.es)
*
* This file is part of WireBrush for Spam project.
*
* Wirebrush for Spam is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* Wirebrush for Spam is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef __TOKENIZE_H__
#define	__TOKENIZE_H__

/*---------------------------------------------------------------------------
   	       							     INCLUDES
 ---------------------------------------------------------------------------*/

#include "hashmap.h"

/*---------------------------------------------------------------------------
                                                                       MACROS
 ---------------------------------------------------------------------------*/

#define INIT_TOKEN ""
#define COUNT_TOKEN "#"

/*---------------------------------------------------------------------------
                                                                    FUNCTIONS
 ---------------------------------------------------------------------------*/


/**
 * Tokenizes a message and stores each different token in a hashmap with its
 * number of duplicates.
 */
map_t tokenize(char *email, map_t stopwords);

/**
 * Tokens hashmap liberations.
 */
void free_tokenize(map_t tokens);

#endif	/* TOKENIZE_H */

