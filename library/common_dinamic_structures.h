/* 
 * File:   common_dinamic_structures.h
 * Author: drordas
 *
 * Created on 18 de febrero de 2011, 16:09
 */

#ifndef COMMON_DINAMIC_STRUCTURES_H
#define	COMMON_DINAMIC_STRUCTURES_H

#include <stdbool.h>

#define isEven(a) ((a)%2==0?true:false)
#define isOdd(a) ((a)%2==0?false:true)

#define NODE_MISSING -3
#define LIST_EMPTY -1
#define NODE_FOUND -2
#define NODE_OK 0
#define NODE_FAIL 1

#define LH_ERROR -1
#define LH_OK 0
#define LH_MISSING 1
#define LH_EXIST 2

typedef void *element;

struct node{
   element value;
   struct node *next;
};

typedef struct node node;

struct dnode{
   element value;
   struct dnode *next;
   struct dnode *prev;
};

typedef struct dnode dnode;

typedef int (*PFunction_4)(element, element, element, element);
typedef int (*PFunction_3)(element, element, element);
typedef int (*PFunction)(element, element);
typedef int (*PFree)(element);

//GENERIC VECTOR

typedef struct g_vector Gvector;

typedef void *gElement;
typedef int (*GFree)(gElement);
typedef void (*GTwo)(gElement, gElement);

//STRING VECTOR

typedef struct t_vector Svector;

typedef void *SElem;
typedef int (*PFelem)(SElem elem, char *string);
typedef int (*PFString)(char * elem);


#endif	/* COMMON_DINAMIC_STRUCTURES_H */

