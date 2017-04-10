/****************************************************************
*
*   File    : linkedhashmap.c
*   Purpose : Implements a linkedhashmap library.
*
*
*   Author  : David Ruano Ordás
*
*
*   Date    : March  14, 2011
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

#include <stdio.h>
#include <string.h>
#include "hashmap.h"
#include "linked_list.h"
#include "linkedhashmap.h"
#include "logger.h"

struct linkedhashmap_data{
    linklist *list;
    map_t hashmap;
    int size;
};

linkedhashmap *newlinkedhashmap(){
    linkedhashmap *aux = (linkedhashmap *)malloc(sizeof(linkedhashmap));
    if(aux==NULL){ 
        printlog(LOG_CRITICAL,"[LINKEDHASHMAP.newlinkedhashmap]","Not enought memory\n");
        return NULL;
    }
    aux->hashmap=hashmap_new();
    aux->list=newlinkedlist();
    aux->size=0;
    return aux;
}

int get_lh_element(linkedhashmap *lh, char *key, element *value){
    element result;
    if(lh==NULL){
        printlog(LOG_CRITICAL,"[LINKEDHASHMAP.get_lh_element]","LinkedHashmap not initiallized\n");
        return LH_MISSING;
    }
    if(hashmap_get(lh->hashmap,key,(any_t *)&result)!=MAP_MISSING){
        *value=result;
        return LH_EXIST;
    }
    *value=NULL;
    return LH_MISSING;
    
}

int add_lh_element(linkedhashmap *lh,char *key, element value){
    
    if(lh==NULL){
        printlog(LOG_CRITICAL,"[LINKEDHASHMAP.add_lh_element]","LinkedHashmap not initiallized\n");
        return LH_ERROR;
    }
    element *result;
    if(hashmap_get(lh->hashmap,key,(any_t *)&result)==MAP_MISSING){
        lh->size++;
        addbeginlist(lh->list,value);
        hashmap_put(lh->hashmap,key,value);
        return LH_OK;
    } return LH_EXIST;
}

int add_lh_ordered_element(linkedhashmap *lh, char *key, element value, PFunction comparator){
    if(lh == NULL){
        printlog(LOG_CRITICAL,"[LINKEDHASHMAP.add_lh_ordered_element]","LinkedHashmap not initiallized\n");
        return LH_ERROR;
    }
    element *result;
    if(hashmap_get(lh->hashmap,key,(any_t *)&result)==MAP_MISSING){
        lh->size++;
        addorder(lh->list,value,comparator);
        hashmap_put(lh->hashmap,key,value);
        return LH_OK;
    } return LH_EXIST;
}

void free_linkedhashmap(linkedhashmap *lh, PFree freedata, PFany freekey){

    (freedata != NULL)?(freelist(lh->list,freedata)):(0);
    (freekey != NULL)?(hashmap_iterate_keys(lh->hashmap,freekey,NULL)):(0);
    hashmap_free(lh->hashmap);
    free(lh);
}

linklist *lh_getlist(linkedhashmap *lh){
    return lh->list;
}

map_t lh_gethashmap(linkedhashmap *lh){
    return lh->hashmap;
}

int free_lh_data(element elem){
    free(elem);
    return NODE_OK;
}

int free_data(element data){
    free(data);
    return NODE_OK;
}

int compare_element(element a, element b){
    int *a1=(int *)a;
    int *b1=(int *)b;

    if(*a1<*b1)
        return -1;
    else
        if(*a1>*b1)
            return 1;
        else
            return 0;
}

int get_lh_size(linkedhashmap *lh){
    if (lh == NULL){
        return 0;
    }else return getlengthlist(lh_getlist(lh));
}
