
#ifndef PCRE_REGEX_UTIL_H
#define	PCRE_REGEX_UTIL_H

#include <pcre.h>
#include <stdbool.h>
#include "string_vector.h"
#include "generic_vector.h"
#include "common_dinamic_structures.h"

#define ANCHORED_GROUP 3
#define FULL_ANCHORED 2
#define PARTIAL_ANCHORED 1
#define NONE_ANCHORED 0

typedef struct regex_data_t regex_data; 
typedef struct pcre_vector_data_t pcre_vector;

typedef enum {BOUNDED, UNBOUNDED} pcre_type;

pcre *pcregex_compile(const char *pattern);

int pcregex_match(const pcre *reg, const char *string);

void pcregex_free(pcre *reg);

bool isSpecialChar(char ch);
bool isSpecialString(char *string);

char *pcregex_replace(char *src, pcre *re, char *replace);
char *scapeMetaCharacters(char *string);

char *generateRegExp(Svector *words, pcre_vector *regexVector);
char *generateRegexGen(char *word,pcre_vector *precompiledRegex);
char *pcregex_compress(char *src, pcre_type type);

pcre_vector *compileRegex();
pcre_vector *staticPCREVector();
int hasStartAnchor(Svector *chr);
int startsWith(char *string, char ch);
void freeCompileRegex(pcre_vector *compiledRegex);
void freeRegexData(regex_data *data);

regex_data *newRegexData( char *pattern,pcre *regex, int hitCounter, 
                          double score, Svector *chr);
void setRegexDataChromosomes(regex_data *rData, Svector *chr);
void setRegexDataScore(regex_data *rData, double score);
Svector *getRegexDataChromosomes(regex_data *rData);
char *getRegexDataPattern(regex_data *rData);
pcre *getRegexDataPCRE(regex_data *rData);
int getRegexDataHitCounter(regex_data *rData);
double getRegexDataScore(regex_data *rData);

//int getRegexDataHamCounter(regex_data *rData); TODO
Svector *divideRegex(char *pattern);
Svector *dividePatternInParts(char *pattern);

#endif	/* PCRE_REGEX_UTIL_H */

