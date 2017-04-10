/* 
 * File:   vector.c
 * Author: drordas
 *
 * Created on 26 de marzo de 2012, 13:17
 */

#include <stdio.h>
#include <stdlib.h>
#include "linked_vector.h"
#include "common_dinamic_structures.h"

vector *new_vector(){
    
    vector *aux=malloc(sizeof(vector));
    aux->size=0;
    aux->actual = NULL;
    aux->header = NULL;
    aux->tail = NULL;
    
    return aux;
}

vNode *createEmptyNode(vNode *prev, vNode *next){
    vNode *new = malloc(sizeof(vNode));
    new->value = NULL;
    new->prev = prev;
    new->next = next;
    
    return new;
}

void addtoVectorAt(vector *v, int position, element item){
    int i=0;
    vNode *aux = v->header;
    
    if(position < v->size){
        printf("POSITION < SIZE\n");
        for(;i < position-1;i++) aux=aux->next;
        vNode *prev =aux->prev;
        vNode *new = malloc(sizeof(vNode));
        new->value = item;
        new->next = prev->next;
        new->prev = prev;
        prev->next = new;
        aux->prev = new;
        v->size++;
    }else{
        if(position == v->size){
            printf("POSITION == SIZE\n");
            vNode *new = malloc(sizeof(vNode));
            new->value = item;
            new->prev = v->tail;
            new->next = NULL;
            v->tail->next = new;
            v->tail = new;
            v->size++;
        }else{
            printf("POSITION > SIZE\n");
            int numNodes = (position - v->size);
            
            for(i=0;i<numNodes;i++){
                vNode *tmp = malloc(sizeof(vNode));
                tmp->prev = v->tail;
                tmp->next = NULL;
                tmp->value = NULL;
                v->tail->next = tmp;
                v->tail = tmp;
            }
            vNode *node = malloc(sizeof(vNode));
            node->value = item;
            node->next = NULL;
            node->prev = v->tail;
            v->tail->next = node;
            v->tail = node;
            v->size+=(numNodes+1);
        }
    }
}

void addtoVector(vector *v, element item){
    if(v == NULL){ 
        v = new_vector();
        vNode *aux = malloc(sizeof(vNode));
        aux->value = item;
        aux->next = NULL;
        aux->prev = NULL;
        v->header = aux;
        v->tail = aux;
        v->size++;
    }else{
        if( v->header == NULL ){
            vNode *new = malloc(sizeof(vNode));
            new->value = item;
            new->next = NULL;
            new->prev = NULL;
            v->header = new;
            v->tail = new;
            v->size++;
        }else{
            vNode *aux = malloc(sizeof(vNode));
            aux->value = item;
            aux->next = NULL;
            aux->prev = v->tail;
            v->tail->next = aux;
            v->tail = aux;
            v->size++;
        }
    }
}

int getNext(vector *v, element *item){
    if( v->size == 0 ){
        *item=NULL;
        return VECTOR_EMPTY; 
    }else{
        if(v->actual == NULL ){
            v->actual = v->header;
            *item = v->actual->value;
            return HAS_NEXT;
        }else{
            if(v->actual->next == v->tail){
                v->actual = NULL;
                *item = v->tail->value;
                return IS_LAST;
            }else{
                v->actual = v->actual->next;
                *item = v->actual->value;
                return HAS_NEXT;
            }
        }
    }
}

void free_vector(vector *v, PFree f){
    vNode *aux = v->header;
    vNode *temp;
    
    while(aux!=NULL){
        if(aux->value!=NULL && f!=NULL){
            element data = (aux->value);
            f(data);
        }
        temp=aux;
        aux=aux->next;
        free(temp);
    }
    free(v);
}

void print_vector(vector *v, PFree f){
    vNode *aux = v->header;
    
    while(aux!=NULL){
        f(aux);
        aux= aux->next;
    }
}

int print_vItems(element el){
    char *string = (char *)el;
    if(string != NULL){
        printf("[%s]",string);
    }
    return ELEMENT_FOUND;
}

int print_vNode(element el){
    vNode *aux = (vNode *)el;
    if(aux != NULL){
        if (aux->prev == NULL)
            printf("[---]  <--");
        else{ 
            if( aux->prev->value !=NULL)
                printf("[%s]  <--",(char *)aux->prev->value); 
            else printf("[---]  <--"); 
        }
        
        if (aux->value == NULL)
            printf(" [---] ");
        else printf(" [%s] ",(char *)aux->value);
        
        if (aux->next == NULL)
            printf("-->  [---]");
        else{
            if( aux->next->value !=NULL)
                printf("-->  [%s]",(char *)aux->next->value); 
            else printf("-->  [---]"); 
        }
        
        printf("\n");
    }
    return ELEMENT_FOUND;
}

int free_vItems(element el){
    char *string = (char *)el;
    
    if(string == NULL){
        free(string);
    }
    return ELEMENT_FOUND;
}

int getVectorSize(vector *v){
    if(v == NULL) 
        return 0;
    else return v->size;
}

int peekFromVectorAt(vector *v, int position, element *item){
    vNode *aux = NULL;
    if(v == NULL || v->size == 0){
        *item = NULL;
        return VECTOR_EMPTY;
    }else{
        if(position == v->size){
            *item = v->tail->value;
            aux = v->tail->prev;
            free(v->tail);
            v->tail = aux;
            aux->next = NULL;
            return ELEMENT_FOUND;
        }else{
            if(position > v->size){
                *item = NULL;
                return ELEMENT_NOT_FOUND;
            }else{
                int i=0;
                aux = v->header;
                for(;i < (position-1);i++) aux = aux->next;
                *item = aux->value;
                aux->prev->next = aux->next;
                free(aux);
                return ELEMENT_FOUND;
            }
        }
    }
}