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
#include <pthread.h>
#include "dlinked_list.h"
#include "common_dinamic_structures.h"
#include "logger.h"

/*
 * Struct which stores the pointers to the beginning and end of the linkedlist.
 * Also stores the lenght of the linkedlist.
 */

struct dlinkedlist{
    dnode *header;
    dnode *tail;
    int lenght;
    pthread_mutex_t mutex;
};

/**
 * Inicializes the struct of the linkedlist.
 * @return the linked-list initialized.
 */

dlinklist *newDlinkedlist(){

    dlinklist *newlist = (dlinklist *) malloc(sizeof(dlinklist));
    if(newlist==NULL){
        printlog(LOG_CRITICAL,"[LINKED_LIST.newlinkedlist]","Not enought memory");
        return NULL;
    }
    
    newlist->header=NULL;
    newlist->lenght=0;
    newlist->tail=NULL;
    
    pthread_mutex_init(&(newlist->mutex),NULL);
    
    return newlist;
}

/**
 * Adds a new node at the beginning of the linked-list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
int addbeginDlist(dlinklist *list, element token) {

    dnode *new;
    int toret=NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    
    //Reservamos memoria para esta estructura

    if ((new = (dnode *) malloc(sizeof(dnode)))) {
        
        list->lenght++;

        // Leemos el nombre y lo guardamos
        new->value= token; 
        if (list->header == NULL){
            //List is Empty
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
    } else {
        printlog(LOG_CRITICAL,"[LINKED_LIST.addbeginlist]","Not enought memory");
        toret=NODE_FAIL;
    }

    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

/**
 * Adds a new node at the end of the list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
void addendDlist(dlinklist *list, element token){

    dnode *new;
    
    pthread_mutex_lock(&(list->mutex));
    
    // Reservamos memoria para esta estructura
    if ( (new = (dnode *) malloc(sizeof(dnode))) != NULL){
        list->lenght++;

        // Leemos el nombre y lo guardamos
        new->value= token;
        new->next=NULL;

        if(list->header == NULL){
           // If List is empty we create First Node.
           list->header=new;
           list->tail=new;
        }
        else{
           new->prev=list->tail;
           list->tail->next=new;
           list->tail=new;
        }
    } else printlog(LOG_CRITICAL,"[LINKED_LIST.addendlist]","Not enought memory");

    pthread_mutex_unlock(&(list->mutex));
}

/**
 * Adds the node to the linkedlist using descendant sort.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list (must be a sorted type)
 */
void addorderDlist(dlinklist *list, element token,PFunction f){
	
    dnode *new = NULL;
    
    pthread_mutex_lock(&(list->mutex));
    
    if ((new = (dnode *) malloc(sizeof(dnode)))) {
              
        list->lenght++;
        new->value= token; 
        
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

            if(f(list->header->value, token) < 0){ //
                new->next=list->header;
                new->prev = NULL;
                list->header->prev=new;
                list->header=new;
            }
            else{
                dnode *pos=list->header;
                dnode *ant=list->header;

                while (pos->next!=NULL && f(pos->value, token) >= 0){
                    ant=pos;
                    pos=pos->next;
                }
                //INSERTO EL ULTIMO
                if( f(pos->value, token) >= 0){
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
    }else printlog(LOG_CRITICAL,"[LINKED_LIST.addorder]", "Not enought memory");
    
    pthread_mutex_unlock(&(list->mutex));
}

/**
 * Gets the lenght of the linked-list
 * @param list the linked-list which wants to know the lenght
 * @return the length of the linked-list (or -1 if is not created).
 */
int getlengthDlist(dlinklist *list){
    if(list == NULL) 
        return 0;
    else return list->lenght;
}

/**
 * Gets and removes the element at the first position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the first position
 * @return the result obtained by applying the function
 */
int removefirstDlist(dlinklist *list, element *elem){

    int toret = NODE_OK;
    pthread_mutex_lock(&(list->mutex));
    
    if(list->lenght==0){
        *elem=NULL;
        toret = LIST_EMPTY;
    }

    dnode *first=list->header;

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
int removelastDlist(dlinklist *list, element *elem){

    int toret = NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    
    if(list->lenght==0){
        *elem=NULL;
        toret=LIST_EMPTY;
    }

    dnode *aux=list->header;

    if(list->lenght==1){
        *elem=aux->value;
        list->header=NULL;
        list->tail=NULL;
    }
    else{
        *elem = list->tail->value;
        aux = list->tail->prev;
        aux->next = NULL;
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
int getatDlist(dlinklist *list,int position, element *elem){
    int i;
    int toret = NODE_OK;
    pthread_mutex_lock(&(list->mutex));
    
    dnode *pointer=list->header;
    
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
void freeDlist(dlinklist *list, PFree f){
    int i=0;
    dnode *aux=list->header;
    dnode *temp;
    
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
    list->lenght=0;
    free(list);
    list=NULL;
}

/**
 * Deletes @amount of elements from end of list.
 * @param list the linked-list to be deleted.
 * @PFree callback to erasing funtion.
 * @amount number of elements to be erased
 */
void freeAmountAtEndDlist(dlinklist *list, long int amount, PFree f){
    if (list != NULL){
        pthread_mutex_lock(&(list->mutex));
        int i=0;
        dnode *aux;
        if (list->lenght <= amount){
            for(;i<list->lenght;i++){
                aux = list->header;
                if(aux!=NULL){
                    void *data=(void *)(aux->value);
                    (data != NULL && f != NULL)?(f(data)):(0);

                    if(aux->next!=NULL){
                        list->header=aux->next;
                        free(aux);
                    } else free(aux);
                }
            }
            list->lenght=0;
        }else{
            dnode *prev;
            for(;i<amount;i++){
                aux = list->tail;
                if(aux!=NULL){
                    void *data=(void *)(aux->value);
                    (data != NULL && f != NULL)?(f(data)):(0);
                    if (list->tail->prev != NULL){
                        prev=list->tail->prev;
                        prev->next = NULL;
                        list->tail=prev;
                        free(aux);
                        list->lenght--;
                    }else free(aux);
                }
            }
        }
        free(list);
        list = NULL;
        pthread_mutex_unlock(&(list->mutex));
        pthread_mutex_destroy(&(list->mutex));
    }
}

/*
 * Iteratively call f with argument (item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int dlinklist_iterate_data(dlinklist *list, PFunction f, element item){

    if (list == NULL || list->lenght <= 0) return NODE_MISSING;	
    
    dnode *aux=list->header;
    int i;
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
 * Iteratively call f with argument (item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int dlinklist_reverse_remove(dlinklist *list, long int amount, PFree g){

    if (list == NULL || list->lenght <= 0) return NODE_MISSING;	
    
    dnode *aux=list->tail;
    int i;

    for(i=0;i<amount;i++){
        element data = (element)(aux->value);
        (g != NULL )?(g(data)):(0);
        
        if ( aux->prev != NULL ){
            list->tail = aux->prev;
            list->lenght--;
            free(aux);
            aux=list->tail;
        }        
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
int dlinklist_iterate_items(dlinklist *list, PFunction_3 f, element item, element item2){
    dnode *aux=list->header;
    int i;
    if (list->lenght <= 0) return NODE_MISSING;	

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
int dlinklist_iterate_three(dlinklist *list, PFunction_4 f, element item, element item2, element item3){
    dnode *aux=list->header;
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

int getfirstDlist(dlinklist *list, element *elem){
    int toret = NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    if (list== NULL || list->lenght<=0 || list->header == NULL || 
        list->header->value == NULL){
        *elem = NULL;
        toret = NODE_MISSING;
    }else *elem = list->header->value;
    
    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

int getlastDlist(dlinklist *list, element *elem){
    int toret = NODE_OK;
    
    pthread_mutex_lock(&(list->mutex));
    if (list== NULL || list->lenght<=0 || list->tail == NULL || 
        list->tail->value == NULL){
        *elem=NULL;
        toret = NODE_MISSING;
    }else *elem = list->tail->value;
    
    pthread_mutex_unlock(&(list->mutex));
    
    return toret;
}

void printDlist(dlinklist *list){
    
    if (list == NULL || list->lenght <= 0){ 
        printf("[NULL]\n");	
    }
    else{
        dnode *aux=list->header;
        int i;
        
        for(i=0;i<list->lenght;i++){
            void *data = (void *)(aux->value);
            (aux->prev == NULL)?(printf("()<--")):(printf("%s<--",(char *)(aux->prev->value)));
            printf("%s",(char *)data);
            ( aux->next == NULL)?(printf("-->()\n")):(printf("-->%s\n",(char* )(aux->next->value)));
            if(aux->next!=NULL) {
                aux=aux->next;
            }
        }
        printf("\n");
    }
}

void setDlistLenght(dlinklist *list, int value){
    list->lenght=value;
}
