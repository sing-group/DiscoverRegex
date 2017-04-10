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
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include "string_vector.h"
#include "hashmap.h"
#include "string_util.h"
#include "generic_vector.h"
#include "linked_vector.h"
#include "logger.h"

Svector *newStringvector(long int size){
    Svector *vect = NULL;
    
    
    if((vect = (Svector *)malloc(sizeof(Svector))) == NULL){
        printlog(LOG_CRITICAL,"[SVector.newStringvector]","Error allocating memory\n");
        return NULL;
    }
    
    (size > 0)?
        (vect->data = (char **)malloc(size * sizeof(char *))):
        (vect->data=NULL);
    vect->size = size;
    
    long int i=0;
    for(;i<vect->size;i++) vect->data[i]=NULL;

    return vect;
}

int addStringAt(Svector *vect, long int pos, char *item){
    if(vect == NULL || pos >= vect->size || vect->data == NULL || pos == -1){ 
        char **aux = NULL;
        if ((aux = (char **)realloc(vect->data,(vect->size+1)*sizeof(char *)))!=NULL){
            vect->data = aux;
            vect->data[vect->size]=item;
            vect->size++;
            return SOK;   
        }else{
            printlog(LOG_CRITICAL,"[SVector.addStringAt]","Cannot reallocate enought memory\n");
            return SFail;
        }
    }else vect->data[pos] = item;
    
    return SOK;
}

int addSortedString(Svector *vect, long int pos, /*PFString f,*/ char *item){
   
    if(vect == NULL || pos >= vect->size || vect->data == NULL || pos == -1){ 
        printlog(LOG_CRITICAL,"[SVector.addSortedString]","Svector not created\n");
        return SFail;
    }else{
        printf("ITEM: %s ",item);
        if( pos == 0 || !strcmp(item,"(") || !strcmp(item,"(?:") ||
            !strcmp(item,"|") || !strcmp(item,"^(") || !strcmp(item,")") || 
            !strcmp(item,"^(?:") ){
            vect->data[pos]=item;
            printf(" ES UN CARACTER ESPECIAL Y LO METO EN %ld\n",pos);
        }else{
            printf("EMPIEZO EN POS= %ld\n",pos);
            
            long int k=0;
            printf("[ ");
            for(;k<pos;k++) printf("%s ",vect->data[k]);
            printf("]\n");
            //vect->data[pos]=item;
            long int i=pos-1;
            long int changePos = pos;
            for(;i>=0;i--){
                if ( strcmp(vect->data[i],"(") && strcmp(vect->data[i],"(?:") &&
                     strcmp(vect->data[i],"|") && strcmp(vect->data[i],"^(") && 
                     strcmp(vect->data[i],")") && strcmp(vect->data[i],"^(?:") )
                {
                    if ( strcmp(vect->data[i],item) > 0){
                        printf("V[%ld]=%s\n",i,vect->data[i]);
                        printf("V[%ld]=%s\n",pos,vect->data[pos]);
                        changePos = i;
                    }
                }
            }
            for(k=pos;k>changePos;k--) vect->data[k]=vect->data[k-1];
            vect->data[changePos]=item;
        }
        
    }
    return SOK;
    
}

int addNewStringAtEnd(Svector *vect,char *item){
    if(vect == NULL){ 
        if((vect = (Svector *)malloc(sizeof(Svector))) == NULL){
            printlog(LOG_CRITICAL,"[SVector.addNewStringAtEnd]","Error allocating memory\n");
            return SFail;
        }
        vect->data = (char **)malloc(1 * sizeof(char *));
        vect->size = 1;
        vect->data[0]=item;
        return SOK;
    }else{
        char **aux = NULL;
        if ( (aux = (char **)realloc(vect->data,(vect->size+1)*sizeof(char *))) !=NULL ){
            vect->data = aux;
            vect->data[vect->size]=item;
            vect->size++;
            return SOK;
        }else{
            printlog(LOG_WARNING,"[SVector.addNewStringAtEnd]","Cannot realloc string vector");
            return SOK;
        }
    }
    return SOK;
}

int addNewStringAtBegin(Svector *vect,char *item){
    if(vect == NULL){ 
        if((vect = (Svector *)malloc(sizeof(Svector))) == NULL){
            printlog(LOG_CRITICAL,"[SVector.addNewStringAtBegin]","Error allocating memmory\n");
            return SFail;
        }
        vect->data = (char **)malloc(1 * sizeof(char *));
        vect->size = 1;
        vect->data[0] = item;
        return SOK;
    }else{
        char **aux = NULL;
        if ((aux = (char **)realloc(vect->data,(vect->size+1)*sizeof(char *)))!=NULL){
            long int i;
            vect->data=aux;
            for(i=vect->size-1;i>=0;i--) vect->data[i+1]=vect->data[i];
            vect->data[0]=item;
            vect->size++;
            return SOK;
        }else{
            printlog(LOG_CRITICAL,"[SVector.addNewStringAtBegin]","Cannot reallocate enought memory\n");
            return SFail;
        }
    }
    return SOK;
}

int replaceStringAt(Svector *vect, long int pos, char *item){
    if(vect == NULL || pos >= vect->size || vect->data == NULL){ 
        printlog(LOG_CRITICAL,"[SVector.addNewStringAtBegin]","Unable to replace element\n");
        return SFail;
    }else{
        if( vect->data[pos] != NULL) free(vect->data[pos]);
        vect->data[pos] = item;
        return SOK;
    }
}

void swapStringVector(Svector *vect, long int posA, long int posB){
    if(vect == NULL || posA >= vect->size || vect->data == NULL || posB > vect->size){ 
        printlog(LOG_CRITICAL,"[SVector.swapStringVector]","Unable to replace element\n");
    }else{
        char *aux = vect->data[posA];
        vect->data[posA]=vect->data[posB];
        vect->data[posB]=aux;
    }
}

int removeStringAt(Svector *vect, long int pos){
    if(vect == NULL || pos >= vect->size || vect->data == NULL || pos < 0){ 
        printlog(LOG_CRITICAL,"[SVector.addNewStringAtBegin]","Unable to remove element\n");
        return SFail;
    }else{
        if( vect->data[pos] != NULL){
            char **aux = (char **)malloc(sizeof(char *)*(vect->size-1));
            long int i;
            long int inserPos=0;
            for (i=0;i<vect->size;i++){
                (i != pos)?
                    (aux[inserPos++]=vect->data[i]):
                    (free(vect->data[i]));
            }
            free(vect->data);
            vect->data=aux;
            --vect->size;
        }
    }
    return SOK;
}

int removeStringInterval(Svector *vect, long int initPos, long int offset){
    //Svector *toret = NULL;

    long int endPos = offset + initPos -1;
    if(vect == NULL || initPos > vect->size || vect->data == NULL){ 
        printlog(LOG_CRITICAL,"[SVector.removeStringInterval]","Unable to remove element\n");
        return SFail;
    }else{
        if( vect->data[initPos] != NULL){
            (endPos >= vect->size)?(endPos=vect->size-1):(0);
            char **aux = (char **)malloc(sizeof(char *)*(vect->size-(endPos-initPos+1)));
            long int i;
            long int inserPos=0;
            for (i=0;i<(vect->size);i++){
                (i < initPos || i > (endPos) )?
                    (aux[inserPos++]=vect->data[i]):
                    (free(vect->data[i]));
            }
            free(vect->data);
            vect->data=aux;
            vect->size=vect->size-(endPos-initPos+1);
        }
    }
    return SOK;
}

char *getStringAt(Svector *vect, long int pos){
    if (vect == NULL || pos >= vect->size)
        return NULL;
    else return vect->data[pos];
}

void freeStringVector(Svector *vect){
    if(vect != NULL && vect->data != NULL){
        long int i;
        for(i=0;i<vect->size;i++){
            if (vect->data[i] != NULL) free((char *)vect->data[i]);
        }
        free(vect->data);
        free(vect);
    }
}

long int getStringVectorSize(Svector *vect){
    if(vect == NULL)
        return 0;
    else return vect->size;
}

Svector *mergeSVectors(Svector *a, Svector *b, long int insertPos){
    
    if ( getStringVectorSize(a) == 0 && 
         getStringVectorSize(b) == 0 ) return NULL;
    
    
    Svector *new = newStringvector(a->size+b->size);
        
    long int i;
    char *aux = NULL;
    for(i=0;i<insertPos;i++){
        if( (aux = a->data[i]) != NULL )
            new->data[i]=a->data[i];
    }
    
    for(i=0;i<b->size;i++){ 
        if( (aux = b->data[i]) != NULL )
            new->data[insertPos+i]=b->data[i];
    }
    
    for(i=insertPos+i+1;i<getStringVectorSize(a);i++){
        if( (aux = a->data[i]) != NULL )
            new->data[i]=a->data[i];
    }
        
    free(a->data);
    free(a);
    free(b->data);
    free(b);
    
    return new;
}

void shuffleStringVector(Svector *vector){
    
    long int size = getStringVectorSize(vector);
    long int i;
    //srand( time(NULL) );
    
    for (i=size-1; i > 1;i--){
        long int randPos = rand() % (i+1);
        
        //SWAP
        char *aux = vector->data[i];
        vector->data[i]=vector->data[randPos];
        vector->data[randPos]=aux;
    }
}

void shuffleStringVectorAt(Svector *vector, long int pos){
    
    long int size = getStringVectorSize(vector);
    long int i;
    //srand( time(NULL) );
    
    for (i=size-1; i > pos+1;i--){
        long int randPos = rand() % (i+1);
        
        //SWAP
        char *aux = vector->data[i];
        vector->data[i]=vector->data[randPos];
        vector->data[randPos]=aux;
    }
}

Svector *cloneStringVector(Svector *vector){
   
    Svector *toret = newStringvector(vector->size);
    long int i=0;
    
    for(;i<vector->size;i++)
        toret->data[i]=strdup(vector->data[i]);

    return toret;    
}

/**
 * Create a new SVector by joining two Svectors. SVector created must be freed.
 * @param v1 First vector
 * @param f1 Comparative funtion associated to first vector. Element is inserted if f1 returns 1.
 * @param e1 Element inserted as parameter to the function f1.
 * @param v2 Second vector
 * @param f2 Comparative funtion associated to second vector. Element is inserted if f2 returns 1.
 * @param e2 Element inserted as parameter to the function f2.
 * @return the new Vector.
 */
Svector *joinStringVectors(Svector *v1, PFelem f1, SElem e1, 
                           Svector *v2, PFelem f2, SElem e2){
    
    long int i;
    long int insertPos = 0;
    long int countInsertionsV1 = 0;
    long int countInsertionsV2 = 0;
    
    for(i=0;i<v1->size;i++) if( f1 == NULL || f1(e1,v1->data[i]) ) countInsertionsV1++;
    
    for(i=0;i<v2->size;i++) if( f2 == NULL || f2(e2,v2->data[i]) ) countInsertionsV2++;
    
    Svector *toret = newStringvector(countInsertionsV1 + countInsertionsV2);
    
    for(i=0;i<v1->size;i++) if( f1 == NULL || f1(e1,v1->data[i]) ) toret->data[insertPos++] = strdup(v1->data[i]);
    
    for(i=0;i<v2->size;i++) if( f2 == NULL || f2(e2,v2->data[i]) ) toret->data[insertPos++] = strdup(v2->data[i]);
    
    return toret;
}

Svector *concatStringVectors(Svector *v1, Svector *v2, long int initPos, long int endPos){
    
    long int i;
    long int begin;
    long int end;
    long int insertPos = 0;
    Svector *toret = NULL;
    
    if (initPos > endPos) return toret;
    
    if (initPos == endPos) return cloneStringVector(v1);
    
    begin = (initPos > 0)?(initPos):(0);
    end = (endPos < v2->size)?(endPos):(v2->size);
    
    if ( (toret = newStringvector( v1->size + (end-begin) )) !=NULL ){
        for(i=0;i<v1->size;i++) toret->data[insertPos++] = strdup(v1->data[i]);
        while(begin < end) toret->data[insertPos++] = strdup(v2->data[begin++]);
    }

    return toret;
}

Svector *concatStringVectorsAddEnd(Svector *v1, Svector *v2, long int initPos, long int endPos, char *str){
    
    long int i;
    long int begin;
    long int end;
    long int insertPos = 0;
    Svector *toret = NULL;
    
    if (initPos > endPos || str == NULL) return toret;
    
    if (initPos == endPos){ 
        toret = newStringvector(v1->size+1);
        for(i=0;i<v1->size;i++) toret->data[i]=strdup(v1->data[i]);
        toret->data[toret->size-1]=str;
        return toret;
    }
    
    begin = (initPos > 0)?(initPos):(0);
    end = (endPos < v2->size)?(endPos):(v2->size);
    
    if ( (toret = newStringvector( v1->size + (end - begin + 1) )) !=NULL ){
        for(i=0;i<v1->size;i++) toret->data[insertPos++] = strdup(v1->data[i]);
        while(begin < end) toret->data[insertPos++] = strdup(v2->data[begin++]);
    }
    
    toret->data[toret->size-1] = str;
    
    return toret;
}

Svector *concatStringVectorsAddBegin(Svector *v1, Svector *v2, long int initPos, long int endPos, char *str){
    long int i;
    long int begin;
    long int end;
    Svector *toret = NULL;
    
    if (initPos > endPos || str == NULL) return toret;
    
    if (initPos == endPos){ 
        toret = newStringvector(v1->size+1);
        toret->data[0]=str;
        for(i=0;i<v1->size;i++) toret->data[i+1]=strdup(v1->data[i]);
        return toret;
    }
    
    begin = (initPos > 0)?(initPos):(0);
    end = (endPos < v2->size)?(endPos):(v2->size);
    
    if ( (toret = newStringvector( v1->size + (end - begin + 1) )) !=NULL ){
        toret->data[0]= str;
        for(i=0;i<v1->size;i++) toret->data[i+1] = strdup(v1->data[i]);
        while(begin < end) toret->data[++i] = strdup(v2->data[begin++]);
    }    
    return toret;
}

Svector *concatStringVectorsAddBeginEnd(Svector *v1, Svector *v2, long int initPos, long int endPos, char *str_beg, char *str_end){
    long int i;
    long int begin;
    long int end;
    Svector *toret = NULL;
    
    if (initPos > endPos || str_beg == NULL || str_end == NULL) return toret;
    
    if (initPos == endPos){ 
        toret = newStringvector(v1->size+2);
        toret->data[0]=str_beg;
        for(i=0;i<v1->size;i++) toret->data[i+1]=strdup(v1->data[i]);
        toret->data[toret->size-1]=str_end;
        return toret;
    }
    
    begin = (initPos > 0)?(initPos):(0);
    end = (endPos < v2->size)?(endPos):(v2->size);
    
    if ( (toret = newStringvector( v1->size + (end - begin + 2) )) !=NULL ){
        toret->data[0]= str_beg;
        for(i=0;i<v1->size;i++) toret->data[i+1] = strdup(v1->data[i]);
        while(begin < end) toret->data[++i] = strdup(v2->data[begin++]);
        toret->data[toret->size-1]=str_end;
    }    
    return toret;
}

void fisherYatesShuffle(Svector *vector, PFString f){
    long int size = getStringVectorSize(vector);
    long int i = size -1 ;
    long int pos = 0;
    //srand( time(NULL) );

    while( i > 1 ){
        while ( i > 1 && f(getStringAt(vector,i)) ) i--;
        //printf("[1]- i %ld\n\n",i);
        do{
            pos = rand() % i;
        }while( f(getStringAt(vector,pos)) && pos!=0 );
        if( !f(getStringAt(vector,i)) && !f(getStringAt(vector,pos) )){
            char *aux = vector->data[i];
            vector->data[i]=vector->data[pos];
            vector->data[pos]=aux;
        }
        i--;
    }
}

/**
 * Obtain a slice of StringVector according to initPos and lenght.
 * @param vector SVector to be spliced.
 * @param initPos source position.
 * @param lenght size of SVector obtained.
 * @return the new Vector.
 */
Svector *spliceStringVector(Svector *vector, long int initPos, long int lenght){
    Svector *toret = NULL;
    
    if (vector != NULL && vector->data != NULL && initPos < vector->size && lenght > 0){
        (initPos == FROM_BEGIN)?(initPos=0):(0);
        long int final= ((initPos+lenght-1) >= vector->size)?(vector->size-initPos):(lenght);
        toret = newStringvector(final);
        long int i=0;
        long int pos=initPos;
        long int toretPos = 0;
        
        for(;i<final;i++) toret->data[toretPos++]=strdup(vector->data[pos++]);
    }else toret = cloneStringVector(vector);
    
    return toret;
}

void stringVectorBubbleSort2(Svector *vector){
    long int i;
    long int j;
    
    if (vector != NULL && vector->data != NULL && vector->size > 0){
        for(i=1;i < vector->size; i++){
            for(j=0;j <vector->size-1;j++){
                if(strcmp(vector->data[j],vector->data[j+1]) > 0){
                    char *temp = vector->data[j+1];
                    vector->data[j+1]=vector->data[j];
                    vector->data[j]=temp;
                }
            }
        }
    }else printlog(LOG_CRITICAL,"[SVector.StringVectorBubbleSort]","Unable to sort vector\n");
}

/**
 * Obtain a sorted slice of StringVector according to initPos and lenght.
 * @param vector SVector to be spliced.
 * @param initPos source position.
 * @param lenght size of SVector obtained.
 * @return the new Vector.
 */
Svector *spliceSortedStringVector(Svector *vector, long int initPos, long int lenght){
    Svector *toret = NULL;
    
    if (vector != NULL && vector->data != NULL && initPos < vector->size && lenght > 0){
        (initPos == FROM_BEGIN)?(initPos=0):(0);
        long int final= ((initPos+lenght-1) >= vector->size)?(vector->size-initPos):(lenght);
        toret = newStringvector(final);
        long int i=0;
        long int pos=initPos;
        long int toretPos = 0;
        
        for(;i<final;i++) toret->data[toretPos++]=strdup(vector->data[pos++]);
    }else toret = cloneStringVector(vector);
    
    return toret;
}

char *stringVectorToString(Svector *vector){
    char *toret = NULL;
    if (vector!= NULL && vector->data!=NULL){
        long int i=0;
        for(;i<vector->size;i++) toret = appendstr(toret,getStringAt(vector,i));
    }
    
    return toret;
}

void printStringVector(Svector *vector){
    printf("[");
    long int i=0;
    if(vector!= NULL){
        for(;i<getStringVectorSize(vector);i++){
            printf(" %s",getStringAt(vector,i));
        }
    }else printf(" ] <=> SIZE IS 0");
    printf(" ] <=> SIZE IS %ld\n\n",getStringVectorSize(vector));

}