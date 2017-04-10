/***************************************************************************                        
*
*   File    : ruleset.h
*   Purpose : Implements a ruleset (set of rules) for filtering
*            
*            
*   Original Author: Ivan Paz, Jose Ramon Mendez (from Grindstone project)
*   Has been widelly modifyed since them
* 
*   Memory improvements, modifications, inclusion of new fields
*       and functions: David Ruano, Noemi Perez, Jose Ramon Mendez
* 
*   Date    : October  14, 2010
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

#ifndef __RULESET_H__
#define __RULESET_H__
#include "hashmap.h"
#include "list_files.h"
#include "vector.h"

/* Implementation of a SpamAssassinRuleset
   @author Jose R. Mendez Reboredo
*/

//You should use pointers to ruleset type

/**
 * Struct for the rule definion. Includes the function name, the params
 * passed to function and a pointer to the real function implementation
 * in order to achieve better performance on runtime
 */

/**
 * A typedef for rule in order to get manipulation simple
 */

#define META_RULE 1<<1
#define NORMAL_RULE 1<<3
#define DEFINITIVE_SCORE 1<<4
#define NORMAL_SCORE 1<<5
#define NOT_NUMERIC_SCORE -1

#define VALID 0
#define INVALID 1

//enum e_rule_class {SPAM, HAM};
//typedef enum e_rule_class rule_class;

enum plugins_load_mode { AUTOMATIC, MANUAL, UNABLE };

typedef enum plugins_load_mode p_load_mode;

struct p_info{
    char *name;
    double cpu_load;
    double io_load;
    double average_load;
    double execution_time;
};

typedef struct p_info p_info;


struct t_attributes{
    long int num_executions; //contador de las veces que salta la regla.
    long int num_success; //contador de las veces que acierta la regla.
    long int num_fails; //contador de las veces que falla la regla.
    double confidence; // pendiente de la regla. Indica hacia donde vá.
    double significance; //peso de la regla.
};

typedef struct t_attributes attributes;

struct definition {
    char *name;
    char *param;
    function_t *pointer; //Pointer to the function.
    char *tflags; //function flags.
    p_info *plugin; //The function plugin structure;
};

typedef struct definition definition;

/**
 * Struct for parser definition. Includes the parser name, and a 
 * pointer to the real parser in order to achieve better performance on
 * runtime
 */
struct parser{
    char *parserName;
    parser_t *parserPointer;
    int parser_type;
};

typedef struct parser parser;

struct score_intervals{
    float positive;
    float negative;
};

typedef struct score_intervals score_intervals;

//BEGIN VICTOR

struct mo_rule_t{
    char *plugin_id;
    char *rule_id;
    float rule_score;
    float rule_estimated_cpu_time;
    float rule_estimated_io_time;
    double rule_real_cpu_time;
    double rule_real_io_time;
    long int content_lenght;
    int correct_executions;
};

typedef struct mo_rule_t mo_rule;

//END VICTOR


struct meta_info {
    char *expresion;
    vector *dependant_rules;
    short status;
};

typedef struct meta_info meta_definition;

/**
 * Struct that contains a rule definition. The rule include the name, 
 * the associated score, the definition (in a definition struct), the
 * parser needed (in a parser struct) and the rule description*/
struct rule {
   char *rulename;  //The name of the rule
   void *score;   //The score
   void *def; // Definition of the rule {name and params}
   parser *par; //The parser needed
   attributes *attr;
   char *description; //The rule description
   map_t target_domain; //The target domain flag.
   short characteristic; //The type of rule (META, NORMAL or DEFINITIVE)
};

typedef struct rule rule;

/**
 * A ruleset structure. Handles set of rules (struct rule) and the
 * required_score paramenter that stands for the minimun score for 
 * classify a message as spam
 */

struct schedule_data_t{
   int end_meta;
   int end_definitive;
   int begin_normal;
};
//
typedef struct schedule_data_t schedule_data_t;

struct ruleset {
     float required; //The required score
     score_intervals *intervals;
     
     rule *rules; //The rules
     map_t map; //A map String->Int to lookup rule number from rule name
     map_t meta_map;
     map_t dependant_map;
     
     //rule *def_rules; //Definitive rules.
     int meta_size;
     int def_size;
     int size;  //The size of the map
     schedule_data_t *sdata_t;
     p_load_mode plugin_overload_manager;
};

typedef struct ruleset ruleset;

rule *get_rule(const ruleset *rules, const int ruleno);

//Achieve the score for a rule
void *get_rule_score(const rule *r);

int has_zero_score(const rule *r);

//char *get_definitive_score(const ruleset *rules, int ruleno);

//float get_meta_score(const ruleset *rules, int metano);

//Find the index of a rule
int get_rule_index(const ruleset *rules, char* rulename);

rule *get_rule_by_name(const ruleset *rules, char* rulename);

short get_rule_characteristic_at(const ruleset *rules, const int ruleno);

short get_rule_characteristic(const rule *r);

//int get_def_index(const ruleset *rules, char* rulename);

int get_meta_index(const ruleset *rules, char* rulename);

rule *get_meta_by_name(const ruleset *rules, char* rulename);

//Load a ruleset from a file
ruleset *load_ruleset(const filelist *files,const filelist *attrs,map_t parsers_name);

//Write a ruleset using standard output
void write_ruleset(const char* filename, const ruleset *rules);


void write_rule_attributes(const char* filename,const ruleset *rules);

//Write a ruleset using standard output
void out_ruleset(const ruleset *rules);

//Retrieve the number of rules
int count_rules(const ruleset *rules);

//Retrieve the number of rules
int count_definitive_rules(const ruleset *rules);

//Retrieve the number of meta rules
int count_meta_rules(const ruleset *rules);

//Mutate the ruleset (evolutionary computation)
ruleset *mutate(const ruleset *rulesin);

//Make a random merge of two rulesets (used for evolutionary computation)
ruleset *combine(const ruleset *rules1, const ruleset *rules2);

//Duplicate a ruleset.
ruleset *duplicate_ruleset(const ruleset * src);

//Set required score
void set_required_score(ruleset *rules, float required);

//Get the required score
float get_required_score(const ruleset *rules);

//Set upper stop score
void set_upper_score(ruleset *rules, float upper);

//Set lower stop score
void set_lower_score(ruleset *rules, float lower);

//Get the lower stop score. For default filter evaluation
float get_lower_score(const ruleset *rules);

//Get the upper stop score. For default filter evaluation
float get_upper_score(const ruleset *rules);

//Get the sum of the positive scores. For smart filter evaluation
float get_positive_score(const ruleset *rules);

//Get the sum of the negativee scores. For smart filter evaluation
float get_negative_score(const ruleset *rules);

//Get the name of the rule.
char *get_rulename(const rule*r);

char *get_rulename_at(const ruleset *rules, const int ruleno);

attributes *get_rule_attributes(const rule *r);

//Get the number of times the rule is triggered
//long int get_num_triggered(const ruleset *rules, const int ruleno);
long int get_num_executions_attr(attributes *attr);

//Get the number of times the rule matches hits correctly the e-mail. 
long int get_num_success_attr(attributes *attr);

//Get the number of times the rule fails with the e-mail class. 
long int get_num_fails_attr(attributes *attr);

//Get the confidence of the rule
double get_confidence_attr(attributes *attr);

//Get the rule significance
double get_significance_attr(attributes *attr);

void set_attributes(rule *r, long int executions, long int success, 
                    long int fails, double confidence, double significance);

char *get_metaname(const rule *r);

definition *get_definition(const rule *r);

char *get_definition_name(const rule *r);

char *get_definition_param(const rule *r);

vector *get_dependant_rules(const rule *r);

function_t *get_definition_pointer(const rule *r);

//Get the definition plugin for the rule.
char *get_definition_name_plugin(rule *r);

char *get_definition_name_plugin_at(ruleset *rules, const int ruleno);

//Get the IO load value for the rule
float get_definition_io_load_plugin(const rule *r);

//Get the CPU consumption value for the rule
float get_definition_cpu_load_plugin(const rule *r);

//Get the average value for the rule.
float get_definition_avg_load_plugin(const rule *r);

//Get the rule flags.
char *get_definition_tflags(const rule *r);

//Get the rule parser
parser *get_parser(const rule *r);

//Get the rule parser name
char *get_parser_name(const rule *r);

//Get the rule domain restriction
map_t get_rule_domain(const rule *r);

//Get rule domain name.
map_t get_meta_domain(const rule *r);

//Get definition values for META rule.
char *get_meta_definition(const rule *r);

//Get the rule parser pointer
parser_t *get_parser_pointer(const rule *r);

//Set de debug mode for the application.
void set_debug_mode(int debug);

//Get the name of the rule
void set_rulename(rule *r, const char* rulename);

//Set a definition for the definitive rule.
void set_def_definition(rule *r, const char *defName, const char * defParam);

//Set a definition for the rule.
void set_definition(rule *r, const char *defName, const char * defParam);

//Set a definition name for the rule.
void set_definition_name(rule *r, const char * newName);

//Set a definition param for the rule.
void set_definition_param(rule *r, const char * newParam);

//Set a definition pointer for the rule.
void set_definition_pointer(rule *r, function_t *newPointer);

//Set the definition plugin for the rule.
void set_definition_plugin(rule *r, const char *newPlugin, float cpu_load, float io_load, float time);

//Set newFlags for the rule.
void set_tflags(rule *r, const char *newFlags);

//Set new parser for the definitive rule.
void set_def_parser(rule *r, const char *parserName );
//Set new parser for the rule.
void set_parser(rule *r, const char *parserName);//, int parserType);

//Set new parser name for the rule.
void set_parser_name(rule *r,const char *newParserName);

//Set new parser pointer for the rule
void set_parser_pointer(rule *r, parser_t *newParserPointer);

//Set the description of the rule
void set_description(rule *r, const char* desc);

//void set_meta_expression(ruleset *rules, int ruleno, const char* exp);
void set_meta_definition(ruleset *rules, rule *r, char *expresion);

//Set the number of time the rule is triggered
void set_num_executions_attr(attributes *attr, long int executions);

//Set the number of times the rule has matched an email succesfully
void set_num_success_attr(attributes *attr, long int success);

//Set the number of times the rule has matched an email incorrectly 
void set_num_fails_attr(attributes *attr, long int fails);

//Set the confidence of a rule
void set_confidence_attr(attributes *attr, double confidence);

//get the significance of a rule.
void set_significance_attr(attributes *attr, double significance);

//Get the description of the rule
char *get_description(const rule *r);

int is_dependant_rule(ruleset *rules,const rule *rule);

//Obtain if the rule is META or not.
short is_valid_meta(rule *r);

//Free memory used by a ruleset
void ruleset_free(ruleset *r);

//Preshedule rules
void preschedule(ruleset *r);

//BEGIN THESIS

struct random_vector{
    int *vector;
    int size;
};

typedef struct random_vector random_vector;

random_vector *randomize_vector(ruleset *rules);

int free_vector_element(element data);

random_vector **generate_filter(ruleset *rules, int numfilters);

void create_filter_files(random_vector **filters, int numfilters, ruleset *rules);

//END THESIS

//BEGIN VICTOR

map_t create_rule_hashmap(ruleset *rules,double overload);
void free_mo_hashmap(map_t rulemap);
void fprintf_sorted_rules(ruleset *rules,map_t rulemap, FILE *output);
void add_total_time(map_t rulemap,struct timeval t_inicio, struct timeval t_final);
void fprint_properties_file(map_t rulemap, FILE *output);
void fprint_prescheduler_second_paper(map_t rulemap, char *prescheduler_name);
void print_prescheduler_second_paper(map_t rulemap, char *prescheduler_name);
void print_properties_file(map_t rulemap);
void add_new_email(map_t rulemap);
void add_real_execution_times(map_t rulemap,char *rulename,double cpu_time,double io_time, long int size);
void increase_sfe_execution(map_t rulemap);
void increase_executed_rule(map_t rulemap);

void print_debug_prescheduler(map_t rulemap);
        
//END VICTOR

#endif