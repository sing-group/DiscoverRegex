/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   experimental.c
 * Author: drordas
 *
 * Created on 4 de noviembre de 2016, 14:25
 */

#include <stdio.h>
#include <stdlib.h>
#include "experimental.h"
#include "logger.h"
#include "string_vector.h"

void setMatrixNhamAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->nham = value;
    }
}

void setMatrixNregexAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->nregex = value;
    }
}

void setMatrixNspamAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->nspam = value;
    }
}

void setMatrixFPAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->fp = value;
    }
}

void setMatrixFNAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->fn = value;
    }
}

void setMatrixTpAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->tp = value;
    }
}

void setMatrixTnAt(experiment *exp, int folder, long int value){
    if(folder < exp->iter && exp->cmatrix != NULL){
        exp->cmatrix[folder]->tn = value;
    }
}

//////////////////////////////////////////////////

long int getMatrixNregexAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->nregex;
    }else return 0;
}

long int getMatrixNhamAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->nham;
    }else return 0;
}

long int getMatrixNspamAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->nspam;
    }else return 0;
}

long int getMatrixFPAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->fp;
    }else return 0;
}

long int getMatrixFNAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->fn;
    }else return 0;
}

long int getMatrixTpAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->tp;
    }else return 0;
}

long int getMatrixTnAt(experiment *exp, int folder){
    if(folder < exp->iter && exp->cmatrix != NULL){
        return exp->cmatrix[folder]->tn;
    }else return 0;
}

/////////////////////////////////////////////////

experiment *createExpemimentData(int numFolds){
    experiment *toret = NULL;
    int i=0;
    if (numFolds >0){
        toret = malloc(sizeof(experiment));
        toret->test=malloc(sizeof(folder));
        toret->trn=malloc(sizeof(folder));
        toret->test->hamPath=newStringvector(numFolds);
        toret->test->spamPath=newStringvector(numFolds);
        toret->trn->hamPath=newStringvector(numFolds);
        toret->trn->spamPath=newStringvector(numFolds);
        toret->test_emails = NULL;
        toret->cmatrix = malloc(sizeof(conf_matrix *)*numFolds);
        for(;i<numFolds;i++){
            toret->cmatrix[i]=malloc(sizeof(conf_matrix));
            toret->cmatrix[i]->tp=0;
            toret->cmatrix[i]->fp=0;
            toret->cmatrix[i]->tn=0;
            toret->cmatrix[i]->fn=0;
            toret->cmatrix[i]->nham=0;
            toret->cmatrix[i]->nspam=0;
            toret->cmatrix[i]->nregex=0;
        }
        toret->iter=numFolds;
    }
    return toret;
}

tst_data *get_tst_data(experiment *exp){
    if (exp !=NULL && exp->test_emails!=NULL)
        return exp->test_emails;
    return NULL;
}

void freeExperimentData(experiment *exp){
    if (exp != NULL){
        if(exp->test!=NULL){
            (exp->test->hamPath!=NULL)?(freeStringVector(exp->test->hamPath)):(0);
            (exp->test->spamPath!=NULL)?(freeStringVector(exp->test->spamPath)):(0);
            free(exp->test);
        }

        if(exp->trn!=NULL){
            (exp->trn->hamPath!=NULL)?(freeStringVector(exp->trn->hamPath)):(0);
            (exp->trn->spamPath!=NULL)?(freeStringVector(exp->trn->spamPath)):(0);
            free(exp->trn);
        }
        if( exp->cmatrix != NULL){
            int i=0;
            for(;i<exp->iter;i++) free(exp->cmatrix[i]);
        }
        free(exp->cmatrix);
        free(exp);
    }
}

Svector *getTstHam(experiment *exp){
    if(exp == NULL || exp->test == NULL || exp->test->hamPath == NULL){
        return NULL;
    }else return (exp->test->hamPath);
}

char *getTstHamAt(experiment *exp, long int pos){
    if(exp == NULL || exp->test == NULL || exp->test->hamPath == NULL){
        return NULL;
    }else return getStringAt(exp->test->hamPath,pos);
}

Svector *getTstSpam(experiment *exp){
    if(exp == NULL || exp->test == NULL || exp->test->spamPath == NULL){
        return NULL;
    }else return (exp->test->spamPath);
}

char *getTstSpamAt(experiment *exp, long int pos){
    if(exp == NULL || exp->test == NULL || exp->test->spamPath == NULL){
        return NULL;
    }else return getStringAt(exp->test->spamPath,pos);
}

Svector *getTrnHam(experiment *exp){
    if(exp == NULL || exp->trn == NULL || exp->trn->hamPath == NULL){
        return NULL;
    }else return (exp->trn->hamPath);
}

char *getTrnHamAt(experiment *exp, long int pos){
    if(exp == NULL || exp->trn == NULL || exp->trn->hamPath == NULL){
        return NULL;
    }else return getStringAt(exp->test->hamPath,pos);
}

Svector *getTrnSpam(experiment *exp){
    if(exp == NULL || exp->trn == NULL || exp->trn->spamPath == NULL){
        return NULL;
    }else return (exp->trn->spamPath);
}

char *getTrnSpamAt(experiment *exp, long int pos){
    if(exp == NULL || exp->trn == NULL || exp->trn->spamPath == NULL){
        return NULL;
    }else return getStringAt(exp->trn->spamPath,pos);
}

void addTstHamAt(experiment *exp, long int pos, char *item){
    if (exp->test->hamPath == NULL || pos > getStringVectorSize(exp->test->hamPath)  ){
        printlog(LOG_CRITICAL,"[EXPERIMENTAL]","Error adding element to Ham test vector\n");
    }else addStringAt(exp->test->hamPath,pos,item);
}

void addTstSpamAt(experiment *exp, long int pos, char *item){
    if (exp->test->spamPath == NULL || pos > getStringVectorSize(exp->test->spamPath)  ){
        printlog(LOG_CRITICAL,"[EXPERIMENTAL]","Error adding element to Spam test vector\n");
    }else{ 
        //printf("v[%ld]=>%s\n",pos,item);
        addStringAt(exp->test->spamPath,pos,item);
    }
}

void addTrnHamAt(experiment *exp, long int pos, char *item){
    if (exp->trn->hamPath == NULL || pos > getStringVectorSize(exp->trn->hamPath)  ){
        printlog(LOG_CRITICAL,"[EXPERIMENTAL]","Error adding element to Ham train vector\n");
    }else addStringAt(exp->trn->hamPath,pos,item);
}

void addTrnSpamAt(experiment *exp, long int pos, char *item){
    if (exp->trn->spamPath == NULL || pos > getStringVectorSize(exp->trn->spamPath)  ){
        printlog(LOG_CRITICAL,"[EXPERIMENTAL]","Error adding element to Spam train vector\n");
    }else addStringAt(exp->trn->spamPath,pos,item);
}

int getIterations(experiment *exp){
    return exp->iter;
}

void printExperimentData(experiment *exp){
    int i=0;
    printf("TRAINING SOURCES\n");
    for(i=0;i<exp->iter;i++){
        printf("[%d]-SPAM: '%s'\n",i,getStringAt(exp->trn->spamPath,i));
        printf("[%d]-HAM: '%s'\n",i,getStringAt(exp->trn->hamPath,i));
    }
    printf("TEST SOURCES\n");
    for(i=0;i<exp->iter;i++){
        printf("[%d]-SPAM: '%s'\n",i,getStringAt(exp->test->spamPath,i));
        printf("[%d]-HAM: '%s'\n",i,getStringAt(exp->test->hamPath,i));
    }
}

void printConfusionMatrix(experiment *exp){
    int i=0;
    printf("||================================================||\n");
    printf("||                CONFUSION MATRIX                ||\n");
    printf("||================================================||\n");
    for (i=0;i<exp->iter;i++){
        printf(" =========== \n");
        printf("  FOLDER %d  \n",i);
        printf(" =========== \n");
        printf("  nregex:%ld\n",exp->cmatrix[i]->nregex);
        printf("  nham:\t%ld\n",exp->cmatrix[i]->nham);
        printf("  nspam:%ld\n",exp->cmatrix[i]->nspam);
        printf("  TP:\t%ld\n",exp->cmatrix[i]->tp);
        printf("  TN:\t%ld\n",exp->cmatrix[i]->tn);
        printf("  FP:\t%ld\n",exp->cmatrix[i]->fp);
        printf("  FN:\t%ld\n",exp->cmatrix[i]->fn);
        printf(" =========== \n");
    }
}

void confusionMatrixToFile(experiment *exp,int folder){
    FILE *fp = NULL;
    if ( (fp = fopen ("results/cmatrix.csv", "r")) == NULL){
        if ( (fp = fopen ("results/cmatrix.csv", "w")) != NULL) {
            fprintf(fp,"folder,nregex,nham,nspam,tp,tn,fp,fn\n");
            fprintf(fp,"%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",folder,
                    exp->cmatrix[folder]->nregex,
                    exp->cmatrix[folder]->nham,
                    exp->cmatrix[folder]->nspam,exp->cmatrix[folder]->tp,
                    exp->cmatrix[folder]->tn,exp->cmatrix[folder]->fp,
                    exp->cmatrix[folder]->fn);
            fflush(fp);
            fclose(fp);
        }else printf("Cannot open file 'cmatrix.csv'");
    }else{
        fclose(fp);
        if ( (fp = fopen ("results/cmatrix.csv", "a+")) != NULL){
            fprintf(fp,"%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",folder,
                    exp->cmatrix[folder]->nregex,
                    exp->cmatrix[folder]->nham,
                    exp->cmatrix[folder]->nspam,exp->cmatrix[folder]->tp,
                    exp->cmatrix[folder]->tn,exp->cmatrix[folder]->fp,
                    exp->cmatrix[folder]->fn);
            fflush(fp);
            fclose(fp);
        }else printf("Cannot open file 'cmatrix.csv'");
    }
}




