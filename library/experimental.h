/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   experimental.h
 * Author: drordas
 *
 * Created on 4 de noviembre de 2016, 14:25
 */

#ifndef _EXPERIMENTAL_H_
#define _EXPERIMENTAL_H_

#include "common_dinamic_structures.h"
#include "filedata.h"

typedef struct group_data_t{
    Svector *hamPath;
    Svector *spamPath;
} folder;

typedef struct statistics_data_t{
    long int fp;
    long int fn;
    long int tp;
    long int tn;
    long int nham;
    long int nspam;
    long int nregex;
} conf_matrix;

typedef struct exp_data_t{
    folder *test;
    folder *trn;
    tst_data *test_emails;
    conf_matrix **cmatrix;
    int iter;
}experiment;



experiment *createExpemimentData(int numFolds);
void freeExperimentData(experiment *exp);
Svector *getTstHam(experiment *exp);
Svector *getTstSpam(experiment *exp);
Svector *getTrnHam(experiment *exp);
Svector *getTrnSpam(experiment *exp);
int getIterations(experiment *exp);

char *getTstHamAt(experiment *exp, long int pos);
char *getTstSpamAt(experiment *exp, long int pos);
char *getTrnHamAt(experiment *exp, long int pos);
char *getTrnSpamAt(experiment *exp, long int pos);

//int getCurrentExperiment(experiment *exp);
tst_data *get_tst_data(experiment *exp);

void setMatrixNhamAt(experiment *exp, int folder, long int value);
void setMatrixNspamAt(experiment *exp, int folder, long int value);
void setMatrixFPAt(experiment *exp, int folder, long int value);
void setMatrixFNAt(experiment *exp, int folder, long int value);
void setMatrixTpAt(experiment *exp, int folder, long int value);
void setMatrixTnAt(experiment *exp, int folder, long int value);
void setMatrixNregexAt(experiment *exp, int folder, long int value);

long int getMatrixNhamAt(experiment *exp, int folder);
long int getMatrixNspamAt(experiment *exp, int folder);
long int getMatrixFPAt(experiment *exp, int folder);
long int getMatrixFNAt(experiment *exp, int folder);
long int getMatrixTpAt(experiment *exp, int folder);
long int getMatrixTnAt(experiment *exp, int folder);
long int getMatrixNregexAt(experiment *exp, int folder);

void addTstHamAt(experiment *exp, long int pos, char *item);
void addTstSpamAt(experiment *exp, long int pos, char *item);
void addTrnHamAt(experiment *exp, long int pos, char *item);
void addTrnSpamAt(experiment *exp, long int pos, char *item);
void printExperimentData(experiment *exp);
void printConfusionMatrix(experiment *exp);
void confusionMatrixToFile(experiment *exp,int folder);
//void updateExperiment(experiment exp);

#endif /* EXPERIMENTAL_H */

