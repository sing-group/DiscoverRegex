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

#ifndef _D_LINKEDHASHMAP_H_
#define	_D_LINKEDHASHMAP_H_

#include "common_dinamic_structures.h"
#include "dlinked_list.h"
#include "hashmap.h"

struct dlinkedhashmap_data;
typedef struct dlinkedhashmap_data dlinkedhashmap;

dlinkedhashmap *new_dlinkedhashmap();
int get_dlhElement(dlinkedhashmap *, char *, element *);
int add_dlhElement(dlinkedhashmap *,char *, element);
int add_dlhSortedElement(dlinkedhashmap *, char *, element, PFunction);
void free_dlh(dlinkedhashmap *, PFree);
//void free_dlh(dlinkedhashmap *lh, PFree fd, PFany fk);
long int get_dlhlinkedlistSize(dlinkedhashmap *);
int get_dlhmapSize(dlinkedhashmap *lh);
int dlh_iterateData(dlinkedhashmap *, PFunction, element);
int dlh_iterateKeys(dlinkedhashmap *, PFany, any_t);
int dlh_reverseIterateData(dlinkedhashmap *, PFunction, element);
int dlh_iterateDataTwo(dlinkedhashmap *, PFunction_3, element, element);
int dlh_reverseIterateDataUntil(dlinkedhashmap *, long int, PFunction, element);
int dlh_reverseRemoveData(dlinkedhashmap *, long int, PFree);
int dlh_dataHasCondition(dlinkedhashmap *, PFunction, element);
int dlh_reverseOperateRemoveData(dlinkedhashmap *lh, long int amount, PFunction f, element item);

#endif	

