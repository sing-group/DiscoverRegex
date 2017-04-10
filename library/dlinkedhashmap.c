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
#include <pthread.h>
#include "hashmap.h"
#include "dlinkedhashmap.h"
#include "logger.h"
#include "common_dinamic_structures.h"

// INTERNAL LIST DEFINITION
typedef struct content_data_t{
    char *keyPointer;
    element value;
} content_t;

content_t *createContent(char *key, element data){
    content_t *toret = NULL;
    if ( (toret = malloc(sizeof(content_t))) != NULL  ){
        toret->keyPointer = key;
        toret->value = data;
    }
    
    return toret;
}

element *getContent(content_t *content){
    if (content == NULL)
        return NULL;
    else return content->value;
}

typedef struct lh_linkedlist{
    dnode *header;
    dnode *tail;
    int lenght;
} lhlinkedlist;

lhlinkedlist *newlhlinkedlist(){
    lhlinkedlist *newlist = (lhlinkedlist *) malloc(sizeof(lhlinkedlist));
    if(newlist==NULL){
        printlog(LOG_CRITICAL,"[LINKED_LIST.newlinkedlist]","Not enought memory");
        return NULL;
    }
    
    newlist->header=NULL;
    newlist->lenght=0;
    newlist->tail=NULL;
    
    return newlist;
}

struct dlinkedhashmap_data{
    lhlinkedlist *list;
    map_t hashmap;
    int size;
};

dlinkedhashmap *new_dlinkedhashmap(){
    dlinkedhashmap *aux = (dlinkedhashmap *)malloc(sizeof(dlinkedhashmap));
    if(aux==NULL){ 
        printlog(LOG_CRITICAL,"[LINKEDHASHMAP.newlinkedhashmap]","Not enought memory\n");
        return NULL;
    }
    
    aux->hashmap=hashmap_new();
    aux->list=newlhlinkedlist();
    aux->size=0;
    return aux;
}

int get_dlhElement(dlinkedhashmap *lh, char *key, element *value){
    element *result;
    if(lh==NULL){
        printlog(LOG_CRITICAL,"[LINKEDHASHMAP.get_lh_element]","LinkedHashmap not initiallized\n");
        return LH_MISSING;
    }
    
    if(hashmap_get(lh->hashmap,key,(any_t *)&result)!=MAP_MISSING){
        *value=((content_t *)result)->keyPointer;
        return LH_EXIST;
    }
    *value=NULL;
    return LH_MISSING;   
}

int add_dlhElement(dlinkedhashmap *lh,char *key, element value){
    
    if(lh==NULL || lh->list == NULL){
        printlog(LOG_CRITICAL,"[dlinkedhashmap.add_lh_element]","Linkedhashmap not initiallized\n");
        return LH_ERROR;
    }
    
    element result = NULL;
    if(hashmap_get(lh->hashmap,key,(any_t *)&result) == MAP_MISSING){
            //
            dnode *new = NULL;
            lhlinkedlist *list = lh->list;
            int toret= LH_OK;

            if ( (new = (dnode *) malloc(sizeof(dnode))) ) {        
                list->lenght++;

                new->value= createContent(key,value); 
                
                if (list->header == NULL){
                    new->next=NULL;
                    new->prev=NULL;
                    list->header=new;
                    list->tail=new;
                }
                else{
                    new->next=list->header;
                    list->header->prev=new;
                    list->header=new;
                }
            }else {
                printlog(LOG_CRITICAL,"[dlinkedhashmap.addDlhElement]","Not enought memory");
                toret=LH_ERROR;
            }
            //
            hashmap_put(lh->hashmap,key,new);
            lh->size++;
            return toret;
    } return LH_EXIST;
}

int add_dlhSortedElement(dlinkedhashmap *lh, char *key, element value, PFunction f){
    if(lh == NULL || lh->list == NULL){
        printlog(LOG_CRITICAL,"[dlinkedhashmap.add_lh_ordered_element]","Linkedhashmap not initiallized\n");
        return LH_ERROR;
    }
    
    element *new = NULL;
    if(hashmap_get(lh->hashmap,key,(any_t *)&new)==MAP_MISSING){
        //
        dnode *new = NULL;
        if ((new = (dnode *) malloc(sizeof(dnode)))) {
            lhlinkedlist *list = lh->list;
            list->lenght++;
            new->value= createContent(key,value); 
            //SI LA LISTA ESTA VACIA
            if (list->header == NULL){
                new->next=NULL;
                new->prev=NULL;
                list->header=new;
                list->tail=new;
            }
            else{
                //SI HEADER ES < QUE EL NUEVO -> INSERTO AL PRINCIPIO
                //f = -1 a<b
                //f = 0 a=b
                //f = 1 a>b
                if(f( getContent(list->header->value), value) < 0){ //
                    new->next=list->header;
                    new->prev = NULL;
                    list->header->prev=new;
                    list->header=new;
                }
                else{
                    dnode *pos=list->header;
                    dnode *ant=list->header;

                    while (pos->next!=NULL && f(getContent(pos->value), value) >= 0){
                        ant=pos;
                        pos=pos->next;
                    }
                    //INSERTO EL ULTIMO
                    if( f(getContent(pos->value), value) >= 0){
                        new->next = NULL;
                        new->prev=list->tail;
                        list->tail->next=new;
                        list->tail=new;
                    }
                    else{ //INSERTO EN EL MEDIO
                        ant->next=new;
                        new->prev=ant;
                        new->next=pos;
                        pos->prev=new;
                    }
                }
            }       
        }else printlog(LOG_CRITICAL,"[dlinkedhashmap.addDlhOrderedElement]", "Not enought memory");
        //
        hashmap_put(lh->hashmap,key,new);
        lh->size++;
        return LH_OK;
    } 
    return LH_EXIST;
}

int freelhKeys(any_t null, any_t key){
    (key != NULL)?(free(key)):(0);
    return MAP_OK;
}

void free_dlh(dlinkedhashmap *lh, PFree fd){

    if(fd != NULL && lh->list!=NULL){
        int i=0;
        lhlinkedlist *list = lh->list;
        dnode *aux=list->header;
        dnode *temp;

        for(;i<list->lenght;i++){
            if(aux!=NULL){
                content_t *data=(content_t *)(aux->value);
                (data != NULL && fd != NULL)?(fd(data->value)):(0);
                free(data->keyPointer);
                free(data);
                if(aux->next != NULL){
                    temp=aux;
                    aux=aux->next;
                    free(temp);
                }else free(aux);
            }
        }
        list->lenght=0;
        free(list);
        list=NULL;
    }
    //hashmap_iterate_keys(lh->hashmap,&freelhKeys,NULL);
    hashmap_free(lh->hashmap);
    free(lh);
    lh = NULL;
}

long int get_dlhlinkedlistSize(dlinkedhashmap *lh){
    if (lh == NULL){
        return 0;
    }else return lh->list->lenght;
}

int get_dlhmapSize(dlinkedhashmap *lh){
    if (lh == NULL){
        return 0;
    }else return hashmap_length(lh->hashmap);
}

int dlh_iterateData(dlinkedhashmap *lh, PFunction f, element item){

    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_MISSING;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->header;
    int i=0;
    /* Linear probing */

    for(;i<list->lenght;i++){
        content_t *data = (content_t *)(aux->value);
        int status = f(item, data->value);
        
        if (status != NODE_OK) return status;        
        if (aux->next!=NULL) aux=aux->next;
    }
    return NODE_OK;
}

int dlh_iterateDataTwo(dlinkedhashmap *lh, PFunction_3 f, element item, element item2){

    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_MISSING;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->header;
    int i=0;
    /* Linear probing */

    for(;i<list->lenght;i++){
        content_t *data = (content_t *)(aux->value);
        int status = f(item2, item, data->value);
        
        if (status != NODE_OK) return status;        
        if (aux->next!=NULL) aux=aux->next;
    }
    return NODE_OK;
}

int dlh_iterateKeys(dlinkedhashmap *lh, PFany f, any_t item){

    if (lh == NULL || lh->hashmap == NULL || hashmap_length(lh->hashmap) <= 0) 
        return MAP_MISSING;
    
    return hashmap_iterate_keys(lh->hashmap, f, item);
    
}

int dlh_reverseIterateData(dlinkedhashmap *lh, PFunction f, element item){
    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_MISSING;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->tail;
    int i=0;
    /* Linear probing */

    for(;i<list->lenght;i++){
        content_t *data = (content_t *)(aux->value);
        int status = f(item, data->value);
        
        if (status != NODE_OK) return status;        
        if (aux->prev!=NULL) aux=aux->prev;
    }
    return NODE_OK;
}

int dlh_reverseRemoveData(dlinkedhashmap *lh, long int amount, PFree g){

    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_ERROR;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->tail;
    int i;

    for(i=0;i<amount;i++){
        content_t *data = (content_t *)(aux->value);
        if (data != NULL && g != NULL){
            hashmap_remove(lh->hashmap,data->keyPointer);
            if(data->keyPointer== NULL) printf("NULL"); else printf("->%s\n",data->keyPointer);
            g(data->value);
            free(data->keyPointer);
            free(data);
            list->lenght--;
            if ( aux->prev != NULL ){
                list->tail = aux->prev;
                free(aux);
                aux=list->tail;
            }
        }else return NODE_MISSING;
    }
    return NODE_OK;
}

int dlh_reverseIterateDataUntil(dlinkedhashmap *lh, long int amount, PFunction f, element item){
    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_MISSING;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->tail;
    int i=0;
    /* Linear probing */

    for(;i<amount;i++){
        content_t *data = (content_t *)(aux->value);
        int status = f(item, data->value);
        
        if (status != NODE_OK) return status;        
        if (aux->prev!=NULL) aux=aux->prev;
    }
    return NODE_OK;
}

int dlh_dataHasCondition(dlinkedhashmap *lh, PFunction f, element item){
    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_MISSING;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->header;
    int i=0;
    /* Linear probing */

    for(;i<list->lenght;i++){
        content_t *data = (content_t *)(aux->value);
        if (f(item, data->value)) return NODE_FOUND;
               
        if (aux->next != NULL) 
            aux=aux->next;
        else return NODE_MISSING;
    }
    return NODE_MISSING;
}

int dlh_reverseOperateRemoveData(dlinkedhashmap *lh, long int amount, PFunction f, element item){

    if (lh == NULL || lh->list == NULL || lh->list->lenght <= 0) 
        return LH_ERROR;	
    
    lhlinkedlist *list = lh->list;
    dnode *aux=list->tail;
    int i;

    for(i=0;i<amount;i++){
        content_t *data = (content_t *)(aux->value);
        if (data != NULL && f != NULL){
            hashmap_remove(lh->hashmap,data->keyPointer);
            f(item, data->value);
            free(data->keyPointer);
            free(data);
            list->lenght--;
            if ( aux->prev != NULL ){
                list->tail = aux->prev;
                free(aux);
                aux=list->tail;
                list->tail->next=NULL;
            }
        }else return NODE_MISSING;
    }
    return NODE_OK;
}