/* 
 * File:   mating_pool.h
 * Author: drordas
 *
 * Created on 4 de febrero de 2016, 14:13
 */

#ifndef _MATING_POOL_H_
#define	_MATING_POOL_H_

#define MOK 1
#define MFAIL 0

typedef struct sMating_data_t sMatingPool;
typedef void * matingElement;

typedef int (*MFree)(matingElement);

sMatingPool *newSMatingPool(long int size);
int getMatingElement(sMatingPool *pool, long int pos, matingElement *elem);
//int addMatingElement(sMatingPool *pool, long int pos, matingElement elem);
int addMatingElementAtEnd(sMatingPool *pool, matingElement elem);
long int getMatingPoolSize(sMatingPool *pool);
void freeMattingPool(sMatingPool *pool,MFree f);

#endif	/* MATING_POOL_H */

