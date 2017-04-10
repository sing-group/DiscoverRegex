/****************************************************************
*
*   File    : run_regex.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include "filedata.h"
#include "genetic_types.h"
#include "string_util.h"
#include "pcre_regex_util.h"
#include "dlinkedhashmap.h"
#include "common_dinamic_structures.h"
#include "fileutils.h"
#include "string_vector.h"
#include "genetic.h"
#include "logger.h"
#include "dictionary.h"
#include "list_files.h"
#include "iniparser.h"
#include "experimental.h"
#include "linkedhashmap.h"

#define nGenerations 30

#define MAX_POPULATION(x) ((x*50)/100)


int freePreCompiledRegex(any_t item, any_t data, any_t key){
    if ( ((pcre *)data) != NULL){ 
        pcregex_free((pcre *)data);
        data = NULL;
    }
    if ( ((char *)key) != NULL){ 
        free(key);
        key = NULL;
    }
    return MAP_OK;
}
experiment *loadFilesFromDir(){
    experiment *toret  = NULL;
    
    toret = createExpemimentData(1);
    
    addTrnSpamAt(toret,0,createString("dst/trn_.spam.txt"));
    addTstSpamAt(toret,0,createString("dst/tst_.spam.txt"));
    addTrnHamAt(toret,0,createString("dst/trn_.ham.txt"));
    addTstHamAt(toret,0,createString("dst/tst_.ham.txt"));
    
    return toret;
}

int isRegexMatch(element item, element data){
    regex_data *rData = (regex_data *)data;
    char *email = (char *)item;
    
    if (email == NULL || rData == NULL)
        return 0;
    else return (pcregex_match(getRegexDataPCRE(rData),email)>0);
}

int obtainResult(element item, element data){
    linklist *list = (linklist *)item;
    regex_data *rData = (regex_data *)data;
    if (getRegexDataScore(rData) > 1.0F){
        addendlist(list,rData);
        return NODE_OK;
    }else return NODE_FAIL;
}

void computeMeasures(experiment *exp, dlinkedhashmap *lh, int folder){
    long int nEmail;
    linklist *list = NULL;
    if ( exp != NULL && getStringVectorSize(getTstHamLines(get_tst_data(exp)))>0 && 
         getStringVectorSize(getTstSpamLines(get_tst_data(exp))) >0 && lh!=NULL && 
         get_dlhlinkedlistSize(lh)>0 )
    {   
        list = newlinkedlist();
        dlh_iterateData(lh,&obtainResult,list);
        printlog(LOG_INFO,"[MAIN]","[2.1] - Total number of valid regex: (%ld)\n",getlengthlist(list));
        
        long int nham = getStringVectorSize(getTstHamLines(get_tst_data(exp)));
        long int nspam = getStringVectorSize(getTstSpamLines(get_tst_data(exp)));
        long int nregex = getlengthlist(list);
        printlog(LOG_INFO,"[MAIN]","[2.2] - Test Corpus Information\n\tNumber of Ham E-mails %ld\n\tNumber of Spam E-mails %ld\n",nham,nspam);
        
        setMatrixNhamAt(exp,folder,nham);
        setMatrixNspamAt(exp,folder,nspam);
        setMatrixNregexAt(exp,folder,nregex);
        
        printlog(LOG_INFO,"[MAIN]","[2.3] - Checking HAM e-mails...\n");
        for(nEmail=0;nEmail < getStringVectorSize(getTstHamLines(get_tst_data(exp)));nEmail++){
            switch(linklistHasCondition(list,&isRegexMatch,getStringAt(getTstHamLines(get_tst_data(exp)),nEmail))){
                case NODE_FOUND: setMatrixFPAt(exp,folder,getMatrixFPAt(exp,folder)+1);
                     break;
                case NODE_MISSING: setMatrixTnAt(exp,folder,getMatrixTnAt(exp,folder)+1);
                     break;
            }
        }
        printlog(LOG_INFO,"[MAIN]","[2.4] - Checking SPAM e-mails...\n");
        for(nEmail=0;nEmail < getStringVectorSize(getTstSpamLines(get_tst_data(exp)));nEmail++){
            switch(linklistHasCondition(list,&isRegexMatch,getStringAt(getTstSpamLines(get_tst_data(exp)),nEmail))){
                case NODE_FOUND: setMatrixTpAt(exp,folder,getMatrixTpAt(exp,folder)+1);
                     break;
                case NODE_MISSING: setMatrixFNAt(exp,folder,getMatrixFNAt(exp,folder)+1);
                     break;
            }
        }
        freelist(list,NULL);
    }else printlog(LOG_WARNING,"[MAIN]","Chromosome array is empty. Cannot compute measures\n");
}

int freePopulation(element element){
    regex_data *rData = (regex_data *)element;
    freeRegexData(rData);
    return LH_OK;
}

int writePopulation(element item, element data){
    FILE *fp = (FILE *) item;
    regex_data *rData = (regex_data *)data;
    
    fprintf(fp,"%s\n",getRegexDataPattern(rData));
    return NODE_OK;
}

void populationToFile(dlinkedhashmap *population, int folder){
    char *str = createString("results/rules/rules.f%d.out",folder);
    FILE *fp = fopen(str,"w+");
    if(!fp)
        printlog(LOG_CRITICAL,"[MAIN]","Cannot open 'rules.out' file\n");
    else {
        printf("-------------------------------------------\n");
        dlh_iterateData(population,&writePopulation,fp);
        printf("\n");
        fclose(fp);
    }
    free(str);
}

int main(int argc, char** argv) {
    genetic_info *genInfo = NULL;
    long int numGenerations = nGenerations;
    experiment *exp= NULL ;
    int i =0;
      
    start_logger(createLogSettings("run_regex.log",LOG_INFO,STDOUT,FILEOUT,STDOUT,FILEOUT), "run_regex");
    
    srand(time(NULL));
    printlog(LOG_INFO,"[MAIN]","Initializing Seed\n");
    if ( (exp = loadFilesFromDir()) == NULL ||  
         (genInfo = malloc(sizeof(genetic_info))) == NULL ||
         (genInfo->breed = malloc(sizeof(breedData))) == NULL ){
        printlog(LOG_CRITICAL,"[MAIN]","Cannot allocate enough memmory to store main data\n");
        stop_logger();
        return EXIT_FAILURE;
    }
    
    genInfo->static_regex = staticPCREVector();
    genInfo->population = NULL;
    
    for(i=0;i<getIterations(exp);i++){
        //INITIALIZING GENREGEX DATA
        
        genInfo->breed->numSlots = 0;
        printlog(LOG_INFO,"[MAIN]","[1] - Starting Regex Generation from fold %d/%d\n",i+1,nGenerations);
        printlog(LOG_INFO,"[MAIN]","[1.1] - Loading training files '%s', '%s'\n",getTrnSpamAt(exp,i),getTrnHamAt(exp,i));
        genInfo->data=loadToFileData(getTrnSpamAt(exp,i),getTrnHamAt(exp,i));
        printlog(LOG_INFO,"[MAIN]","[1.2] - Loading testing files '%s', '%s'\n",getTstSpamAt(exp,i),getTstHamAt(exp,i));
        exp->test_emails=loadTstData(getTstSpamAt(exp,i),getTstHamAt(exp,i));
        genInfo->max_population=MAX_POPULATION(getStringVectorSize(getSpamLines(genInfo->data)));
        genInfo->precompiled_regex = hashmap_new();
        genInfo->population=new_dlinkedhashmap();
        shuffleGenericVector(getSpamData(genInfo->data));
        //GENERATING REGEX
        float fract = (float)((getGenericVectorSize(getSpamData(genInfo->data))*1.0)/numGenerations);
        int actualGeneration = 0;
        printlog(LOG_INFO,"[MAIN]","[1.3] - MAX POPULATION IS %ld\n",genInfo->max_population);
        printlog(LOG_INFO,"[MAIN]","[1.4] - Using %d generations to create SPAM regex\n",nGenerations);

        int offset = (int)(actualGeneration*fract);
        
        while(actualGeneration < nGenerations){
            printlog(LOG_INFO,"[MAIN]","-----------------------------------------\n");
            printlog(LOG_INFO,"[MAIN]","[1.4.1] - Generation %d\n",actualGeneration);
            if (!createChromosomes( genInfo,offset,(int)(ceil(offset+fract-1))) ){ 
                printlog(LOG_WARNING,"[MAIN]","[1.4.2] - Cannot generate chromosomes from spam email\n");
                offset = (int)(++actualGeneration * fract);
                continue;
            }
            breedPopulation( genInfo );            
            killPopulation( genInfo );
            printlog(LOG_INFO,"[MAIN]","[1.4.3] - Population is %ld/%ld\n",
                     get_dlhlinkedlistSize(genInfo->population),genInfo->max_population);
            offset = (int)(++actualGeneration * fract);
        }
        
        printlog(LOG_INFO,"[MAIN]","[1.5] - Finish Regex Generation on fold\n");
        printlog(LOG_INFO,"[MAIN]","[2] - Computing accuracy measures\n");
        computeMeasures(exp,genInfo->population,i);
        confusionMatrixToFile(exp,i);
        printlog(LOG_INFO,"[MAIN]","[3] - Finish computation of accuracy measures\n");
        printlog(LOG_INFO,"[MAIN]","[4] - Writting Regex to file\n");
        populationToFile(genInfo->population,i);
        printlog(LOG_INFO,"[MAIN]","[5] - Freeing space to compute next fold.\n");
        free_dlh(genInfo->population,&freePopulation);
        freeTstData(exp->test_emails);
        hashmap_iterate_elements(genInfo->precompiled_regex,&freePreCompiledRegex,NULL);
        hashmap_free(genInfo->precompiled_regex);    
        freeFileData(genInfo->data);
    }
    
    freeCompileRegex(genInfo->static_regex);
    printConfusionMatrix(exp);
    freeExperimentData(exp);
    free(genInfo->breed);
    free(genInfo);
    stop_logger();
}