#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_vector.h"
#include "spam_data_util.h"

struct spam_data_t{
    char * line;
    Svector * words;
    spamElement aux;
};

char *getSpamLine(SpamData * data){
    if (data == NULL || data->line == NULL){
        return NULL;
    }else return data->line;
}

Svector *getSpamWords(SpamData * data){
    if (data == NULL || data->words == NULL){
        return NULL;
    }
    else return data->words;
}

SpamData *newSpamData(char *line, Svector *words){
    SpamData *data = malloc(sizeof(SpamData));
    data->line = line;
    data->words = words;
    return data;
}

void setAuxElement(SpamData *data, spamElement element, SDFree f){
    if(data!= NULL){
        if ( data->aux != NULL )
            f(data->aux);
        else data->aux = element;
    }
}

void freeSpamData(SpamData *data){
    free(data);
}
