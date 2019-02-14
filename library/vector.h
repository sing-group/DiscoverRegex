/* 
 * File:   vector.h
 * Author: drordas
 *
 * Created on 26 de marzo de 2012, 13:17
 */

#ifndef _VECTOR_H_
#define	_VECTOR_H_

#include "common_dinamic_structures.h"

#define VECTOR_OK 0
#define VECTOR_FAIL -1

struct vector{
    void **v;
    int size;
};

typedef struct vector vector;

vector *new_vector(int size);
void free_vector(vector *v, PFree f);

#endif	/* VECTOR_H */

