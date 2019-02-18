/* 
 * File:   meta_library.h
 * Author: david
 *
 * Created on 30 de julio de 2011, 11:42
 */

#ifndef _META_LIBRARY_H_
#define	_META_LIBRARY_H_

#include <string.h>
#include "hashmap.h"
#include "vector.h"

int evaluate_meta(char *toEval, map_t executed_rules);

int count_meta_dependant_rules(char *expresion);

vector *parse_dependant_rules(char *expresion);

void free_meta_dependant_rules(vector *v);

#endif	/* META_LIBRARY_H */

