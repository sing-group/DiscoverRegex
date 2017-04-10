/****************************************************************
*
*   File    : linked_list.c
*   Purpose : Implements a linked list for storing data.
*
*
*   Author  : David Ruano Ordás
*
*
*   Date    : November  03, 2011
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
#include "linked_list.h"
#include "common_dinamic_structures.h"
#include "generic_vector.h"
#include "logger.h"
#include <pthread.h>

/*
 * Struct which stores the pointers to the beginning and end of the linkedlist.
 * Also stores the lenght of the linkedlist.
 */

struct linkedlist{
    node *header;
    node *tail;
    node *next;
    int lenght;
    pthread_mutex_t mutex;
};

/**
 * Inicializes the struct of the linkedlist.
 * @return the linked-list initialized.
 */

linklist *newlinkedlist(){

    linklist *newlist = (linklist *) malloc(sizeof(linklist));
    if(newlist==NULL){
        printlog(LOG_CRITICAL,"[LINKED_LIST.newlinkedlist]","Not enought memory");
        return NULL;
    }
    
    newlist->header=NULL;
    newlist->lenght=0;
    newlist->tail=NULL;
    newlist->next = newlist->header;
    pthread_mutex_init(&(newlist->mutex),NULL);
    
    return newlist;
}

int linklistHasNext(linklist *list,element *elem){
    if(list->next==list->tail){
        *elem = list->next->value;
        list->next=list->next->next;
        return NODE_OK;
    }else{
        list->next = list->header;
        *elem = NULL;
        return NODE_FAIL;
    }
}

/**
 * Adds a new node at the beginning of the linked-list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
int addbeginlist(linklist *list, element token) {

    node *new;
    int toret=NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    
    //Reservamos memoria para esta estructura

    if ((new = (node *) malloc(sizeof(node)))) {
        
        list->lenght++;

        // Leemos el nombre y lo guardamos
        new->value= token; 

        if (list->header == NULL){
            //List is Empty
            new->next=NULL;
            list->header=new;
            list->next=new;
            list->tail=new;
        }
        else{
            new->next=list->header;
            list->header=new;
        }
    } else {
        printlog(LOG_CRITICAL,"[LINKED_LIST.addbeginlist]","Not enought memory");
        toret=NODE_FAIL;
    }

    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}
/**
 * Adds the node to the linkedlist using descendant sort.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list (must be a sorted type)
 */
void addorder(linklist *list, element token,PFunction f){
	
    node *new;
    
    pthread_mutex_lock(&(list->mutex));
    
    if ((new = (node *) malloc(sizeof(node)))) {
        
        list->lenght++;
        new->value= token; 
        
	//SI LA LISTA ESTA VACIA
        if (list->header == NULL){
            new->next=NULL;
            list->header=new;
            list->tail=new;
            list->next=new;
        }
        else{
            //SI HEADER ES < QUE EL NUEVO -> INSERTO AL PRINCIPIO
            //f = -1 a<b
            //f = 0 a=b
            //f = 1 a>b

            if(f(list->header->value, token) < 0){ //
                new->next=list->header;
                list->header=new;
                list->next = new;
            }
            else{
                node *ant=list->header;
                node *pos=list->header;

                while (pos->next!=NULL && f(pos->value, token) >= 0){
                    ant=pos;
                    pos=pos->next;
                }
                //INSERTO EL ULTIMO
                if(f(pos->value, token) >= 0){
                    pos->next=new;
                    new->next=NULL;
                    list->tail=new;
                }
                //INSERTO EN EL MEDIO
                else{
                    new->next=ant->next;
                    ant->next=new;
                }
            }
        }
        
    }else printlog(LOG_CRITICAL,"[LINKED_LIST.addorder]", "Not enought memory");
    
    pthread_mutex_unlock(&(list->mutex));
}

void addAtPos(linklist *list, element token,int position){
    
    pthread_mutex_lock(&(list->mutex));
    
    node *pointer = NULL;
    
    if(position <= list->lenght){
        pointer = list->header;
        int i=0;
        for(;i<position;i++) pointer=pointer->next;
        pointer->value = token;
        list->header = pointer;
        list->next = pointer;
        
    }else{
        
        int difference = list->lenght - position;
        int i=0;
        
        for(; i<difference; i++){
            node *temporary = malloc(sizeof(node));
            temporary->next = NULL;
            temporary->value = NULL;
            list->tail->next = temporary;
        }
        
        node *final = malloc(sizeof(node));
        final->next = NULL;
        final->value = token;
        list->tail->next = final;
    }
    
    pthread_mutex_unlock(&(list->mutex));
}

/**
 * Gets the lenght of the linked-list
 * @param list the linked-list which wants to know the lenght
 * @return the length of the linked-list (or -1 if is not created).
 */
int getlengthlist(linklist *list){
    if(list==NULL) 
        return -1;
    else return list->lenght;
}

/**
 * Adds a new node at the end of the list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
void addendlist(linklist *list, element token){

    node *new;
    
    pthread_mutex_lock(&(list->mutex));
    
    // Reservamos memoria para esta estructura
    if ( (new = (node *) malloc(sizeof(node))) != NULL){
        list->lenght++;

        // Leemos el nombre y lo guardamos
        new->value= token;
        new->next=NULL;

        if(list->header == NULL){
           // If List is empty we create First Node.
           list->next=new;
           list->header=new;
           list->tail=new;
        }
        else{
           list->tail->next=new;
           list->tail=new;
        }
    } else printlog(LOG_CRITICAL,"[LINKED_LIST.addendlist]","Not enought memory");

    pthread_mutex_unlock(&(list->mutex));
}

/**
 * Returns the content of the first element of the list.
 * @param list the linked-list to insert at.
 * @return returns the result obtained by applying the function
 */
int getfirst(linklist *list, element *elem){
    
    int toret = NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    
    if(list->lenght==0){
        *elem=NULL;
        toret = NODE_MISSING;
    }

    *elem=list->header->value;
    
    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

/**
 * Returns the content of the tail node.
 * @param list the linked-list to insert at.
 * @return the result obtained by applying the function
 */
int getlast(linklist *list, element *elem){
    
    int toret = NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    
    if(list->lenght==0){
        *elem=NULL;
        toret = NODE_MISSING;
    }

    *elem=list->tail->value;
    
    pthread_mutex_unlock(&(list->mutex));

    return toret;
}

/**
 * Gets and removes the element at the first position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the first position
 * @return the result obtained by applying the function
 */
int removefirst(linklist *list, element *elem){

    int toret = NODE_OK;
    pthread_mutex_lock(&(list->mutex));
    
    if(list->lenght==0){
        *elem=NULL;
        toret = LIST_EMPTY;
    }

    node *first=list->header;

    if(list->lenght==1){
        *elem=first->value;
        list->header=NULL;
        list->tail=NULL;

    }
    else{
        list->header=first->next;
        *elem=first->value;
    }
    free(first);
    list->lenght--;
    
    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

/**
 * Gets and removes the element at the last position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the last position
 * @return the result obtained by applying the function
 */
int removelast(linklist *list, element *elem){

    int toret = NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    
    if(list->lenght==0){
        *elem=NULL;
        toret=LIST_EMPTY;
    }

    node *aux=list->header;

    if(list->lenght==1){
        *elem=aux->value;
        list->header=NULL;
        list->tail=NULL;
    }
    else{
        while(aux->next!=list->tail){
            aux=aux->next;
        }
        list->tail=aux;
        aux=aux->next;
        *elem=aux->value;
    }
    
    free(aux);
    list->lenght--;
    
    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

/**
 * Get element content at a determined position
 * @param list the linked-list to insert at.
 * @param the position of the token
 * @return the result obtained by applying the function
 */
int getatlist(linklist *list,int position, element *elem){
    int i;
    int toret = NODE_OK;
    pthread_mutex_lock(&(list->mutex));
    
    node *pointer=list->header;
    
    if(list==NULL || list->lenght==0) {
        *elem=NULL;
        toret = LIST_EMPTY;
    }
    
    if(position == 0){
        *elem=pointer->value;
        toret = NODE_OK;
    }
    
    if(position<=list->lenght){
        for(i=0;i<position;i++){
            pointer=pointer->next;
        }
        
        *elem=pointer->value;
        toret = NODE_OK;
    }
    else{
        *elem=NULL;
        toret = NODE_MISSING;
    }
    
    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

/**
 * Deletes all the linked-list.
 * @param list the linked-list to be deleted.
 */
void freelist(linklist *list, PFree f){
    int i=0;
    node *aux=list->header;
    node *temp;
    
    for(;i<list->lenght;i++){
        if(aux!=NULL){
            void *data=(void *)(aux->value);
            if(data != NULL && f != NULL) 
                f(data);
            
            if(aux->next!=NULL){
                temp=aux;
                aux=aux->next;
                free(temp);
            } else free(aux);
        }
    }
    
    pthread_mutex_destroy(&(list->mutex));
    
    free(list);
    list=NULL;
}

/*
 * Iteratively call f with argument (item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int linklist_iterate_data(linklist *list, PFunction f, element item){

    /* Cast the hashmap */
    node *aux=list->header;
    int i;
    /* On empty hashmap, return immediately */
    if (list->lenght <= 0)
            return NODE_MISSING;	

    /* Linear probing */

    for(i=0;i<list->lenght;i++){
        void *data = (void *)(aux->value);
        int status = f(item, data);
        
        if (status != NODE_OK) return status;        
        if (aux->next!=NULL) aux=aux->next;
    }
    return NODE_OK;
}

/*
 * Iteratively call f with argument (item2, item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int linklist_iterate_items(linklist *list, PFunction_3 f, element item, element item2){
    node *aux=list->header;
    int i;
    if (list->lenght <= 0)
       return NODE_MISSING;	

    /* Linear probing */
    for(i=0;i<list->lenght;i++){
        void *data = (void *)(aux->value);
        int status = f(item2, item, data);
        
        if (status != NODE_OK) return status;
        if(aux->next!=NULL) aux=aux->next;
    }
    return NODE_OK;
}

/*
 * Iteratively call f with argument (item3, item2, item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int linklist_iterate_three(linklist *list, PFunction_4 f, element item, element item2, element item3){
    node *aux=list->header;
    int i;
    if (list->lenght <= 0)
       return NODE_MISSING;	

    /* Linear probing */
    for(i=0;i<list->lenght;i++){
        void *data = (void *)(aux->value);
        int status = f(item3, item2, item, data);
        
        if (status != NODE_OK) return status;
        if(aux->next!=NULL) aux=aux->next;
    }
    return NODE_OK;
}

linklist *splice_linklist(linklist *list, int initPos, int among){
    if( list != NULL && list->lenght != 0){
        
        int numElem = (initPos+among >= list->lenght)?
                        (list->lenght-initPos):(among);
        int i;
        node *pointer=list->header;
        node *previous= NULL;

        for (i=0;i<initPos;i++){ 
            previous = pointer;
            pointer=pointer->next;
        }
        
        linklist *toret = newlinkedlist();
        
        for (;i<numElem;i++){
            node *new = (node *) malloc(sizeof(node));
            new->value = pointer->value;
            new->next = NULL;
            
            if (toret->header == NULL){
                toret->header = new;
                toret->tail = new;
                toret->next = new;
            }else{
                toret->tail->next = new;
                toret->tail = new;
            }
            
            if(pointer->next != NULL) previous=pointer->next;
            free(pointer);
            pointer = previous;
            toret->lenght++;
            list->lenght--;
        }
        
        return toret;
    }
    
    return list;
}

Gvector * linklist_to_GVector(linklist *list, int init, int among){
    Gvector *toret = NULL;
    
    if(list != NULL && list->lenght != 0 && init <= list->lenght && among > 0){
        int numElems = among;
        if ( (among + init) >= list->lenght ) numElems = list->lenght-init;
        int pos = 0;
        toret = newGenericVector(numElems);
        node *actual = list->header;
        
        for (;pos<init;pos++) actual = actual->next;
        
        for (pos=0;pos<numElems;pos++){
            addElementAt(toret,pos,actual->value);
            actual = actual->next;
        }
    }
    
    return toret;
}

int linklistHasCondition(linklist *list, PFunction f, element item){
    if (list == NULL || list->lenght <= 0) 
        return LIST_EMPTY;	
    
    node *aux=list->header;
    int i=0;
    /* Linear probing */

    for(;i<list->lenght;i++){
        void *data = (void *)(aux->value);
        if (f(item, data)) return NODE_FOUND;
               
        if (aux->next != NULL) 
            aux=aux->next;
        else return NODE_MISSING;
    }
    return NODE_MISSING;
}

