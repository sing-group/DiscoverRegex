/* 
 * File:   vector.h
 * Author: drordas
 *
 * Created on 3 de noviembre de 2015, 17:20
 */

#ifndef _GVECTOR_H_
#define	_GVECTOR_H_

#include "common_dinamic_structures.h"

#define GFail 0
#define GOK 1

#define FROM_BEGIN -1
#define TO_END -1

Gvector *newGenericVector(long int size);
int addElementAt(Gvector *vect, long int pos, gElement item);
int addElement(Gvector *vect, gElement item);
int getElementAt(Gvector *vect, long int pos, gElement *elem);
int addNewElementAtEnd(Gvector *vect,gElement item);
void freeGenericVector(Gvector *vect, GFree f);
void removeGenericElementAt(Gvector *vect, long int pos, GFree f);
long int getGenericVectorSize(Gvector *vect);
void shuffleGenericVector(Gvector *vector);
Gvector *getGenericVectorSlice(Gvector *vector, long int init, long int end);
Gvector *spliceGenericVector(Gvector *vector, long int init, long int lenght);
Gvector *joinGenericVector(Gvector *v1,Gvector *v2);
#endif	/* VECTOR_H */

