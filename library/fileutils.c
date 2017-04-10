/* 
 * File:   fileReader.c
 * Author: drordas
 *
 * Created on 14 de octubre de 2015, 13:37
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "string_util.h"
#include "common_dinamic_structures.h"
#include "pcre_regex_util.h"
#include "fileutils.h"
#include "hashmap.h"
#include "generic_vector.h"
#include "string_vector.h"
#include "filedata.h"
#include "logger.h"

Svector *separateLineInWords(char *line);
Svector *separateFileInLines(char *filePath);

tst_data *loadTstData(char *spamPath, char *hamPath){
    return newTstData(separateFileInLines(spamPath),separateFileInLines(hamPath));
}

file_data *loadToFileData(char *filePath1, char *filePath2){
    char *line = NULL;
    FILE *fp = NULL;
    char ch;
    long int countLines = 0;
    long int totalLines;
    
    if( (fp =fopen(filePath1,"r")) == NULL ){
        printlog(LOG_CRITICAL,"[FILEUTILS]","Cannot open file\n");
        return NULL;
    }else printlog(LOG_INFO,"[FILEUTILS]","Loading SPAM data from '%s'\n",filePath1);
    
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0 || strlen(trim(line))<=1 || isSpecialString(trim(line))) continue; //Discard line if void
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    free(line);
 
    if(countLines == 0){ 
        fclose(fp);
        return NULL;
    }
    
    rewind(fp);
        
    Svector *spamLines = newStringvector(countLines);
    Gvector *spamData = newGenericVector(countLines);
    
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    totalLines = countLines;
    countLines = 0;
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            printlog(LOG_INFO,"[FILEUTILS]","Loading line %ld/%ld",countLines,totalLines);
            if (strcmp(trim(line),"") == 0 || strlen(trim(line))<=1 || isSpecialString(trim(line))) continue; //Discard line if void
            addStringAt(spamLines,countLines,strdup(line));
            char *no_blanks = removeRecursiveBlanks(strdup(line));
            addElementAt(spamData,countLines,newEmailData(no_blanks,separateLineInWords(no_blanks)));
            countLines++;
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
        }else line=append(line,ch);
        
    }while (!feof(fp));
    
    free(line);
    fclose(fp);
    
    countLines = 0;
    
    if( (fp =fopen(filePath2,"r")) == NULL ){
        printlog(LOG_CRITICAL,"[FILEUTILS]", "Cannot open file\n");
        return NULL;
    }else printlog(LOG_INFO,"[FILEUTILS]","Loading HAM data from '%s'\n",filePath2);

    
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0 || strlen(trim(line))<=1 || isSpecialString(trim(line))) continue; //Discard line if void
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    free(line);
    
    if(countLines == 0){ 
        fclose(fp);
        return newFileData(spamLines,spamData,NULL,NULL);
    }
    rewind(fp);

    Svector *hamLines = newStringvector(countLines);
    Gvector *hamData = newGenericVector(countLines);
    
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    totalLines = countLines;
    countLines = 0;
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            printlog(LOG_INFO,"[FILEUTILS]","Loading line %ld/%ld",countLines,totalLines);
            if (strcmp(trim(line),"") == 0 || strlen(trim(line))<=1 || isSpecialString(trim(line))) continue; //Discard line if void
            addStringAt(hamLines,countLines,strdup(line));
            char *no_blanks = removeRecursiveBlanks(strdup(line));
            Svector *separatedLines = separateLineInWords(no_blanks);
            addElementAt(hamData,countLines,newEmailData(no_blanks,separatedLines));
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    
    free(line);
    fclose(fp);
    
    printlog(LOG_INFO,"[FILEUTILS]","Summary:\t\t\t\n\tTotal loaded %ld spam lines\n\tTotal loaded %ld ham lines\n",getGenericVectorSize(spamData),getGenericVectorSize(hamData));
  
    return newFileData(spamLines,spamData,hamLines,hamData);
}

Gvector *separateFileInWords(char *filePath){
    char *line = NULL;
    FILE *fp = NULL;
    char ch;
    long int countLines = 0;
    Gvector *lineVector = NULL;
    
    if( (fp =fopen(filePath,"r")) == NULL ){
        printlog(LOG_CRITICAL,"[FILEUTILS]","Cannot open file '%s'\n",filePath);
        return NULL;
    }
    
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0) continue; //Discard line if void
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    free(line);
    
 
    if(countLines == 0){ 
        fclose(fp);
        return lineVector;
    }
    
    rewind(fp);
    lineVector = newGenericVector(countLines);
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    countLines = 0;
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0) continue; //Discard line if void
            addElementAt(lineVector,countLines,separateLineInWords(line));
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    free(line);
    fclose(fp);
    return lineVector;
}



void freeWordVector(Gvector *vector, PFree f){
    if ( vector == NULL || getGenericVectorSize(vector) <=0 ){
        freeGenericVector(vector,f);
    }
}

Svector *separateFileInLines(char *filePath){
    char *line = NULL;
    FILE *fp = NULL;
    char ch;
    long int countLines = 0;
    
    if( (fp =fopen(filePath,"r")) == NULL ){
        printlog(LOG_CRITICAL,"[FILEUTILS]","Cannot open file '%s'\n", filePath);
        return NULL;
    }else printlog(LOG_INFO,"[FILEUTILS]","Loading test files '%s'\n",filePath);

    
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0) continue; //Discard line if void
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    free(line);
    
    rewind(fp);

    Svector *linesVector = newStringvector(countLines);
    line = (char *) malloc(sizeof(char));
    strcpy(line,"");
    countLines = 0;
    
    do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0) continue; //Discard line if void
            addStringAt(linesVector,countLines,line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
            countLines++;
        }else line=append(line,ch);
        
    }while (!feof(fp));
    
    free(line);
    fclose(fp);
    
    printlog(LOG_INFO,"[FILEUTILS]","Total loaded %d lines\n",(int)getStringVectorSize(linesVector));
    
    return linesVector;
}


/*
long int wordCounter(char *line){
    long int count = 0;
    //long int wordcounter=0;
    long int wordcounter = 1;
    long int size = strlen(line);
    
    while(count < size-1){
        if( !isspace(line[count]) && isspace(line[count+1]) ){
            wordcounter++;
        }
        count++;
    }
    if(!isspace(line[count]) && isspace(line[count+1])) wordcounter++;
    return wordcounter;
}
 
Svector *separateLineInWords(char *line){
    long int wordCount = wordCounter(line);
    Svector *words = NULL;

    if (wordCount > 0){
        words = newStringvector(wordCount);
        char *dup = strdup(line);
        char *word = strtok(dup," \t\n\r");
        long int insertionPos = 0;
        while( word != NULL ){
            char *finalWord = NULL;
            long int pos = 0;
            while(pos < strlen(word)){
                if(isSpecialChar(word[pos])) finalWord=appendchar(finalWord,'\\');
                finalWord = appendchar(finalWord,word[pos]);
                pos++;
            }
            addStringAt(words,insertionPos++,finalWord);
            word = strtok(NULL," \t\n\r");
        }
        free(dup);
    }
    return words;
}
*/

long int wordCounter(char *line){
    long int count = 0;
    long int wordcounter = 1;
    long int size = strlen(line);
    
    while(count < size-1){
        if( (!isspace(line[count]) && isspace(line[count+1])) || 
            ((line[count]==',' || line[count]==';') && count < size-1 )){
            wordcounter++;
        }
        count++;
    }
    if( (!isspace(line[count]) && isspace(line[count+1])) || 
        ((line[count]==',' || line[count]==';') && count < size-1)) wordcounter++;
    return wordcounter;
}

Svector *separateLineInWords(char *line){
    long int wordCount = wordCounter(line);
    Svector *words = NULL;
    
    if (wordCount > 0){
        words = newStringvector(wordCount);
        long int size = strlen(line);    
        long int insertionPos = 0;
        char *finalWord = NULL;
        long int pos = 0;
        while(pos < size){
            if(isspace(line[pos])){
                if(finalWord != NULL ){
                    addStringAt(words,insertionPos++,finalWord);
                    finalWord = NULL;
                }
            }else{
                if(line[pos]==',' || line[pos]==';'){ 
                    finalWord = appendchar(finalWord,line[pos]);
                    addStringAt(words,insertionPos++,finalWord);
                    finalWord = NULL;
                }else{
                    if (isSpecialChar(line[pos])) finalWord=appendchar(finalWord,'\\');
                    finalWord=appendchar(finalWord,line[pos]);
                }
            }
            pos++;
        }
        addStringAt(words,insertionPos++,finalWord);
    }
    return words;
}

Svector *fileToSvector (char *filePath){
    char *line = NULL;
    char ch;
    FILE *fpSpam;
    Svector *words = NULL;
    map_t regexMap = NULL;
        
    //REGEX COMPILATION
    if( (line=(char *)malloc(sizeof(char))) == NULL  ||
        (fpSpam = fopen(filePath,"r")) == NULL || 
        (regexMap = hashmap_new()) == NULL)       
    {
        printlog(LOG_CRITICAL,"[FILEUTILS]","Cannot allocate enought memory\n");
        return NULL;
    }

    strcpy(line,"");
    
    do{
        ch=fgetc(fpSpam);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"") == 0) continue; //Discard line if void
            int count = 0;
            int size = strlen(line);
            words = newStringvector(wordCounter(line));
            char *word = (char *)malloc(sizeof(char));
            strcpy(word,"");
            int wordcounter=0;

            while(count < size){
                if( (isalnum(line[count])) ||
                    (line[count]==':' && isspace(line[count+1])))
                {
                    if( count!=0 && (!isalnum(line[count-1]))){
                        addStringAt(words,wordcounter,word);
                        wordcounter++;
                        word = (char *)malloc(sizeof(char));
                        strcpy(word,"");
                    }
                }else{
                    if (count!=0){
                        addStringAt(words,wordcounter,word);
                        wordcounter++;
                        word = (char *)malloc(sizeof(char));
                        strcpy(word,"");
                    }
                    if (isSpecialChar(line[count])) word=appendchar(word,'\\');
                }
                word=appendchar(word,line[count]);
                count++;
            }
            addStringAt(words,wordcounter,word);
            freeStringVector(words);
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
        }else line=append(line,ch);
        
    } while (ch!=EOF);
    
    free(line);
    fclose(fpSpam);
    
    return NULL;
}

char *fileToString(char *filePath){
    FILE *fp = NULL;
    long length=0;
    char *buffer = NULL;
    printlog(LOG_INFO,"[FILEUTILS]", "Opening file %s\n",filePath);
    if ( (fp=fopen(filePath,"r"))!= NULL ){
        fseek(fp,0L,SEEK_END);
        length = ftell(fp);
        fseek (fp, 0L, SEEK_SET);
        if ( (buffer = malloc(sizeof(char)*(length+1))) ){
            long readed= fread(buffer,sizeof(char),length,fp);
            if(readed == length){
                buffer[length-1]='\0';
                printlog(LOG_INFO,"[FILEUTILS]","File '%s' succesfully readed\n",filePath);
                printlog(LOG_INFO,"[FILEUTILS]","Real Size: %ld\n\t\t   Readed: %ld\n",length,readed);
                printlog(LOG_INFO,"[FILEUTILS]","BUFFER: %s\n",buffer);
            }
        }
        fclose(fp);
    }
    
    return buffer;
}

int ae_load_file_to_memory(const char *filename, char **result)
{ 
    int size = 0;
    
    FILE *f = fopen(filename, "rb");
    if (f == NULL){
        *result = NULL;
        return -1;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *result = (char *)malloc(size+1);
    if (size != fread(*result, sizeof(char), size, f)) {
        free(*result);
        return -1;
    }

    fclose(f);
    (*result)[size] = '\0';
    return size;
}