/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "common_dinamic_structures.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "linked_list.h"
#include "generic_vector.h"
#include "string_vector.h"
#include "filedata.h"
#include "logger.h"


struct email_data_t{
    Svector *words;
    char *line;
};

struct file_data_t{
    Svector *SpamLines;
    Gvector *SpamData;
    Svector *HamLines;
    Gvector *HamData;
};

struct tst_data_t{
    Svector *SpamLines;
    Svector *HamLines;
};

Svector *getTstHamLines(tst_data *tst){
    if (tst!= NULL && getStringVectorSize(tst->HamLines)>0){
        return tst->HamLines;
    }
    return NULL;
}

Svector *getTstSpamLines(tst_data *tst){
    if (tst!= NULL && getStringVectorSize(tst->SpamLines)>0){
        return tst->SpamLines;
    }
    return NULL;
}

email_data *newEmailData (char *line, Svector *words){
    email_data *data = NULL;
    if ( (data = malloc(sizeof(email_data))) != NULL){
        data->line= line;
        data->words= words;
    }else printlog(LOG_CRITICAL,"[FILEDATA.newEmailData]","Cannot create email_data structure. Not enough memmory\n");
    return data;
}

void freeEmailData(email_data *data){
    if(data != NULL){
        if (data->line != NULL) free(data->line);
        if (data->words != NULL) freeStringVector(data->words);
        free(data);
    }
}

void setEmailDataLine(email_data *data,char *line){
    if(data != NULL) data->line=line;
}

void replaceEmailDataLine(email_data *data,char *line){
    if(data != NULL){ 
        if(data->line != NULL) free(data->line);
        data->line=line;
    }
}

void setEmailDataWords(email_data *data,Svector *words){
    if(data != NULL) data->words= words;
}

void replaceEmailDataWords(email_data *data,Svector *words){
    if (data != NULL){
        if (data->words != NULL) freeStringVector(words);
        data->words=words;
    }
}

int freeGEmailData(gElement item){
    email_data *data = (email_data *)item;
    
    if (data != NULL){
        if (data->line != NULL) free(data->line);
        if (data->words != NULL) freeStringVector(data->words);
        free(data);
    }
    
    return GOK;
}

char *getEmailFullLine(email_data *data){
    return data->line;
}

Svector *getEmailLineWords(email_data *data){
    return data->words;
}

////////////////////////////////////////////////////////////////////////////////


tst_data *newTstData(Svector *spamLines, Svector *hamLines){
    tst_data * toret = NULL;
    if (getStringVectorSize(spamLines)>0 && getStringVectorSize(hamLines) > 0){
        toret = malloc(sizeof(tst_data));
        toret->HamLines=hamLines;
        toret->SpamLines=spamLines;
    }
    return toret;
}

file_data *newFileData(Svector *SpamLines, Gvector *SpamData, 
                       Svector *HamLines, Gvector *HamData){
    file_data *fData = NULL;
    if ( (fData = malloc(sizeof(file_data))) != NULL ){
        fData->SpamLines=SpamLines;
        fData->SpamData=SpamData;
        fData->HamLines=HamLines;
        fData->HamData=HamData;
    }else printlog(LOG_CRITICAL,"[FILEDATA.newFileData]","Cannot create file_data structure. Not enough memmory\n");
    
    return fData;
}

void freeFileData(file_data *data){
    if (data != NULL){
        if (data->HamLines != NULL) freeStringVector(data->HamLines);
        if (data->SpamLines != NULL) freeStringVector(data->SpamLines);
        if (data->HamData != NULL) freeGenericVector(data->HamData,&freeGEmailData);
        if (data->SpamData != NULL) freeGenericVector(data->SpamData,&freeGEmailData);
        free(data);
    }
}

void freeTstData(tst_data *data){
    if (data != NULL){
        if (data->HamLines != NULL) freeStringVector(data->HamLines);
        if (data->SpamLines != NULL) freeStringVector(data->SpamLines);
        free(data);
    }
}

void setFileDataSpamLines(file_data *fData, Svector *SpamLines){
    if (fData != NULL) fData->SpamLines=SpamLines;
}

void removeFileDataSpamLines(file_data *fData){
    if (fData != NULL && fData->SpamLines != NULL) freeStringVector(fData->SpamLines);
}

void replaceFileDataSpamLines(file_data *fData, Svector *SpamLines){
    if (fData != NULL){
        if(fData->SpamLines != NULL) freeStringVector(fData->SpamLines);
        fData->SpamLines=SpamLines;
    }
}

void setFileDataSpamData(file_data *fData, Gvector *SpamData){
    if (fData != NULL) fData->SpamData=SpamData;
}

void removeFileDataSpamData(file_data *fData){
    if (fData != NULL && fData->SpamData != NULL)
        freeGenericVector(fData->SpamData,&freeGEmailData);
}

void replaceFileDataSpamData(file_data *fData, Gvector *SpamData){
    if (fData != NULL){
        if(fData->SpamData != NULL){
            freeGenericVector(fData->SpamData,&freeGEmailData);
            fData->SpamData = SpamData;
        }
    }
}

Svector * getSpamLines(file_data *fData){
    return fData->SpamLines;
}

Gvector * getSpamData(file_data *fData){
    return fData->SpamData;
}


///////////////////////////////////////////////////////////////////////////

void setFileDataHamLines(file_data *fData, Svector *HamLines){
    if (fData != NULL) fData->HamLines=HamLines;
}

void removeFileDataHamLines(file_data *fData){
    if (fData != NULL && fData->HamLines != NULL) freeStringVector(fData->HamLines);
}

void replaceFileDataHamLines(file_data *fData, Svector *HamLines){
    if (fData != NULL){
        if(fData->HamLines != NULL) freeStringVector(fData->HamLines);
        fData->HamLines=HamLines;
    }
}

void setFileDataHamData(file_data *fData, Gvector *HamData){
    if (fData != NULL) fData->HamData=HamData;
}

void removeFileDataHamData(file_data *fData){
    if (fData != NULL && fData->HamData != NULL)
        freeGenericVector(fData->HamData,&freeGEmailData);
}

void replaceFileDataHamData(file_data *fData, Gvector *HamData){
    if (fData != NULL){
        if(fData->HamData != NULL) freeGenericVector(fData->HamData,&freeGEmailData);
        fData->HamData = HamData;
    }
}

email_data *getHamEmailDataAt(file_data *fData, long int pos){
    email_data *obtained = NULL;
    if ( fData != NULL && pos < getGenericVectorSize(fData->HamData))
         getElementAt(fData->HamData,pos,(gElement *)&obtained);
    return obtained;
}

email_data *getSpamEmailDataAt(file_data *fData, long int pos){
    email_data *obtained = NULL;
    if ( fData != NULL && pos < getGenericVectorSize(fData->SpamData))
         getElementAt(fData->SpamData,pos,(gElement *)&obtained);
    return obtained;
}

Svector * getHamLines(file_data *fData){
    return fData->HamLines;
}

Gvector * getHamData(file_data *fData){
    return fData->HamData;
}