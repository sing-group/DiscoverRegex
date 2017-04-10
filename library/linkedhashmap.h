/***************************************************************************
*
*   File    : linkedhashmap.h
*   Purpose : Implements a linkedhashmap library.
*
*
*   Author: David Ruano Ordás
*
*
*   Date    : March  14, 2010
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

#ifndef _LINKEDHASHMAP_H_
#define	_LINKEDHASHMAP_H_

#include "common_dinamic_structures.h"
#include "linked_list.h"
#include "hashmap.h"

struct linkedhashmap_data;
typedef struct linkedhashmap_data linkedhashmap;

linkedhashmap *newlinkedhashmap();
int add_lh_element(linkedhashmap *lh,char *key, element value);
int add_lh_ordered_element(linkedhashmap *lh, char *key, element value, PFunction comparator);
int get_lh_element(linkedhashmap *lh, char *key, element *value);
int get_lh_size(linkedhashmap *lh);
linklist *lh_getlist(linkedhashmap *lh);
map_t lh_gethashmap(linkedhashmap *lh);
void free_linkedhashmap(linkedhashmap *lh, PFree freedata, PFany freekey);

#endif	

