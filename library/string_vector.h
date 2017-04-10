/* 
 * File:   vector.h
 * Author: drordas
 *
 * Created on 3 de noviembre de 2015, 17:20
 */

#ifndef _SVECTOR_H_
#define	_SVECTOR_H_

#include "common_dinamic_structures.h"

#define SFail 0
#define SOK 1

struct t_vector{
    char **data;
    long int size;
};

Svector *newStringvector(long int size);

int addStringAt(Svector *, long int, char *);
int addSortedString(Svector *, long int, char *);
int addNewStringAtEnd(Svector *,char *);
int addNewStringAtBegin(Svector *,char *);

int replaceStringAt(Svector *, long int, char *);
int removeStringAt(Svector *, long int);
int removeStringInterval(Svector *, long int, long int);

char *getStringAt(Svector *, long int );
char *stringVectorToString(Svector *);

long int getStringVectorSize(Svector *);

void shuffleStringVector(Svector *);
void fisherYatesShuffle(Svector *, PFString);
void freeStringVector(Svector *);

Svector *cloneStringVector(Svector *);
Svector *spliceStringVector(Svector *, long int, long int );
Svector *joinStringVectors(Svector *, PFelem, SElem, Svector *, PFelem, SElem);
Svector *concatStringVectors(Svector *, Svector *, long int, long int);
Svector *concatStringVectorsAddEnd(Svector *, Svector *, long int, long int, char *);
Svector *concatStringVectorsAddBegin(Svector *, Svector *, long int, long int, char *);
Svector *concatStringVectorsAddBeginEnd(Svector *, Svector *, long int, long int, char *, char *);
Svector *mergeSVectors(Svector *, Svector *, long int);

void printStringVector(Svector *);
void swapStringVector(Svector *, long int, long int);

#endif	/* VECTOR_H */

