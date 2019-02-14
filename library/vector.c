/* 
 * File:   vector.c
 * Author: drordas
 *
 * Created on 26 de marzo de 2012, 13:17
 */

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "vector.h"

vector *new_vector(int size){
    if(size<=0){
        wblprintf(LOG_CRITICAL,"VECTOR","Incorrect size\n");
        return NULL;
    }
    
    vector *aux=malloc(sizeof(vector));
    aux->size=size;
    aux->v=malloc(sizeof(void *)*size);
    int i=0;
    
    for(;i<aux->size;i++){
        aux->v[i]=NULL;
    }
    return aux;

}

void free_vector(vector *v, PFree f){
    if(v!=NULL){
        int i=0;
        for(;i<v->size;i++) 
            (v->v[i]!=NULL)?f(v->v[i]):(0);
    }
    free(v->v);
    v->size=0;
    free(v);
    v=NULL;
}

