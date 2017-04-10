/****************************************************************
*
*   File    : genetic.c
*   Purpose : Implements a linked list for storing data.
*
*
*   Author  : David Ruano Ordás
*
*
*   Date    : January  03, 2016
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "genetic.h"
#include "filedata.h"
#include "pcre_regex_util.h"
#include "string_util.h"
#include "hashmap.h"
#include "linkedhashmap.h"
#include "mating_pool.h"
#include "spam_data_util.h"
#include "sorted_cache.h"
#include "logger.h"
#include "linked_vector.h"

#define CONSTANT_FITNESS 200.00
#define START_POSITION 0

bool isSpecialString(char *string);

int freeHashmapKey(any_t nullpointer, any_t key){
    char *aux = (char *)key;
    if (aux!=NULL) free(aux);
    return MAP_OK;
}

int freeNoneKey(any_t nullpointer, any_t key){
    return MAP_OK;
}

Svector *createGenes(Svector *spamWords, pcre_vector *compiledRegex){
    int choice = 0; // choice of various options
    long int wordcount = getStringVectorSize(spamWords);

    if(wordcount > 0){
        Svector *chromosome = newStringvector(wordcount);
        long int i=0;
        for(;i<wordcount;i++){
            char *word = strdup(getStringAt(spamWords,i));
            choice = (int)(rand()%3);
            switch(choice){
                case 0: addStringAt(chromosome,i,strdup(".*?"));
                        free(word);
                        break;
                case 1: addStringAt(chromosome,i,word);
                        break;  
                case 2: addStringAt(chromosome,i,generateRegexGen(word,compiledRegex));
                        free(word);
                        break;
            }
        }
        return chromosome;
    }else{
        printlog(LOG_WARNING,"[GENETIC.createGenes]","Cannot compute empty line\n");
        return NULL;
    }
}

Svector *optimizeGenes(Svector *chromosomes, bool option){
    // Optimize the chromosomes by grouping repeated genes, and deleting redundant genes (like .*.*)
    // Altering the passed array, so all array operations must be made by reference    
    int repeats = 0;

    if (chromosomes == NULL) return NULL;
    
    if (getStringVectorSize(chromosomes) == 0){ 
        free(chromosomes);
        return NULL;
    }
    
    if (getStringVectorSize(chromosomes) == 1 && !strcmp(getStringAt(chromosomes,0),".*?") ){ 
        freeStringVector(chromosomes);
        return NULL;
    }
    
    int current=(getStringVectorSize(chromosomes)-1);
    
    // Take 2 passes; one to add whitespace and delete repeated ".*"; the 2nd to combine identical
    // repeated genes.
    //1 iteration
    while(current >= 0){
        //Append whitespace unless it's the last genes or ".*"
        if( (option) && current != (getStringVectorSize(chromosomes)-1)){
            if ( strcmp(getStringAt(chromosomes,current),".*?") && 
                 current<(getStringVectorSize(chromosomes)-1) && 
                 strcmp(getStringAt(chromosomes,current+1),".*?") )
                addStringAt(chromosomes,current,appendstr(getStringAt(chromosomes,current),"\\s+?"));
            //if (strcmp(getStringAt(chromosomes,current),"[^\\s]*\\s") )
            //    addStringAt(chromosomes,current,appendstr(getStringAt(chromosomes,current),"\\s+"));
        }
        
        /*while( current && ( !strcmp(getStringAt(chromosomes,current),"[^\\s]*\\s") && 
                            !strcmp(getStringAt(chromosomes,current-1),"[^\\s]*\\s")) ){
            removeStringAt(chromosomes,current);
            current--;
        }
        */
        while( current && ( !strcmp(getStringAt(chromosomes,current),".*?") && 
                            !strcmp(getStringAt(chromosomes,current-1),".*?")) ){
            removeStringAt(chromosomes,current);
            current--;
        }
        
        current--;
    }
    
    //2 iteration
    current = getStringVectorSize(chromosomes)-1;
    
    while(current > 0){
        repeats = 0;
        //If are repeated genes, combine them in (genes){count} format
        while(current && ( !strcmp(getStringAt(chromosomes,current),getStringAt(chromosomes,current-1)) ) ){
            repeats++;
            removeStringAt(chromosomes,current);
            current--;
        }
        
        if (repeats){
            repeats++;
            replaceStringAt(chromosomes,current,createString("(?:%s){%d}",getStringAt(chromosomes,current),repeats));
        }
        current--;
    }
    
    if ( getStringVectorSize(chromosomes) > 0 && (getStringVectorSize(chromosomes) != 1 && 
         (strcmp(getStringAt(chromosomes,0),".*?") || strcmp(getStringAt(chromosomes,0),"\\s+?")) ) ) 
    {
        if ( !strcmp(getStringAt(chromosomes,0),".*?") ) 
            removeStringAt(chromosomes,0);
        else {
            (option)?
                (replaceStringAt(chromosomes,0,createString("^%s",getStringAt(chromosomes,0)))):
                (0);
        }

        if (!strcmp(getStringAt(chromosomes,(getStringVectorSize(chromosomes)-1)),".*?") ){
            removeStringAt(chromosomes,getStringVectorSize(chromosomes)-1);
        }
    }
    return chromosomes;
}

int isMatching(pcre* regex, Svector *toMatch){
    int i;
    for(i=0;i<getStringVectorSize(toMatch);i++){
        if(pcregex_match(regex,getStringAt(toMatch,i))>0){
            return 1;
        }
    }
    return 0;
}

long int match(pcre *regex, Svector *toMatch){
    int i;
    long int count = 0;
    for(i=0;i<getStringVectorSize(toMatch);i++){
        if(pcregex_match(regex,getStringAt(toMatch,i))>0){
            count++;
        }
    }
    return count;
}

int descSort(element item1, element item2){
    regex_data *elem1 = (regex_data *)item1;
    regex_data *elem2 = (regex_data *)item2;
    
    if(getRegexDataScore(elem1) < getRegexDataScore(elem2)) 
        return -1;
    else if (getRegexDataScore(elem1) == getRegexDataScore(elem2)) 
        return 0;
    else return 1;
}

float scoreregexes(char *pattern){
    //return (1+((CONSTANT_FITNESS-strlen(pattern))/CONSTANT_FITNESS));

    if (strlen(pattern) <= 10) 
        return 1;
    else return ((10.0*(1.0/(strlen(pattern)+1.0)))+1);
}

Svector *genRegex(email_data *data,pcre_vector *staticRegexes){
    if (getEmailLineWords(data) != NULL){
        return optimizeGenes(createGenes(getEmailLineWords(data),staticRegexes),true);
    }else{ 
        printlog(LOG_CRITICAL,"[GENETIC.genRegex]", "Cannot obtain email data\n");
        return NULL;
    }
}

bool createChromosomes( genetic_info *genInfo, int initPos, int endPos ){
    
    long int i=initPos;
    long int amount = (endPos-initPos)+1;
    long int counter = 0;
    Gvector *lines = getSpamData(genInfo->data);
    printlog(LOG_INFO,"[GENETIC.createChromosomes]","Generating %d chromosomes\n",amount);
    
    while(counter < amount){
        email_data *data = NULL;
        Svector *chr = NULL;
        char *pattern = NULL;
        char *aux = NULL;
        if(getElementAt(lines,i,(gElement *)&data) == GOK && data != NULL){
            if ( ((chr = genRegex(data,genInfo->static_regex)) != NULL) && 
                 ((pattern = stringVectorToString(chr)) != NULL )){
                pcre *regexp = NULL;
                if ( (hashmap_get(genInfo->precompiled_regex,pattern,(any_t *)&aux) == MAP_OK) ||
                     (regexp = pcregex_compile(pattern)) == NULL ){
                    printlog(LOG_WARNING,"[GENETIC.createChromosomes]","Chromosome '%s' already exists\n",pattern);
                    freeStringVector(chr);
                    free(pattern);
                    return false;
                }else{
                    hashmap_put(genInfo->precompiled_regex,strdup(pattern),regexp);
                    long int numMatches = 0;
                    if ( !isMatching(regexp,getHamLines(genInfo->data)) ){
                        if ( (numMatches = (match(regexp,getSpamLines(genInfo->data)))) > 0 ){
                            float fitness = numMatches*scoreregexes(pattern);
                            add_dlhSortedElement(genInfo->population,pattern,
                                        newRegexData( pattern,regexp,numMatches,
                                                     fitness,chr ),&descSort);
                            genInfo->breed->numSlots += (int)round(fabs(fitness));
                            return true;
                        }else{
                            printlog(LOG_WARNING,"[GENETIC.computeSPAMValidRegex]","Regex '%s' does not match in spam content\n",pattern);
                            free(pattern);
                            freeStringVector(chr);
                            return false;
                        }
                    }else{ 
                        printlog(LOG_WARNING,"[GENETIC.computeSPAMValidRegex]","Regex '%s' matches in ham content\n",pattern);
                        free(pattern);
                        freeStringVector(chr);
                        return false;
                    }
                }
            }else {
                printlog(LOG_WARNING,"[GENETIC.createChromosomes]","Cannot generate chromosome for %s\n",getEmailFullLine(data));
                freeStringVector(chr);
                (pattern != NULL)?(free(pattern)):(0);
                return false;
            }
        }else{ 
            printlog(LOG_CRITICAL,"[GENETIC.createChromosomes]","Cannot obtain email data type\n");
            return false;
        }
        counter++;
        i++;
    }
    
    return true;
}

int hashmapToSVector(any_t item2, any_t item, any_t data){
    Svector *vector = (Svector *)item;
    int *insertPosition = (int *)item2;
    char *gen = (char *)data;
     
    addStringAt(vector,*insertPosition,gen);
    (*insertPosition)++;
    return MAP_OK;
}

int deleteKeys(any_t item, any_t key){
    return MAP_OK;
}

int printKey(any_t nullpointer, any_t key){
    printf("%s ",((char *)key));
    return MAP_OK;
}

void maintainUniquesUnsorted(Svector **chromosome){
    int i;
    map_t hashmap = hashmap_new();
    
    for(i=0;i<getStringVectorSize(*chromosome);i++){
        char *aux = NULL;
        if(hashmap_get(hashmap,getStringAt(*chromosome,i),(any_t *)&aux) != MAP_OK){
            char *key = strdup(getStringAt(*chromosome,i));
            hashmap_put(hashmap,key,key);
        }
    }
        
    int *pos = malloc(sizeof(int));
    *pos=0;
    Svector *toret = newStringvector(hashmap_length(hashmap));
    hashmap_iterate_items(hashmap,&hashmapToSVector,toret,pos);
    
    hashmap_free(hashmap);
    freeStringVector(*chromosome);
    free(pos);
    
    *chromosome= toret;
}

int isSpecialToken(char *string){
    if ( startsWith(string,'(') || !strcmp(string,")") || 
         !strcmp(string,"|") ||  startsWith(string,'^'))
        return 1;
    else return 0;
}

void sortGenes(Svector *chromosome){
    long int i;
    long int j=0;
    long int size = getStringVectorSize(chromosome);
    
    for(i=1;i < size; i++){
        for(j=0;j < size-1;j++){
            long int k=j;
            char *elem1=getStringAt(chromosome,k);
            while ( k < size-2 && isSpecialString(elem1) ) elem1=getStringAt(chromosome,++k);
            
            long int l=k+1;
            char *elem2=getStringAt(chromosome,l);
            while (l < size-1 && isSpecialString(elem2) ) elem2=getStringAt(chromosome,++l);
                
            if( k < size && l < size && !isSpecialString(elem2) && 
                !isSpecialString(elem1) && strcmp(elem1,elem2) > 0 )
            {
                char *temp = getStringAt(chromosome,k);
                addStringAt(chromosome,k,getStringAt(chromosome,l));
                addStringAt(chromosome,l,temp);
            }
        }
    }
}

void mutate(Svector **chromosome){
    
    if (getStringVectorSize(*chromosome) > 1){
        long int random = (int)( rand() % getStringVectorSize(*chromosome) );
        long int amount = (int)( rand() % (getStringVectorSize(*chromosome)-random)+1);
        long int insertPos = 0;
        long int i=0;
        Svector *toret = newStringvector(amount+(amount-1)+2);
        
        fisherYatesShuffle(*chromosome,&isSpecialToken);

        addStringAt(toret,insertPos++,strdup("(?:"));
    
        while (i < (amount-1) && random+i < getStringVectorSize(*chromosome) ){
            addStringAt(toret,insertPos++,strdup(getStringAt(*chromosome,random+i)));
            addStringAt(toret,insertPos++,strdup("|"));
            i++;
        }
        
        addStringAt(toret,insertPos++,strdup(getStringAt(*chromosome,random+i)));
        addStringAt(toret,getStringVectorSize(toret)-1,strdup(")"));
        sortGenes(toret);
        
        freeStringVector(*chromosome);        
        *chromosome = toret;
    }
}

void deletegenes(Svector **chromosome){
    if (getStringVectorSize(*chromosome) > 1){
        long int random = (long int)( rand() % getStringVectorSize(*chromosome) );
        long int amount = (long int)( rand() % (getStringVectorSize(*chromosome)-random));
        Svector *aux= spliceStringVector(*chromosome,random,amount);        
        fisherYatesShuffle(*chromosome,&isSpecialToken);
        freeStringVector(*chromosome);
        *chromosome = aux;
    }
}

bool isOpenGroup(char *string){
    return (!strcmp(string,"(") || !strcmp(string,"(?:") || 
            !strcmp(string,"^(") || !strcmp(string,"^(?:"));
}

void removeDuplicates(Svector *chromosome){
    long int i=0;
    char *string = NULL;
    map_t posMap = hashmap_new();
    map_t negMap = hashmap_new();
    map_t queryMap = NULL;
    long int numGroups = 0;
    
    for(;i<getStringVectorSize(chromosome);i++){
        if (strstr(getStringAt(chromosome,i),"(") != NULL) numGroups++;
    }
    
    bool isPositive[numGroups];
    numGroups=-1;
    for(i=0;i<getStringVectorSize(chromosome);i++){
        long int counter = 0;
        long int endPos = i;
        
        if(isOpenGroup(getStringAt(chromosome,endPos))){
            if ( !strcmp(getStringAt(chromosome,endPos),"^(") ||
                 !strcmp(getStringAt(chromosome,endPos),"^(?:") ) 
                isPositive[++numGroups]=false; 
            else{
                if ( !strcmp(getStringAt(chromosome,endPos),"(") || 
                     !strcmp(getStringAt(chromosome,endPos),"(?:") ){ 
                    (numGroups>0 && !isPositive[numGroups])?
                        (isPositive[++numGroups]=false):
                        (isPositive[++numGroups]=true);
                }else if (!strcmp(getStringAt(chromosome,endPos),")")) --numGroups;
            }
        }
            
        while (endPos < getStringVectorSize(chromosome) && 
               !isSpecialString(getStringAt(chromosome,endPos))){
            string = appendstr(string,getStringAt(chromosome,endPos));
            endPos++;
            counter++;
        }

        if (string != NULL){
            char *aux = NULL;
            queryMap = (isPositive[numGroups])?(posMap):(negMap);
            if (hashmap_get(queryMap,string,(any_t *)&aux) == MAP_MISSING){
                hashmap_put(queryMap,string,string);
                string = NULL;        
                i=endPos;
            }else{
                if(!strcmp(getStringAt(chromosome,i-1),"|")){ i--; counter++;}                    
                long int l = i-1, lbalance = 0;
                long int r = i+counter, rbalance = 0;
                long int rJoin=0, lJoin = 0;
 
                while( l > 0 && r < getStringVectorSize(chromosome) && 
                       isSpecialString(getStringAt(chromosome,l)) && 
                       isSpecialString(getStringAt(chromosome,r)) )
                {
                    if ( isOpenGroup(getStringAt(chromosome,l)) ) lbalance++;
                    if ( !strcmp(getStringAt(chromosome,r),")") ) rbalance++;

                    if ( !strcmp(getStringAt(chromosome,l),"|") && 
                         !isSpecialString(getStringAt(chromosome,l+1)) && 
                         !isSpecialString(getStringAt(chromosome,l-1)) )
                        if ( isOpenGroup(getStringAt(chromosome,l-1)) && 
                             isOpenGroup(getStringAt(chromosome,l+1))){
                            lJoin++;
                        }
                    if (!strcmp(getStringAt(chromosome,r),"|")){
                        if ( r == getStringVectorSize(chromosome)-1 || 
                             ( !strcmp(getStringAt(chromosome,r),"|") &&
                               isOpenGroup(getStringAt(chromosome,r-1)) &&
                               isOpenGroup(getStringAt(chromosome,r+1))) ){
                               rJoin++;
                        }
                    }
                    l--;
                    r++;
                }

                if (lbalance > 0 && rbalance > 0){
                    if (lbalance == rbalance || lbalance - rbalance < 0) {
                        i-=lbalance;
                        counter += lbalance*2;
                    }else if (lbalance - rbalance > 0)
                        counter += rbalance;
                }
 
                if ( lJoin == rJoin && lJoin > 1){
                    i-=lJoin-2;
                    counter+=rJoin+2;
                }else if ( lJoin > rJoin && lJoin > 0 ){
                    i-=1; 
                    counter+=1;
                }else if ( rJoin > lJoin && rJoin > 0) counter+=1;

                removeStringInterval(chromosome,i--,counter);
                if( isOpenGroup(getStringAt(chromosome,i-1)) ){
                    long int k=i;
                    while( !isSpecialString(getStringAt(chromosome,k)) && 
                           k < getStringVectorSize(chromosome)) k++;
                    if( !strcmp(getStringAt(chromosome,k),")") ){ 
                        removeStringAt(chromosome,k);
                        removeStringAt(chromosome,i-1);
                    }
                }
            }
            free(string);
            string = NULL;
        }
    }

    hashmap_iterate_keys(posMap,&freeHashmapKey,NULL);
    hashmap_iterate_keys(negMap,&freeHashmapKey,NULL);
    hashmap_free(posMap);
    hashmap_free(negMap);
}

/*Svector *joinChromosomes(Svector *chromosome1, Svector *chromosome2){
    Svector *toret = newStringvector(0);
    long int i=0;
    bool isFirst = true;
    map_t repeated = hashmap_new();
    
    pcre *grep = pcregex_compile("\\)|\\(|\\||\\.\\*");
    for(;i<getStringVectorSize(chromosome1);i++){
        char *prev = getStringAt(chromosome1,i);
        char *aux = NULL;
        if ( !pcregex_match(grep,prev) && 
             hashmap_get(repeated,prev,(any_t*)&aux) == MAP_MISSING )
        {
            if (isFirst){ 
                addNewStringAtEnd(toret,strdup(prev));
                isFirst=false;
            }else{
                addNewStringAtEnd(toret, strdup("|"));
                addNewStringAtEnd(toret, strdup(prev));
            }
            hashmap_put(repeated,prev,prev);
        }
    }
    
    for(i=0;i<getStringVectorSize(chromosome2);i++){
        char *prev = getStringAt(chromosome2,i);
        char *aux = NULL;
        if ( !pcregex_match(grep,prev) && 
             hashmap_get(repeated,prev,(any_t*)&aux) == MAP_MISSING )
        {
            if (isFirst){ 
                addNewStringAtEnd(toret,strdup(prev));
                isFirst=false;
            }else{
                addNewStringAtEnd(toret, strdup("|"));
                addNewStringAtEnd(toret, strdup(prev));
            }
            hashmap_put(repeated,prev,(any_t *)&aux);
        }
    }
    
    pcregex_free(grep);
    
    hashmap_iterate(repeated,&freeNoneKey,NULL);
    hashmap_free(repeated);
    return toret;
}*/

Svector *joinChromosomesInGenes(Svector *chromosome1, Svector *chromosome2){
    Svector *toret = newStringvector(0);
    long int i=0;
    
    map_t repeated = hashmap_new();
    
    pcre *grep = pcregex_compile("\\)|\\(|\\||\\.\\*");
    for(;i<getStringVectorSize(chromosome1);i++){
        char *prev = getStringAt(chromosome1,i);
        char *aux = NULL;
        if ( !pcregex_match(grep,prev) && 
             hashmap_get(repeated,prev,(any_t*)&aux) == MAP_MISSING )
        {
            addNewStringAtEnd(toret,strdup(prev));
            hashmap_put(repeated,prev,prev);
        }
    }
    
    for(i=0;i<getStringVectorSize(chromosome2);i++){
        char *prev = getStringAt(chromosome2,i);
        char *aux = NULL;
        if ( !pcregex_match(grep,prev) && 
             hashmap_get(repeated,prev,(any_t*)&aux) == MAP_MISSING )
        {
            addNewStringAtEnd(toret,strdup(prev));
            hashmap_put(repeated,prev,prev);
        }
    }
    
    pcregex_free(grep);
    
    hashmap_iterate(repeated,&freeNoneKey,NULL);
    hashmap_free(repeated);
    return toret;
}

/*
Svector *crossover2(regex_data *p1, regex_data *p2){
    Svector *father = cloneStringVector(getRegexDataChromosomes(p1));
    Svector *mother = cloneStringVector(getRegexDataChromosomes(p2));
    Svector *toret = NULL;

    
    if( (rand() % 2 ) == 0){
        toret = joinChromosomesInGenes(mother,father);
        mutate(&toret);
    }else{
        //printf("CROSSOVER OPTION NUMBER 2\n");
        //4 cases of mutation:
        //  1. mother + father
        //  2. (?:mother) + father
        //  3. mother + (?:father) 
        //  4. (?:mother) + (?:father)
        // 
        // There is probably simpler/cleaner logic to perform the following.  
        // The goal is to avoid costly nested groups,
        // and keep things in "(foo|bar|baz)" format, and not "((foo|bar)|baz)" format
        // 
        // In mutation occurs via deleting a random amount of genes.  Depending 
        // on the grouping characteristsics, a father, mother, or both 
        // may have genes deleted before they are bred.  If oth parents are 
        // pre-grouped, no deletion occurs
        // 
        // TODO: write a more robust deletion/mutation subroutine that handles 
        // all cases, and mutates all types of children
        // as opposed to pre-mutation some parents only
        //
        char *fatherLastGen = getStringAt(father,getStringVectorSize(father)-1);
        char *motherLastGen = getStringAt(mother,getStringVectorSize(mother)-1);
        
        if ( !strcmp(fatherLastGen,")") && !strcmp(motherLastGen, ")") ){
            if(!strcmp(getStringAt(father,0),"^(?:")) addNewStringAtEnd(mother,strdup("|"));
            else replaceStringAt(mother,getStringVectorSize(mother)-1,strdup("|"));
            if( !strcmp(getStringAt(father,0),"(?:") ) removeStringAt(father,0);
            toret = joinStringVectors(mother,NULL,NULL,father,NULL,NULL);
            removeDuplicates(toret);
        }else if( !strcmp(motherLastGen,")") ){
            deletegenes(&father);
            replaceStringAt(mother,getStringVectorSize(mother)-1,strdup("|"));
            toret = joinStringVectors(mother,NULL,NULL,father,NULL,NULL);
            addNewStringAtEnd(toret,strdup(")"));
            removeDuplicates(toret);
        }else if( !strcmp(fatherLastGen,")") ){
            deletegenes(&mother);
            addNewStringAtBegin(mother,strdup("(?:"));
            addNewStringAtEnd(mother,strdup("|"));
            removeStringAt(father,0);
            toret = joinStringVectors(mother,NULL,NULL,father,NULL,NULL);
            removeDuplicates(toret);
        }else{
            deletegenes(&mother);
            deletegenes(&father);
            addNewStringAtBegin(mother,strdup("(?:"));
            addNewStringAtEnd(mother,strdup("|"));
            toret = joinStringVectors(mother,NULL,NULL,father,NULL,NULL);
            addNewStringAtEnd(toret,strdup(")"));
            removeDuplicates(toret);
        }
    }
    
    freeStringVector(father);
    freeStringVector(mother);
    return toret;
}*/

Svector *crossover(regex_data *p1, regex_data *p2){
    Svector *father = cloneStringVector(getRegexDataChromosomes(p1));
    Svector *mother = cloneStringVector(getRegexDataChromosomes(p2));
    Svector *toret = NULL;
    
    if( (rand() % 2 ) == 0){
        toret = joinChromosomesInGenes(mother,father);
        mutate(&toret);
    }else{
        //printf("CROSSOVER OPTION NUMBER 2\n");
        /*4 cases of mutation:
         *  1. mother + father
         *  2. (?:mother) + father
         *  3. mother + (?:father) 
         *  4. (?:mother) + (?:father)
         * 
         * There is probably simpler/cleaner logic to perform the following.  
         * The goal is to avoid costly nested groups,
         * and keep things in "(foo|bar|baz)" format, and not "((foo|bar)|baz)" format
         * 
         * In mutation occurs via deleting a random amount of genes.  Depending 
         * on the grouping characteristsics, a father, mother, or both 
         * may have genes deleted before they are bred.  If oth parents are 
         * pre-grouped, no deletion occurs
         * 
         * TODO: write a more robust deletion/mutation subroutine that handles 
         * all cases, and mutates all types of children
         * as opposed to pre-mutation some parents only
         */
        char *fatherLastGen = getStringAt(father,getStringVectorSize(father)-1);
        char *motherLastGen = getStringAt(mother,getStringVectorSize(mother)-1);
        
        if ( !strcmp(fatherLastGen,")") && !strcmp(motherLastGen, ")") ){
            replaceStringAt(mother,getStringVectorSize(mother)-1,strdup("|"));
            toret = concatStringVectors(mother,father,1,getStringVectorSize(father));
            removeDuplicates(toret);
        }else if( !strcmp(motherLastGen,")") ){
            deletegenes(&father);
            replaceStringAt(mother,getStringVectorSize(mother)-1,strdup("|"));
            toret = concatStringVectorsAddEnd(mother,father,0,getStringVectorSize(father),strdup(")"));
            removeDuplicates(toret);
        }else if( !strcmp(fatherLastGen,")") ){ //VOY AQUI!!
            deletegenes(&mother);
            addNewStringAtEnd(mother,strdup("|"));
            toret = concatStringVectorsAddBegin(mother,father,1,getStringVectorSize(father),strdup("(?:"));
            removeDuplicates(toret);
        }else{
            deletegenes(&mother);
            deletegenes(&father);
            addNewStringAtEnd(mother,strdup("|"));
            toret = concatStringVectorsAddBeginEnd(mother,father,0,
                        getStringVectorSize(father),strdup("(?:"),strdup(")"));
            removeDuplicates(toret);
        }
    }
    
    freeStringVector(father);
    freeStringVector(mother);
    return toret;
}

int addToMatingPool(element item, element data){
    regex_data *rData = (regex_data *)data;
    sMatingPool *pool = (sMatingPool *)item;
    int numSlots = (int)round(fabs(getRegexDataScore(rData)));
    int i=0;
    for(;i<numSlots;i++){ 
        if(addMatingElementAtEnd(pool,rData) == MFAIL){
            printlog(LOG_CRITICAL,"[GENETIC.addToMatingPool]", "Cannot insert element in Matting Pool\n"); 
        }
    }
    return NODE_OK;
}

int breedIndividuals(element item2, element item, element data){
    linklist *childs = (linklist *)item2;
    genetic_info *genInfo = (genetic_info *)item;
    sMatingPool *pool = genInfo->breed->pool;
    regex_data *father = (regex_data *)data;
    regex_data *mother = NULL;

    if (father == NULL || 
        getMatingElement(pool,(long int)(rand() % getMatingPoolSize(pool)),(matingElement *)&mother) == MFAIL ||
        mother == NULL){
        printlog(LOG_CRITICAL,"[GENETIC.breedIndividuals]", "Cannot obtain individual from mating pool\n"); 
    }else{
        Svector *child = optimizeGenes(crossover(father,mother),false);
        char *pattern = NULL;
        if ( getStringVectorSize(child) >0 && (pattern = stringVectorToString(child)) != NULL){
            pcre *regexp = NULL;
            char *aux;
            if ( (hashmap_get(genInfo->precompiled_regex,pattern,(any_t *)&aux) == MAP_OK) ||
                 (regexp = pcregex_compile(pattern)) == NULL ){
                printlog(LOG_WARNING,"[GENETIC.breedIndividuals]","Chromosome '%s' already exists\n",pattern);
                freeStringVector(child);
                free(pattern);
                pcregex_free(regexp);
            }else{
                hashmap_put(genInfo->precompiled_regex,pattern,regexp);
                if ( !isMatching(regexp,getHamLines(genInfo->data)) ){
                    long int numMatches;
                    if ( (numMatches = match(regexp,getSpamLines(genInfo->data))) > 0){
                        addendlist( childs,newRegexData( pattern,regexp,numMatches,
                                    (numMatches*scoreregexes(pattern)),child) );
                    }else{
                        freeStringVector(child);
                        printlog(LOG_WARNING,"[GENETIC.breedIndividuals]","Regex '%s' does not match in spam content\n",pattern);
                    }
                }else{
                    freeStringVector(child);
                    printlog(LOG_WARNING,"[GENETIC.breedIndividuals]","Regex '%s' match in ham content\n",pattern);
                }
            }
        }else{
            freeStringVector(child);
            printlog(LOG_CRITICAL,"[GENETIC.breedIndividual]", "Child is '%s' incorrect\n",pattern); 
        }
    }
    
    return NODE_OK;
}

int insertSortedChilds(element item, element data){
    genetic_info *genInfo = (genetic_info *)item;
    dlinkedhashmap *population = genInfo->population;
    regex_data *rData = (regex_data *)data;
    
    (add_dlhSortedElement(population,strdup(getRegexDataPattern(rData)),rData,&descSort)!= LH_EXIST)?
        (genInfo->breed->numSlots+=(int)round(fabs(getRegexDataScore(rData)))):(0); //MIRAR ESTO!!!!
    return NODE_OK;
}

void breedPopulation(genetic_info *genInfo){
    if ( (genInfo->breed->pool = newSMatingPool(genInfo->breed->numSlots)) != NULL) {
        // Refill Mating Pool with elements (chromosomes).
        // Populate the breeding pool -> Use 'Roulette wheel' selection, based on the regex fitness.  
        // A score of 1 gives 1 slot in the breeding pool, score of 3 gives 3 slots.
        printlog(LOG_INFO,"[GENETIC.breedPopulation]","Creating Mating Pool\n");
        dlh_iterateData(genInfo->population,&addToMatingPool,genInfo->breed->pool);
        
        //Breed the population.
        linklist *childs = newlinkedlist();
        printlog(LOG_INFO,"[GENETIC.breedPopulation]","Breeding chromosomes\n");
        dlh_iterateDataTwo(genInfo->population,&breedIndividuals,genInfo,childs);
        
        //Add childs to the global population.
        printlog(LOG_INFO,"[GENETIC.breedPopulation]","Adding valid childs to population\n");
        linklist_iterate_data(childs,&insertSortedChilds,genInfo);
        freelist(childs,NULL);
        freeMattingPool(genInfo->breed->pool,NULL);
    }else printlog(LOG_WARNING,"[GENETIC.breedPopulation]","Error creating Mating Pool. Cannot breed population\n");
}

int freeIndividual(element item, element data){
    breedData *breed = (breedData *) item;
    regex_data *rData = (regex_data *)data;
    breed->numSlots-= (int)round(fabs(getRegexDataScore(rData)));
    freeRegexData(rData);
    return NODE_OK;
}

void killPopulation(genetic_info *genInfo){
    if (get_dlhlinkedlistSize(genInfo->population) > genInfo->max_population){
        long int amount = (get_dlhlinkedlistSize(genInfo->population) - genInfo->max_population);
        printlog(LOG_INFO,"[GENETIC.killPopulation]","Removing worst population\n");
        dlh_reverseOperateRemoveData(genInfo->population,amount,&freeIndividual,genInfo->breed);
    }
}