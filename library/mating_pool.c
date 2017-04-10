/****************************************************************
*
*   File    : mating_pool.c
*   Purpose : Implements a matting pool to store chromosomes.
*
*
*   Author  : David Ruano Ordás
*
*
*   Date    : February  03, 2016
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
#include <math.h>
#include "common_dinamic_structures.h"
#include "mating_pool.h"
#include "generic_vector.h"
#include "logger.h"

typedef void **matingPool;


struct sMating_data_t{
    matingPool matting;
    long int size;
    int inserted;
    int next;
};

sMatingPool *newSMatingPool(long int size){
    sMatingPool *toret = NULL;
    
    if(size != 0){
        if ((toret = (sMatingPool *) malloc(sizeof(sMatingPool))) == NULL){
            printlog(LOG_CRITICAL,"[MATING_POOL.newSMatingPool]","Cannot allocate enough memmory\n");
            return NULL;
        }
        toret->matting = (matingPool)malloc(size * (sizeof(void *)));
        toret->size = size;
        toret->inserted=0;
        toret->next=0;
        long int i=0;
        for(;i<size;i++) toret->matting[i] = NULL;
    }
    
    return toret;
}

int getMatingElement(sMatingPool *pool, long int pos, matingElement *elem){
    
    if(pool == NULL || pos >= pool->size){ 
        printlog(LOG_CRITICAL,"[MATING_POOL.getMatingElement]","Matting Pool undefined or position exeeds mating size\n");
        return MFAIL;
    }else{  
        *elem = pool->matting[pos];
        return MOK;
    }
}

/*
int addMatingElement(sMatingPool *pool, long int pos, matingElement elem){
    if(pool == NULL || pos >= pool->size){ 
        printlog(LOG_CRITICAL,"[MATING_POOL.addMatingElement]","Matting Pool undefined or position exeeds mating size\n");
        return MFAIL;
    }else{
        pool->matting[pos]=elem;
        pool->inserted++;
    }
    
    return MOK;
}
*/

int addMatingElementAtEnd(sMatingPool *pool, matingElement elem){
    if(pool == NULL || pool->next >= pool->size){ 
        printlog(LOG_CRITICAL,"[MATING_POOL.addMatingElement]","Matting Pool undefined or position exeeds mating size\n");
        return MFAIL;
    }else{
        pool->matting[pool->next]=elem;
        pool->next++;
        pool->inserted++;
    }
    
    return MOK;
}

long int getMatingPoolSize(sMatingPool *pool){
    if (pool == NULL){
        return 0;
    }else return pool->size;
}

void freeMattingPool(sMatingPool *pool,MFree f){
    if( pool != NULL ){
        long int i=0;
        for(;i<pool->size;i++){
            (f != NULL && pool->matting[i]!=NULL)?
                (f(pool->matting[i])):
                (0);
        }
        
        free(pool->matting);
        free(pool);
    }
}