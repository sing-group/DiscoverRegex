/* 
 * File:   vector.h
 * Author: drordas
 *
 * Created on 26 de marzo de 2012, 13:17
 */

#ifndef _VECTOR_H_
#define	_VECTOR_H_

#include "common_dinamic_structures.h"

#define ELEMENT_NOT_FOUND 1
#define ELEMENT_FOUND 1
#define VECTOR_EMPTY 2
#define HAS_NEXT 3
#define IS_LAST 4

struct vNode{
   element value;
   struct vNode *prev;
   struct vNode *next;
};

typedef struct vNode vNode;

struct vector{
    vNode *actual;
    vNode *header;
    vNode *tail;
    int size;
};

typedef struct vector vector;

vector *new_vector();
void addtoVectorAt(vector *v, int position, element item);
void addtoVector(vector *v, element item);
void free_vector(vector *v, PFree f);
int getVectorSize(vector *v);

#endif	/* VECTOR_H */

