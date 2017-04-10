#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include "pcre_regex_util.h"
#include "string_util.h"
#include "hashmap.h"
#include "linked_list.h"
#include "logger.h"

#define OVECCOUNT 3000
#define WSCCOUNT 3000

//MACROS FOR compileRegex

#define PCRE_SUBJECT    0 //"subject"
#define PCRE_NUMBERS    1 //"numbers"
#define PCRE_UPHEXDEC   2 //"upperHexDec"
#define PCRE_LOHEXDEC   3 //"lowerHexDec"
#define PCRE_DOMAIN     4 //"networkDomain"
#define PCRE_LOLETTERS  5 //"lowerLetters"
#define PCRE_CAPLETTERS 6 //"capitalLetters"
#define PCRE_WEEKDAYS   7 //"weekDays"
#define PCRE_MONTHS     8 //"months"
#define PCRE_FIRSTCAP   9 //"firstCap"
#define PCRE_ISNUMBER   10 //"isNumber"
#define PCRE_ISUPALPHA  11 //"isUpperAlpha"
#define PCRE_ISLOALPHA  12 //"isLowerAlpha"
#define PCRE_ISTAB      13 //"isTab"
#define PCRE_ISBLANKS   14 //"isBlanks"
#define PCRE_SINGLEINT  15 //"isSingleNumber"
#define PCRE_SINGLEUPCHAR 16 //"isSingleUpperChar"
#define PCRE_SINGLELOCHAR 17 //"isSingleLowerChar"

//MACROS FOR staticPCREVector

#define PCRE_IS_REPEATED_CHAR  0
#define PCRE_IS_NUMBERS        1
#define PCRE_IS_UPHEX          2
#define PCRE_IS_LOHEX          3
#define PCRE_IS_DOMAIN         4
#define PCRE_IS_LOLETTERS      5
#define PCRE_IS_CAPLETTERS     6
#define PCRE_IS_WEEKDAYS       7
#define PCRE_IS_MONTHS         8
#define PCRE_IS_FIRSTCAP       9
#define PCRE_IS_CASH           10
#define PCRE_IS_SINGLEINT      11
#define PCRE_IS_SINGLEUPCHAR   12
#define PCRE_IS_SINGLELOCHAR   13
#define PCRE_IS_GREP           14

#define MAX_PCRE_SIZE 500

struct pcre_vector_data_t {
    pcre **vector;
    int size;
};

struct regex_data_t{
    int hit_counter;
    pcre *compiled_regex;
    char *regexString;
    Svector *chromosomes;
    double score;
};

pcre *pcregex_compile(const char *pattern){
    const char *error;
    int erroffset;
    pcre *reg=NULL;
    
    if( (reg = pcre_compile(pattern,0,&error,&erroffset,NULL)) == NULL){
        printlog(LOG_CRITICAL,"[PCRE_REGEX_UTIL.pcregex_compile]", "Regex not compiled - failed at offset %d: %s\n", erroffset, error);
        printf("PATTERN %s\n",pattern);
    }
        
    return reg;
}


int pcregex_match(const pcre *reg, const char *string){
    int ovector[OVECCOUNT];
    const char *pcreErrorStr;
    int opt;
    
    if (string == NULL) return 0;
    
    pcre_extra *extra = pcre_study(reg,PCRE_EXTRA_MATCH_LIMIT,&pcreErrorStr);
    
    if(pcreErrorStr != NULL){
        printlog(LOG_CRITICAL,"[PCRE_REGEX_UTIL.pcregex_match]","Could not study regex. Error code: %s\n", pcreErrorStr);
        return 0;
    } 
    
    if( (opt = pcre_exec(reg,extra,string,strlen(string),0,PCRE_PARTIAL_HARD,ovector,OVECCOUNT)) < 0){
        pcre_free_study(extra);
        switch(opt){
            case PCRE_ERROR_NULL         : printf("Something was null\n");                      break;
            case PCRE_ERROR_BADOPTION    : printf("A bad option was passed\n");                 break;
            case PCRE_ERROR_BADMAGIC     : printf("Magic number bad (compiled re corrupt?)\n"); break;
            case PCRE_ERROR_UNKNOWN_NODE : printf("Something kooky in the compiled re\n");      break;
            case PCRE_ERROR_NOMEMORY     : printf("Ran out of memory\n");                       break;
            case PCRE_ERROR_MATCHLIMIT   : printf("Backtracking limit\n");                      break;
        }
        return 0;
    }
    else{ 
        pcre_free_study(extra); 
        return 1;
    }
}

char *pcregex_replace(char *src, pcre *re, char *replace){
    int offset=0;
    int ovector[OVECCOUNT];
    char *strdest=NULL;
    int strsize = strlen(src);
    char *str = NULL;
    
    if (src==NULL || re==NULL){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.pcregex_replace]","REGEX or STRING is NULL\n");
        return NULL;
    }

    if (strlen(src)==0) return strdest;
    while ( (pcre_exec(re, NULL, src, strsize, offset, 0, ovector, OVECCOUNT) > 0) && (offset <= strsize) ) 
    {
        if(ovector[0] > offset){
            str = getSubstring(src,offset,ovector[0]-offset);
            strdest=appendstr(strdest,str);
            free(str);
        }
        strdest=appendstr(strdest,replace);
        offset = ovector[1];
    }
    if(offset < strsize){
        str = getSubstring(src,offset,(strsize-offset));
        strdest=appendstr(strdest,str);
        free(str);
    }
    
    free(src);
    return strdest;
}

char scapedChar(char *ch){
    if(ch[0]=='\0' || ch[1]== '\0') 
        return '\0';
    else{ 
        if (ch[0] == '\\' && isSpecialChar(ch[1]) ) 
            return ch[1];
        else return '\0';
    }
}

char *scapeMetaCharacters(char *string){
    int pos = 0;
    char *toret = NULL;
    while(string[pos] != '\0'){
        if(isSpecialChar(string[pos])){
            toret=appendchar(toret,'\\');
        }
        toret = appendchar(toret,string[pos]);
        pos++;
    }
    
    return toret;
}

char *pcregex_compress(char *src, pcre_type type){
    char *toret = NULL;
    
    if (src == NULL) return toret;

    if (strlen(src) < 4) 
        toret = strdup(src);
    else{
        char *ptr = src;
        while( *ptr != '\0' ){
            char *end = ptr;
            if( scapedChar(ptr) !='\0'){
                int count= 0;
                while( scapedChar(ptr)!= '\0' && scapedChar(end+2)!= '\0' &&
                       scapedChar(ptr) == scapedChar(end+2) ){ 
                    end+=2;
                    count++;
                }
                
                if(count > 0){
                    (type == BOUNDED)?
                        (toret=concat(toret,"\\%c{%d}",scapedChar(ptr),count+1)):
                        (toret=concat(toret,"\\%c+",scapedChar(ptr)));
                    ptr=end+1;
                }else toret = appendchar(toret,*ptr);
            }//else toret = appendchar(toret,*ptr);
            else {
                char *end = ptr;
                char ch = *ptr;
                int count = 0;
                while(*ptr != '\0' && ch == *(end+1)) {
                    end++; 
                    count++;
                }
                if(count > 0){
                    (type == BOUNDED)?
                        (toret=concat(toret,"%c{%d}",ch,count+1)):
                        (toret=concat(toret,"%c+",ch));
                    ptr=end;
                }else toret = appendchar(toret,*ptr);
            }
            ptr++;
        }    
    }   
    
    free(src);
    return toret;
    
}

bool isSpecialChar(char ch){
    if ( ch == '[' || ch== ']' || ch == '{' || ch == '}' || ch == '(' || 
        ch == ')' || ch == '^' || ch == '$' || ch == '|' || ch == '+' || 
        ch == '?' || ch =='.' || ch =='*' || ch =='\\' || ch == '/' )
        return true;
    else return false;
}

bool isSpecialString(char *string){
    if ( !strcmp(string,"(") || !strcmp(string,"(?:") || !strcmp(string,"^(") || 
         !strcmp(string,"^(?:") || !strcmp(string,")") || !strcmp(string,"|"))
        return true;
    else return false;
}

int startsWith(char *string, char ch){
    return string[0]==ch;
}

void pcregex_free(pcre *reg){
    if(reg!=NULL){
        pcre_free(reg);
        reg = NULL;
    }
}

bool containsURL(char *str){
   pcre * reg= pcregex_compile("(https?:\\/\\/www\\.|http:\\/\\/|https:\\/\\/|www\\.|ftps?:\\/\\/)?([a-z0-9]+@)?[a-z0-9]+([\\-\\.]{1}[a-z0-9]+)*\\.[a-z]{2,5}(:[0-9]{2,5})?(\\/.*)?"); 
   bool matched = false;
   if (reg != NULL){ 
       matched = (pcregex_match(reg,str) > 0);
       pcregex_free(reg);
   }
   return matched;
}

pcre_vector *compileRegex(){
    
    pcre_vector *toret = malloc(sizeof(pcre_vector));
    toret->vector = (pcre **)malloc(sizeof(pcre *)*18);
    toret->size = 18;
    
    if ( (toret->vector[PCRE_SUBJECT] = pcregex_compile("^[-\\w]+?:$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^[-\\w]+?:$");
        return NULL;
    }
   
    if ( (toret->vector[PCRE_NUMBERS] = pcregex_compile("^\\d+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^\\d+?$");
        return NULL;
    }
   
    if ( (toret->vector[PCRE_UPHEXDEC] = pcregex_compile("^[A-F0-9]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^[A-F0-9]+?$");
        return NULL;
    }
   
    if ( (toret->vector[PCRE_LOHEXDEC] = pcregex_compile("^[a-f0-9]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^[a-f0-9]+?$");
        return NULL;
    }

    if ( (toret->vector[PCRE_DOMAIN] = pcregex_compile("^(com|net|org|edu|biz|info|us|es)$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^(com|net|org|edu|biz|info|us|es)$");
        return NULL;
    }

    if ( (toret->vector[PCRE_LOLETTERS] = pcregex_compile("^[a-z]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^[a-z]+?$");
        return NULL;
    }

    if ( (toret->vector[PCRE_CAPLETTERS] = pcregex_compile("^[A-Z]+?$")) == NULL){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^[A-Z]+?$");
        return NULL;
    }

    if ( (toret->vector[PCRE_WEEKDAYS] = pcregex_compile("^(Mon|Tue|Wed|Thu|Fri|Sat|Sun)$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^(Mon|Tue|Wed|Thu|Fri|Sat|Sun)$");
        return NULL;
    }

    if ( (toret->vector[PCRE_MONTHS] = pcregex_compile("^(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)$");
        return NULL;
    }

    if ( (toret->vector[PCRE_FIRSTCAP] = pcregex_compile("^[A-Z][a-z]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX ^[A-Z][a-z]+?$");
        return NULL;
    }

    if ( (toret->vector[PCRE_ISNUMBER] = pcregex_compile("^[0-9]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX [0-9]");
        return NULL;
    }

    if ( (toret->vector[PCRE_ISUPALPHA] = pcregex_compile("[A-Z]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX [A-Z]");
        return NULL;
    }

    if ( (toret->vector[PCRE_ISLOALPHA] = pcregex_compile("[a-z]")) == NULL ){
        printf("[ERROR][PCRE_REGEX]: Cannot compile REGEX [a-z]");
        return NULL;
    }

    if ( (toret->vector[PCRE_ISTAB] = pcregex_compile("\\t")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX \\t");
        return NULL;
    }

    if ( (toret->vector[PCRE_ISBLANKS] = pcregex_compile(" +?")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX  +?");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_SINGLEINT] = pcregex_compile("[0-9]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX [0-9]");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_SINGLEUPCHAR] = pcregex_compile("[A-Z]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX [A-Z]");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_SINGLELOCHAR] = pcregex_compile("[a-z]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.compileRegex]","Cannot compile REGEX [a-z]");
        return NULL;
    }
    
    return toret;
}

pcre_vector *staticPCREVector(){
    
    
    pcre_vector *toret = (pcre_vector *)malloc(sizeof(pcre_vector));
    toret->vector = (pcre **)malloc(sizeof(pcre *)*15);
    toret->size = 15;
    
    if ( (toret->vector[PCRE_IS_REPEATED_CHAR] = pcregex_compile("^(.)\\1{2,}$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^(.)\\1{2,}$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_NUMBERS] = pcregex_compile("^\\d+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^\\d+?$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_UPHEX] = pcregex_compile("^[A-F0-9]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^[A-F0-9]+?$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_LOHEX] = pcregex_compile("^[a-f0-9]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^[a-f0-9]+?$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_DOMAIN] = pcregex_compile("^(?:com|net|org|edu|biz|info|us|es|gal|uk|info)$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^(?:com|net|org|edu|biz|info|us|es|gal|uk|info)$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_LOLETTERS] = pcregex_compile("^[a-z]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^[a-z]+?$/");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_CAPLETTERS] = pcregex_compile("^[A-Z]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^[A-Z]+?$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_WEEKDAYS] = pcregex_compile("^(?:Mon|Tue|Wed|Thu|Fri|Sat|Sun)$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^(?:Mon|Tue|Wed|Thu|Fri|Sat|Sun)$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_MONTHS] = pcregex_compile("^(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_FIRSTCAP] = pcregex_compile("^[A-Z][a-z]+?$")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^[A-Z][a-z]+?$");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_CASH] = pcregex_compile("^(US)?\\$[0-9,]+?")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX ^(US)?\\$[0-9,]+?");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_SINGLEINT] = pcregex_compile("[0-9]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX [0-9]");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_SINGLEUPCHAR] = pcregex_compile("[A-Z]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX [A-Z]");
        return NULL;
    }
    
    if ( (toret->vector[PCRE_IS_SINGLELOCHAR] = pcregex_compile("[a-z]")) == NULL ){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX [a-z]");
        return NULL;
    }
    
    if(  (toret->vector[PCRE_IS_GREP] = pcregex_compile("\\)|\\(|\\||\\.\\*")) == NULL){
        printlog(LOG_CRITICAL,"[PCRE_REGEX.staticPCREVector]","Cannot compile REGEX [a-z]");
        return NULL;
    }
    
    return toret;
}

void freeCompileRegex(pcre_vector *compiledRegex){
    if(compiledRegex != NULL && compiledRegex->vector != NULL){
        int i=0;
        for(;i<compiledRegex->size;i++) pcregex_free(compiledRegex->vector[i]);
    }
    free(compiledRegex->vector);
    free(compiledRegex);
}


void freeRegexData(regex_data *data){
    if(data != NULL){
        if(data->regexString != NULL){
            free(data->regexString);
            data->regexString = NULL;
        }
        if(data->chromosomes != NULL){
            freeStringVector(data->chromosomes);
            data->chromosomes = NULL;
        }
        free(data);
    }
}

regex_data *newRegexData(char *pattern,pcre *regex, int hitCounter, double score, Svector *chr){
    regex_data *toret = malloc(sizeof(regex_data));
    toret->compiled_regex=regex;
    toret->regexString=strdup(pattern);
    toret->hit_counter= hitCounter;
    toret->score = score;
    toret->chromosomes = chr;
    
    return toret;
}

void setRegexDataScore(regex_data *rData, double score){
    if(rData != NULL){
        rData->score = score;
    }else printlog(LOG_CRITICAL,"[PCRE_REGEX.setRegexDataScore]","Cannot set score value in RegexData\n");
}

void setRegexDataChromosomes(regex_data *rData, Svector *chr){
    if(rData != NULL){
        rData->chromosomes = chr;
    }else printlog(LOG_CRITICAL,"[PCRE_REGEX.setRegexDataChromosomes]","Cannot set chromosomes vector in RegexData\n");
}

char *getRegexDataPattern(regex_data *rData){
    if (rData != NULL && rData->regexString != NULL){
        return rData->regexString;
    }    
    return NULL;
}

pcre *getRegexDataPCRE(regex_data *rData){
    if (rData != NULL && rData->compiled_regex != NULL){
        return rData->compiled_regex;
    }    
    return NULL;
}

int getRegexDataHitCounter(regex_data *rData){
    if (rData != NULL){
        return rData->hit_counter;
    }    
    return 0;
}

double getRegexDataScore(regex_data *rData){
    if (rData != NULL){
        return rData->score;
    }    
    return 0;
}

Svector *getRegexDataChromosomes(regex_data *rData){
    if (rData != NULL){
        return rData->chromosomes;
    }    
    return NULL;
}

char *generateRegExp(Svector *words, pcre_vector *regexVector){    
    char *newString = NULL;
    int first = 1;
    long int i=0;
    for(;i<getStringVectorSize(words);i++){
        char *word = getStringAt(words,i);
        int wordlenght = strlen(word);
        if( first && pcregex_match(regexVector->vector[PCRE_SUBJECT],word)){ //Match string as subject
            if(newString!=NULL){ 
                free(newString);
                newString = NULL;
            }
            newString=concat(newString,"^%s",word);
            first = 0;
        }else if (wordlenght > 1) { //If token is a word (> 1 char)
            if(pcregex_match(regexVector->vector[PCRE_NUMBERS],word)){ //Numbers
                newString=appendstr(newString,"\\d+?"); 
            }else if (pcregex_match(regexVector->vector[PCRE_UPHEXDEC],word)){ //Hex with capital letters
                newString=concat(newString,"[A-F0-9]{%d}",wordlenght);
            }else if (pcregex_match(regexVector->vector[PCRE_LOHEXDEC],word)){ //Hex with lowercase letters
                newString=concat(newString,"[a-f0-9]{%d}",wordlenght);
            }else if (pcregex_match(regexVector->vector[PCRE_DOMAIN],word)){ //domain
                newString=appendstr(newString,"(com|net|org|edu|biz|info|us|es)");
            }else if (pcregex_match(regexVector->vector[PCRE_LOLETTERS],word)){ //lower case letters
                newString=appendstr(newString,"[a-z]+?");
            }else if (pcregex_match(regexVector->vector[PCRE_CAPLETTERS],word)){ //Capital letters
                newString=appendstr(newString,"[A-Z]+?");
            }else if (pcregex_match(regexVector->vector[PCRE_WEEKDAYS],word)){ //Days of the Week
                newString = appendstr(newString,"(Mon|Tue|Wed|Thu|Fri|Sat|Sun)");
            }else if (pcregex_match(regexVector->vector[PCRE_MONTHS],word)){ //Months
                newString = appendstr(newString,"(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)");
            }else if (pcregex_match(regexVector->vector[PCRE_FIRSTCAP],word)){ //First capilalized letter
                newString = appendstr(newString,"[A-Z][a-z]+?");
            }else newString = appendstr(newString,word); 
        }else{ //Token is 1 character lenght
            if( pcregex_match(regexVector->vector[PCRE_SINGLEINT],word)){
                newString = appendstr(newString,"[0-9]");
            }else if( pcregex_match(regexVector->vector[PCRE_SINGLEUPCHAR],word) ){
                newString = appendstr(newString,"[A-Z]");
            }else if( pcregex_match(regexVector->vector[PCRE_SINGLELOCHAR],word) ){
                newString = appendstr(newString,"[a-z]");
            }else if ( pcregex_match(regexVector->vector[PCRE_ISTAB],word) ){
                newString = appendstr(newString,"\\t");
            }else{ newString = appendstr(newString,word); }
        }
    }

    return (newString = pcregex_compress(newString,UNBOUNDED));
    //return (newString = pcregex_replace(newString,regexVector[PCRE_ISBLANKS]," +"));
}

char *generateRegexGen(char *word,pcre_vector *precompiledRegex){
    int wordlen = strlen(word);
    char *toret = NULL;
    if( wordlen > 1){
        if(pcregex_match(precompiledRegex->vector[PCRE_IS_REPEATED_CHAR],word)){
            toret = concat(toret,"%c{%d}",word[0],wordlen);
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_NUMBERS],word)){
            toret =appendstr(toret,"\\d+?"); 
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_UPHEX],word)){
            toret = concat(toret,"[A-F0-9]{%d}",wordlen);
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_LOHEX],word)){
            toret = concat(toret,"[a-f0-9]{%d}",wordlen);
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_DOMAIN],word)){
            toret=appendstr(toret,"(?:com|net|org|edu|biz|info|us|es|gal|uk|info)");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_LOLETTERS],word)){
            toret=appendstr(toret,"[a-z]+?");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_CAPLETTERS],word)){
            toret=appendstr(toret,"[A-Z]+?");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_WEEKDAYS],word)){
            toret=appendstr(toret,"(?:Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_FIRSTCAP],word)){
            toret=appendstr(toret,"[A-Z][a-z]+?");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_CASH],word)){
            toret=appendstr(toret,"(US)?\\$[0-9,]+?");
        }else toret = strdup(word);
    }else{
        if (pcregex_match(precompiledRegex->vector[PCRE_IS_SINGLEINT],word)){
            toret = appendstr(toret,"[0-9]");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_SINGLEUPCHAR],word)){
            toret = appendstr(toret,"[A-Z]");
        }else if (pcregex_match(precompiledRegex->vector[PCRE_IS_SINGLELOCHAR],word)){
            toret = appendstr(toret,"[a-z]");
        }else{
            toret = strdup(word);
        }
    }
    
    return toret;
}


int findSeparation(char *pattern, int pos){
    int backward = pos-1;
    int fordward = pos;
    int toret = 0;
    int len = strlen(pattern);
    
    if(pos >= len) return len;
    
    while (!isspace(pattern[backward]) && !isspace(pattern[fordward]) &&
            backward > 0 && pattern[fordward] != '\0' ){
        backward--;
        fordward++;
    }
    
    if (backward == 0) while(!isspace(pattern[fordward]) && pattern[fordward]!='\0') fordward++;
    
    else if (pattern[fordward]=='\0') while(!isspace(pattern[backward]) && backward > 0) backward--;
    
    if ( (backward == 0 && pattern[fordward]=='\0') ) return -1;
    
    if( isspace(pattern[backward])){
        if(pattern[backward+1]=='*' || pattern[backward+1]=='+') 
            toret = backward+1;
        else toret = backward;
    }else{
        if( isspace(pattern[fordward]) ){
            if( pattern[fordward+1] == '*' || pattern[fordward+1] == '+'){
                toret = fordward+1;
            }else toret = fordward;   
        }
    }
        
    return toret;
}

Svector *dividePatternInParts(char *pattern){

    
    int len = strlen(pattern);
    int div = len/2;
    const unsigned int initPos = 0;
    int mediumPos=findSeparation(pattern,div);
    
    if (mediumPos == -1) return NULL;
    
    Svector *toret = newStringvector(2);
    addStringAt(toret,0,getSubstring(pattern,initPos,mediumPos+1));
    addStringAt(toret,1,getSubstring(pattern,mediumPos+1,len-(mediumPos+1)));
    
    return toret;
}

Svector *divideRegex(char *pattern){
    Svector *checkParts = dividePatternInParts(pattern);
    int actualPos= 0;
    int depth = 1;
    
    printlog(LOG_INFO,"[PCRE_REGEX.divideRegex]","REGEX SUB-DIVISION DEPTH %d\n",depth);
    while (actualPos < getStringVectorSize(checkParts) && checkParts != NULL){
        char *subPattern = getStringAt(checkParts,actualPos);
        pcre *aux = NULL;
        if ( (aux = pcregex_compile(subPattern)) == NULL ){
            Svector *aux = NULL;
            if ((aux = dividePatternInParts(subPattern)) == NULL){
                printlog(LOG_INFO,"[PCRE_REGEX.divideRegex]","UNABLE TO SUBDIVIDE REGEX. DELETING...\n");
                removeStringAt(checkParts, actualPos);
            }else {
                checkParts = mergeSVectors(checkParts,dividePatternInParts(subPattern),actualPos);
                depth++;
                printlog(LOG_INFO,"[PCRE_REGEX.divideRegex]","REGEX SUB-DIVISION DEPTH %d\n",depth);
            }
        }else{ 
            pcregex_free(aux);
            actualPos++;
        }
    }
    
    return checkParts;
}