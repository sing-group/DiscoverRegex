/* 
 * File:   genetic.h
 * Author: drordas
 *
 * Created on 27 de enero de 2016, 12:48
 */

#ifndef _GENETIC_H_
#define	_GENETIC_H_

#include "genetic_types.h"

#define SPAM_OPTION  0
#define HAM_OPTION 1


bool createChromosomes( genetic_info *genInfo, int initPos, int endPos );
void breedPopulation(genetic_info *genInfo);
void killPopulation(genetic_info *genInfo);

#endif	/* GENETIC_H */

