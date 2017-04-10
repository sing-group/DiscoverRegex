/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef GENETIC_TYPES_H
#define GENETIC_TYPES_H

#include "filedata.h"
#include "pcre_regex_util.h"
//#include "hashmap.h"
//#include "linked_list.h"
#include "dlinkedhashmap.h"
#include "mating_pool.h"

typedef struct breed_data_t{
    long int numSlots;
    sMatingPool *pool;
} breedData;


typedef struct genetic_data_t {
    //linkedhashmap *population;
    dlinkedhashmap *population;
    file_data *data;
    pcre_vector *static_regex;
    map_t precompiled_regex;
    long int max_population;
    breedData *breed;
} genetic_info;

#endif /* GENETIC_TYPES_H */

