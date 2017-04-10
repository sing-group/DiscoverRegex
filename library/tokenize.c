/***************************************************************************
*
*   File    : tokenize.c
*   Purpose : Realizes the division into a tokens of an e-mail.
*
*
*   Author  : David Ruano Ordás
*   Date    : November  02, 2010
*
*****************************************************************************
*   LICENSING
*****************************************************************************
*
* WB4Spam: An ANSI C is an open source, highly extensible, high performance and
* multithread spam filtering platform. It takes concepts from SpamAssassin project
* improving distinct issues.
*
* Copyright (C) 2010, by Sing Research Group (http://sing.ei.uvigo.es)
*
* This file is part of WireBrush for Spam project.
*
* Wirebrush for Spam is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* Wirebrush for Spam is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***********************************************************************/


/*---------------------------------------------------------------------------
   	       							     INCLUDES
 ---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "string_util.h"

#include "tokenize.h"

/*---------------------------------------------------------------------------
                                                                    FUNCTIONS
 ---------------------------------------------------------------------------*/

/**
 * Tokenizes a message and stores each different token in a hashmap with its
 * number of duplicates.
 */
map_t tokenize(char *email, map_t stopwords){
    char *email_copy = NULL;
    char *token = NULL;
    long int *numtoken;
    map_t tokenmap = NULL;
    any_t stopword;
    
    if(email == NULL){
        wblprintf(LOG_CRITICAL,"TOKENIZE","Error: Email content not found\n");
        return NULL;
    }
    
    if( ( email_copy = strdup(email)) == NULL ){
        wblprintf(LOG_CRITICAL,"TOKENIZE","Error: Not enought memory to allocate email content\n");        
        return NULL;
    }
    
    long int *count=malloc(sizeof(long int));
    *count=0;
    
    if( (tokenmap = hashmap_new()) == NULL){
        wblprintf(LOG_CRITICAL,"TOKENIZE","Error: Cannot create tokens hashmap\n");  
        free(email_copy);
        free(count);
        return NULL;
    }
    
    if( ( token = strtok(email_copy," \n\t#¿?;,.{}[]!¡*" ) ) == NULL ){
        wblprintf(LOG_CRITICAL,"TOKENIZE","Not usable tokens\n");        
        if( count != NULL ) free(count);
        if( email_copy != NULL) free(email_copy);
        if( tokenmap != NULL ) hashmap_free(tokenmap);
        return NULL;
    }
    
    hashmap_put(tokenmap,INIT_TOKEN,email_copy);
    
    while( token != NULL ){
        if( strcmp(token,COUNT_TOKEN) != 0 && strcmp(token,INIT_TOKEN) !=0 && 
            strlen(token) > 3 )
        {
            if( ( stopwords == NULL || hashmap_get(stopwords,to_lower_case(token),(any_t *)&stopword) == MAP_MISSING) ){
                *count=(*count)+1;
                if( hashmap_get(tokenmap,token,(any_t *)&numtoken) == MAP_MISSING ){
                    numtoken = malloc(sizeof(long int));
                    *numtoken = 1;
                    hashmap_put(tokenmap,token,numtoken);
                }else{
                    *numtoken = (*numtoken)+1;
                    hashmap_put(tokenmap,token,numtoken);
                }
            }
        }
        token = strtok(NULL," \n\t#¿?;,.{}[]!¡*");
    }
    
    hashmap_put(tokenmap,COUNT_TOKEN,count);
    //if(email_copy) free(email_copy);
    
    return tokenmap;
}

/**
 * Data liberation for tokenize function.
 */
int free_tokenize_data(any_t item, any_t data){
    
    if( data != NULL ){
        free(data);
        data=NULL;
    }
    
    return MAP_OK;
}

/**
 * Tokens hashmap liberations.
 */
void free_tokenize(map_t tokens){
    //hashmap_iterate_elements(tokens,&print_tokenize,NULL);
    if(tokens != NULL){
        hashmap_iterate(tokens,&free_tokenize_data,NULL);
        hashmap_free(tokens);
        tokens=NULL;
    }
}

/*
int printInfo(any_t item, any_t data, any_t key){
    printf("KEY: %s\n",(char *)key);
    if(strcmp((char *)key,INIT_TOKEN)==0)
        printf("Data: %s\n",(char *)data);
    else
        printf("Data: %d\n",(int)data);
    return MAP_OK;
}

int main(){

    printf("======= MAIN =======\n");
    map_t tok;
    printf("que de que que de a\n\n\t pepe pepe luis a a\n");
    tok=tokenize("que de que que de a\n\n\t pepe pepe luis a a\n");
    hashmap_iterate_elements(tok,&printInfo,NULL);
    free_tokenize(tok);
    printf("BLA\n");
    hashmap_iterate_elements(tok,&printInfo,NULL);
    return 1;
}
*/