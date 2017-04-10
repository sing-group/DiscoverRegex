/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#ifndef _EXP_GENETIC_TYPES_H_
#define _EXP_GENETIC_TYPES_H_

#include "filedata.h"
#include "pcre_regex_util.h"
#include "hashmap.h"
#include "linked_list.h"
#include "linkedhashmap.h"


typedef struct genetic_data_t {
    linklist *survivors;
    linkedhashmap *spam_result;
    linkedhashmap *ham_result;
} genetic_info;

typedef struct generic_data_t{
    file_data *trn_data;
    file_data *tst_data;
    pcre_vector *static_regex;
    map_t precompiled_regex;
} genetic_data;

#endif /* GENETIC_TYPES_H */

