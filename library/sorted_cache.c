/****************************************************************
*
*   File    : Concurrent cache.c
*   Purpose : Implements a dinamic cache using hashmap and linkedlist.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap.h"
#include "linked_list.h"
#include "sorted_cache.h"
#include "logger.h"
#include <pthread.h>

struct cache_data_t{
    linklist *list;
    map_t hashmap;
    int max_size;
    int size;
};

cache_data *newCache(long int max_size){

    cache_data *newcache = NULL;
    
    if (max_size == 0) return NULL;
    
    if( ( newcache = (cache_data *) malloc(sizeof(cache_data)) ) == NULL ){
        printlog(LOG_CRITICAL,"[CACHE.newCache]","Not enought memmory to allocate cache data structure\n");
        return newcache;
    }
    
    if( (newcache->hashmap=hashmap_new()) == NULL) {
        printlog(LOG_CRITICAL,"[CACHE.newCache]","Not enought memmory to allocate hashmap inside cache structure\n");
        free(newcache);
        return NULL;    
    }
    if ( (newcache->list=newlinkedlist())==NULL ){
        printlog(LOG_CRITICAL,"[CACHE.newCache]","Not enought memmory to allocate linklist inside cache structure\n");
        hashmap_free(newcache->hashmap);
        free(newcache);
        return NULL;    
    }
    
    newcache->max_size=max_size;
    
    newcache->size=0;

    return newcache;
}

// PRIVATE FUNTION
c_element *newCacheElement(char *key, element elem){
    
    c_element *new = NULL;
    if( (new=(c_element *)malloc(sizeof(c_element))) == NULL ){
        printlog(LOG_CRITICAL,"[CACHE.newCacheElement]","Not enought memory to create new cache element\n");
        return new;
    }

    new->key=key;
    new->data=elem;

    return new;
}

int push_cache(cache_data *cache, char *key, PFdata f, element elem, PFunction sort){

    c_element *new_elem = NULL;
    int result;
    
    if(cache == NULL || cache->max_size==0) return CACHE_UNDEF;
    printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_1]\n");
    if( cache->hashmap != NULL && 
        hashmap_get(cache->hashmap,key,(any_t *)&new_elem)==MAP_OK){
        return CACHE_ELEM_EXIST;
    }
    printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2]\n");
    if( cache->max_size != CACHE_INFINITE && cache->size >= cache->max_size) {
        c_element *remove = NULL;
        printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1]\n");
        if ( getlast(cache->list,(element *)&remove) == NODE_OK && 
             remove != NULL && sort(remove,elem)  ) 
        {
            printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_1]\n");
            removelast(cache->list,(element *)&remove);       
            printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_2]\n");
            hashmap_remove(cache->hashmap,((c_element *)remove)->key);
            printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_3]\n");
            if(remove != NULL){
               printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_3_1]\n");
               if(remove->data!= NULL) f(remove->data);
               printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_3_2]\n");
               if(remove->key != NULL) free(remove->key);
               printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_3_3]\n");
               free(remove);
               cache->size--;
           }else return CACHE_FAIL;
            printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_1_4]\n");
        }
        printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_2_2]\n");
    }
    printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_3]\n");
    
    if((new_elem=newCacheElement(key,elem))!=NULL){
        printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_3_1]\n");
        addorder(cache->list,new_elem,sort);
        printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_3_2]\n");
        hashmap_put(cache->hashmap,key,new_elem->data);
        printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_3_3]\n");
        cache->size++;
        result= CACHE_OK;
    } else result= CACHE_FAIL;
    
    printlog(LOG_DEBUG,"[CACHE.push_cache]","[PUSH_CACHE_4]\n");
    return result;
}

int peek_cache(cache_data *cache, char *key, element *elem){

    element result;
    int toret;

    if(cache->max_size==0 || hashmap_get(cache->hashmap,key,(any_t *)&result)!=MAP_OK){        
        *elem=NULL;
        toret=CACHE_ELEM_MISSING;        
    }
    else{
        *elem=result;
        toret=CACHE_ELEM_FOUND;
    }
    return toret;
}

void set_cache_size(cache_data *cache,int size){
    cache->max_size=size;
}

int get_cache_size(cache_data *cache){
    int size = 0;
    
    if(cache!=NULL) size=cache->max_size;
    
    return size;
}

void free_cache(cache_data *cache, PFree f){
        
    if(cache!=NULL){
        if (cache->list != NULL) freelist(cache->list,f);
        if (cache->hashmap != NULL){ 
            hashmap_free(cache->hashmap);
        }
        free(cache);
    }
    cache=NULL;
}
