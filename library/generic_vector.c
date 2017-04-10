/****************************************************************
*
*   File    : vector.c
*   Purpose : Implements a linked list for storing data.
*
*
*   Author  : David Ruano Ordás
*
*
*   Date    : October  03, 2014
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
#include <time.h>
#include <string.h>
#include "generic_vector.h"
#include "logger.h"

struct g_vector{
    void **data;
    long int size;
    //int next;
    long int inserted;
};

Gvector *newGenericVector(long int size){
    Gvector *vect = NULL;
    
    if((vect = (Gvector *)malloc(sizeof(Gvector))) == NULL){
        printlog(LOG_CRITICAL,"[GVector.newGenericVector]","Error allocating memmory\n");
        return NULL;
    }
    vect->data = (void **)malloc(size * sizeof(void *));
    vect->size = size;
    vect->inserted = 0;
    
    int i=0;
    for(;i<vect->size;i++) vect->data[i]=NULL;

    return vect;
}

int addElementAt(Gvector *vect, long int pos, void *item){
    
    if(vect == NULL || pos >= vect->size || vect->data == NULL || pos == -1){ 
        printlog(LOG_CRITICAL,"[GVector.addElementAt]","Position %ld exceeds max vector size of %ld\n",pos,vect->size);
        return GFail;
    }else{
        vect->data[pos] = item;
        vect->inserted++;
        return GOK;
    }
}

int addNewElementAtEnd(Gvector *vect, gElement item){
    if(vect == NULL){ 
        if((vect = (Gvector *)malloc(sizeof(Gvector))) == NULL){
            printlog(LOG_CRITICAL,"[GVector.addNewElementAtEnd]", "Error allocating memmory\n");
            return GFail;
        }
        vect->data = (void **)malloc(sizeof(char *));
        vect->size = 1;
        vect->inserted = 1;
        vect->data[0] = item;
        //vect->next = vect->size;
        return GOK;
    }else{
        void **aux = NULL;
        if ((aux = (void **)realloc(vect->data,(vect->size+1)*sizeof(char *)))!=NULL){
            vect->data = aux;
            vect->data[vect->size]=item;
            vect->size++;
            vect->inserted++;
            //vect->next = vect->size;
            return GOK;
        }else{
            printlog(LOG_CRITICAL,"[GVector.addNewElementAtEnd]","Cannot reallocate enought memory\n");
            return GFail;
        }
    }
    return GOK;
}

int addElement(Gvector *vect, gElement item){
    if(vect == NULL){ 
        if ( (vect = newGenericVector(1)) != NULL){
            addElementAt(vect,0,item);
            return GOK;
        }else return GFail;
    }
    
    if ( vect->inserted == vect->size )
        return addNewElementAtEnd(vect,item);
    else{
        int i = 0;
        while(vect->data[i] != NULL) i++;
        if(i < vect->size && vect->data[i]==NULL) vect->data[i]=item;
        vect->inserted++;
    }
    
    return GOK;
}

int getElementAt(Gvector *vect, long int pos, gElement *elem){
    if (vect == NULL || pos >= vect->size){
        printlog(LOG_CRITICAL,"[GVector.getElementAt]","Position exceeds GVector index\n");
        return GFail;
    }else{
        *elem = vect->data[pos];
        return GOK;
    }
}

void freeGenericVector(Gvector *vect, GFree f){
    if(vect != NULL){
        int i=0;
        for(;i<vect->size;i++){
            (vect->data!=NULL && f != NULL)?(f(vect->data[i])):(0);
        }
        free(vect->data);
        free(vect);
        vect = NULL;
    }       
}

void removeGenericElementAt(Gvector *vect, long int pos, GFree f){
    if(vect == NULL)
        printlog(LOG_CRITICAL,"[GVector.removeGenericElementAt]","Cannot remove element, Vector not created\n");
    else{
        if(pos >= vect->size)
            printlog(LOG_CRITICAL,"[GVector.removeGenericElementAt]","Element position exceeds GVector index\n");
        else{
            if(vect->data[pos]!= NULL && f != NULL){
                int i;
                void **aux = NULL;
                f(vect->data[pos]);
                for(i=pos;i<(vect->size-1);i++) vect->data[i]=vect->data[i+1];
                if((aux = (void **)realloc(vect->data,(vect->size-1)*sizeof(char *))) != NULL)
                    vect->data=aux;
                vect->inserted--;
                vect->size--;
            }
        }
    }
}

void removeEmptyElements(Gvector *vect){
    if (vect != NULL && vect->inserted < vect->size){
        int i=0;
        for(;i<(vect->size-1);i++){
            if(vect->data[i]==NULL){
                int j=i+1;
                while(j < vect->size && vect->data[j] == NULL) j++;
                if(j < vect->size){
                    vect->data[i]=vect->data[j];
                    vect->data[j]= NULL;
                }else break;
            }
        }
        void **aux = NULL;
        if( (aux = realloc(vect->data,(vect->size-(vect->size-vect->inserted))*sizeof(char *))) != NULL ){
            vect->data=aux;
            vect->size-=(vect->size-vect->inserted);
        }else printlog(LOG_CRITICAL,"[GVector.removeEmptyElements]","Cannot reallocate memmory\n");
    }
}

long int getGenericVectorSize(Gvector *vect){
    if(vect == NULL)
        return 0;
    else return vect->size;
}

void shuffleGenericVector(Gvector *vector){
    removeEmptyElements(vector);
    
    int size = vector->size;
    int i;
    //srand( time(NULL) );
    for (i=size-1; i > 1;i--){
        int randPos = rand() % (i+1);
        //SWAP
        gElement *tempElem = vector->data[i];
        gElement *randElem = vector->data[randPos];
        vector->data[i]=randElem;
        vector->data[randPos]=tempElem;
    }
}

Gvector *getGenericVectorSlice(Gvector *vector, long int init, long int end){
    removeEmptyElements(vector);
    if( ( vector != NULL && vector->data != NULL && init < vector->size)
          && (init < end || init==FROM_BEGIN || end==TO_END) )
    {
        (init == FROM_BEGIN)?(init=0):(0);
        (end>=vector->size || end==TO_END)?(end=vector->size-1):(0);
        
        Gvector *toret = newGenericVector((end-init)+1);
        
        int i=init;
        int toretPos = 0;
        for(;i<=end;i++) toret->data[toretPos++]=vector->data[i];
        toret->inserted=(end-init)+1;
        return toret;
    }
    return vector;
}

Gvector *spliceGenericVector(Gvector *vector, long int init, long int lenght){
    if( vector != NULL && vector->data != NULL && init < vector->size){
        
        (init == FROM_BEGIN)?(init=0):(0);
        int final= ((init+lenght-1) >= vector->size)?(vector->size-init):(lenght);
        Gvector *toret = newGenericVector(lenght);
        
        int i=0;
        int pos=init;
        int toretPos = 0;
        for(;i<final;i++) toret->data[toretPos++] = vector->data[pos++];
        toret->inserted=lenght;
        return toret;
    }
    return vector;
}

Gvector *joinGenericVector(Gvector *v1,Gvector *v2){
    int i;
    int insert_pos = 0;
    
    Gvector *toret = NULL;
    
    toret = newGenericVector(v1->inserted+v2->inserted);
    
    for (i=0;i<v1->size;i++) if(v1->data[i]!=NULL) toret->data[insert_pos++] = v1->data[i];
    
    for (i=0;i<v2->size;i++) if(v2->data[i]!=NULL) toret->data[insert_pos++] = v2->data[i];
    
    toret->inserted=insert_pos;
    
    return toret;
}