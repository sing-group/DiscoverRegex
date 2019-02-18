#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "stack.h"
#include "logger.h"
#include "vector.h"
#include "meta.h"

#define INIT 0
#define BEGIN_OP1 1
#define READING_OP1_NUMBER 2
#define READING_OP1_RULE 3
#define END_OP1 4
#define BEGIN_OPERATOR 5
#define BEGIN_OP2 6
#define READING_OP2_NUMBER 7
#define READING_OP2_RULE 8
#define END_OP2 9

#define MALFORMED_EXPRESION -1

#define BEGIN_RULE 10
#define END_RULE 11

//Definition of expression structure
struct expression{
    int *opd1;
    int *opd2;
    char opt;
    char neg;
};

typedef struct expression expression;

//void free_meta_dependant_rules(vector *v);

//Returns 1 if the character passed is an operator, 0 otherwise
int isOperator ( char elem) {
    if ( elem == '|' || elem == '&' ||
     elem == '+' || elem == '<' || elem == '>' ) {
        return 1;
    }else return 0;
}

int free_hashmap_data(any_t nullpointer, any_t data, any_t key){
    free((char *)key);
    free((int *)data);
    return MAP_OK;
}

//Returns the result of operating the elements on the expression
//passed
int operate ( expression* elem ) {

    if ( elem->opt=='+' ) {
        return ( *(elem->opd1) + *(elem->opd2) );
    }else if ( elem->opt=='>' ) {
        return ( *(elem->opd1) > *(elem->opd2) );
    }else if ( elem->opt=='<' ) {
        return ( *(elem->opd1) < *(elem->opd2) );
    }else if ( elem->opt=='|' ) {
        return ( *(elem->opd1) | *(elem->opd2) );
    }else if ( elem->opt=='&' ) {
        return ( *(elem->opd1) & *(elem->opd2) );
    }
    return 0;//ESTO NO ESTA BIEN
}

int free_stack_expression(element data){
    free((*(expression *)data).opd1);
    free((*(expression *)data).opd2);
    free((expression *)data);
    return STACK_OK;
}

int evaluate_meta(char *toEval, map_t executed_rules){
    /*if ( argc != 2 ) {
    printf("Error: Wrong parameters\n");
    return MALFORMED_EXPRESSION;
    }*/

    char* currentElement = toEval;
    int count=0;
    int status=INIT;
    int result = 0;
    char* beginExp=NULL;
    char* endExp=NULL;
    char* opdread;
    stack* expStack;
    expStack = newstack();
    element recuperar;
    int aux;

    expression* exp = malloc(sizeof(expression));
    exp->opd1 = NULL;
    exp->opd2 = NULL;
    exp->neg = 0;

    int* punteroMap;

    while(currentElement[count]!='\0'){
        //Blanks before the expression are skipped
        if(status == INIT && currentElement[count]==' ') count++;

        //Checks if the expression does not begin with '('
        else if(status == INIT && currentElement[count] != '(') {
            wblprintf(LOG_CRITICAL,"META","Error, expression must begin with (\n");
            return MALFORMED_EXPRESION;
        }

        //Negation operators are skipped, they're checked before a new subexpression
        else if(currentElement[count] == '~') count++;


        //Parenthesis marks the beginning of the expression
        else if(status == INIT && currentElement[count] == '('){
            //printf("INICIO DE EXPRESION_1\n");
            status = BEGIN_OP1;
            count++;
        }

        //Possible blanks before the first operand are skipped
        else if (status == BEGIN_OP1 && currentElement[count] == ' ' ) count++;            

        //If begins a new subexpression, the current one is put into the stack
        else if ((status == BEGIN_OP1 || status == BEGIN_OP2) && currentElement[count] == '('){
            //printf("Inicio de subexpresion\n");
            push_item( expStack, exp );
            exp = malloc(sizeof(expression));
            exp->opd1 = NULL;
            exp->opd2 = NULL;
            exp->neg = 0;
            if( currentElement[count-1] == '~') exp->neg = 1;                    
            count++;
            status = BEGIN_OP1;
        }

        //When the first number is found, status changes without moving the counter
        //and the memory direction of that char is saved
        else if (status == BEGIN_OP1 && isdigit(currentElement[count]) ) {
            status = READING_OP1_NUMBER;
            beginExp = &currentElement[count];
        }

        //If it founds an alphanumeric character, marks the beginning of the rule
        //to be read
        else if (status == BEGIN_OP1 && isalpha(currentElement[count]) ){
            status = READING_OP1_RULE;
            beginExp = &currentElement[count];
        }

        //While at reading number status, each number moves the counter
        else if(status == READING_OP1_NUMBER && isdigit(currentElement[count]) ){
            if (!isdigit(currentElement[count+1])){
                endExp = &currentElement[count+1];
            }
            count++;
        }

        //While at reading rule status, each character or number moves the counter
        else if(status == READING_OP1_RULE && ( isdigit(currentElement[count])
        || isalpha(currentElement[count]) || currentElement[count]=='_')){
            if (!isdigit(currentElement[count+1]) && !isalpha(currentElement[count+1]) && currentElement[count+1] != '_' ) {
                endExp = &currentElement[count+1];
            }
            count++;
        }

        //Skips possible blanks after the number or rule
        else if((status == READING_OP1_NUMBER || status == READING_OP1_RULE)
        && (currentElement[count] == ' ')) count++;            

        //At the end of operand1, memory is reserved for it and gets copied into the expression struct
        else if((status == READING_OP1_NUMBER || status == READING_OP1_RULE)
        && (isOperator(currentElement[count]) || currentElement[count] == ' ')){
            //printf("FIN DE EXPRESION_1\n");
            if ( status == READING_OP1_NUMBER ) {
                opdread = malloc(sizeof(char)*(endExp-beginExp+1));
                memcpy(opdread, beginExp, sizeof(char)*(endExp-beginExp));
                opdread[endExp-beginExp]='\0';
                exp->opd1 = malloc(sizeof(int));
                *(exp->opd1) = atoi(opdread);
                free(opdread);
            } else { // status = READING_OP1_RULE
                opdread = malloc(sizeof(char)*(endExp-beginExp+1));
                memcpy(opdread, beginExp, sizeof(char)*(endExp-beginExp));
                opdread[endExp-beginExp]='\0';
                //printf("Searching for symbol %s\n",opdread);
                if( hashmap_get( executed_rules, opdread, (any_t*)&punteroMap) == MAP_MISSING ) {
                    //free(punteroMap);
                    punteroMap = malloc(sizeof(int));
                    *punteroMap = -1;
                    printf("regla '%s\n' no estan en el executed_dependant\n",opdread);
                }
                exp->opd1 = malloc(sizeof(int));
                *(exp->opd1) = *punteroMap;
                //printf("Regla leida: %s\n",opdread);
                //printf("Puntuacion regla: %d\n",*(exp->opd1));
                free(opdread);
            }
            //printf("EXPRESION_1 ES: %d\n",*(exp->opd1));
            status=END_OP1;
        }

        //Possible blanks after the operand are skipped
        else if(status == END_OP1 && currentElement[count]==' ') count++;            

        //Operator is read and copied into the expression struct
        else if(status == END_OP1 && isOperator(currentElement[count])){
            //exp->opt = malloc(sizeof(char)+1);
            exp->opt = currentElement[count];
            //printf("OPERADOR: %c\n",currentElement[count]);
            count++;
            status = BEGIN_OPERATOR;
            if ((currentElement[count] == '&' && currentElement[count-1] == '&') || (currentElement[count] == '|' && currentElement[count-1] == '|'))
                count++;

            status = BEGIN_OP2;
            //printf("INICIO DE EXPRESION_2\n");
        }

        //Lazy evaluation for operator &
        else if(status == BEGIN_OP2 && *(exp->opd1)==0 && exp->opt=='&') {
            aux = 1;
            while ( aux != 0 ) {
                if(currentElement[count] == '(') aux++;                    
                else if ( currentElement[count] == ')' ) aux--;

                if (aux!=0) count++;
            }
            exp->opd2 = malloc(sizeof(int));
            *(exp->opd2) = 0;
            status = END_OP2;
            //printf("Evaluacion perezosa de operador & realizada con exito.\n");
        }

        //Lazy evaluation for operator |
        else if(status == BEGIN_OP2 && *(exp->opd1)>0 && exp->opt=='|') {
            aux = 1;
            while ( aux != 0 ) {
                if(currentElement[count] == '(') aux++;                    
                else if ( currentElement[count] == ')' ) aux--;

                if (aux!=0) count++;
            }
            exp->opd2 = malloc(sizeof(int));
            *(exp->opd2) = 1;
            status = END_OP2;
            //printf("Evaluacion perezosa de operador | realizada con exito.\n");
        }

        //Possible blanks before the second operand are skipped
        else if(status == BEGIN_OP2 && currentElement[count] == ' ') count++;            

        //When the first number is found, status changes without moving the counter
        //and the memory direction of that char is saved
        else if (status == BEGIN_OP2 && isdigit(currentElement[count]) ) {
            status = READING_OP2_NUMBER;
            beginExp = &currentElement[count];
        }

        //If it founds an alphanumeric character, marks the beginning of the rule
        //to be read
        else if (status == BEGIN_OP2 && isalpha(currentElement[count]) ){
            status = READING_OP2_RULE;
            beginExp = &currentElement[count];
        }

        //While at reading number status, each number moves the counter
        else if(status == READING_OP2_NUMBER && isdigit(currentElement[count]) ){
            if (!isdigit(currentElement[count+1]))endExp = &currentElement[count+1];
            count++;
        }

        //While at reading rule status, each character or number moves the counter
        else if(status == READING_OP2_RULE && ( isdigit(currentElement[count])
        || isalpha(currentElement[count]) || currentElement[count]=='_')){
            if (!isdigit(currentElement[count+1]) && !isalpha(currentElement[count+1]) && currentElement[count+1] != '_' ) {
                    endExp = &currentElement[count+1];
            }
            count++;
        }

        //Skips possible blanks after the number or rule
        else if((status == READING_OP2_NUMBER || status == READING_OP2_RULE)
        && (currentElement[count] == ' ')){
            count++;
        }

        //At the end of operand2, memory is reserved for it and gets copied into the expression struct
        else if((status == READING_OP2_NUMBER || status == READING_OP2_RULE)
        && (currentElement[count] == ')' || isOperator(currentElement[count]) )){
            //printf("FIN DE EXPRESION_2\n");
            if ( status == READING_OP2_NUMBER ) {
                opdread = malloc(sizeof(char)*(endExp-beginExp+1));
                memcpy(opdread, beginExp, sizeof(char)*(endExp-beginExp));
                opdread[endExp-beginExp]='\0';
                exp->opd2 = malloc(sizeof(int));
                *(exp->opd2) = atoi(opdread);
                free(opdread);
            } else { // status = READING_OP2_RULE
                opdread = malloc(sizeof(char)*(endExp-beginExp+1));
                memcpy(opdread, beginExp, sizeof(char)*(endExp-beginExp));
                opdread[endExp-beginExp]='\0';
                if( hashmap_get( executed_rules, opdread, (any_t*)&punteroMap) == MAP_MISSING ) {
                    //free(punteroMap);
                    punteroMap =(int *)malloc(sizeof(int));
                    *punteroMap = 0;
                    printf("regla '%s\n' no estan en el executed_dependant\n",opdread);

                }
                exp->opd2 = malloc(sizeof(int));
                *(exp->opd2) = *punteroMap;
                //printf("Regla leida: %s\n",opdread);
                //printf("Puntuacion regla: %d\n",*(exp->opd1));
                free(opdread);
            }
	    //Evaluates if result of operand 1 was map_missing ( after evaluating operator )
	    if (*(exp)->opd1==-1) 
               (exp->opt=='&')?(*(exp)->opd1=1):(*(exp)->opd1=0);
	    

            //printf("EXPRESION_2 ES: %d\n",*(exp->opd2)); //
            status=END_OP2;
        }
        else if ( status == END_OP2 ) {
            result = operate(exp);
            if ( exp->neg == '1' ) result *= -1;
            //printf("Resultado de la expresion: %d\n", result);

            if ( currentElement[count] == ')' ) {
                //printf("antes de free 1 \n");
                free(exp->opd1);
                //printf("antes de free 2 \n");
                //printf("antes de free 3 \n");
                free(exp->opd2);
                //printf("antes de free 4 \n");
                //printf("antes de free final \n");
                free(exp);

                //printf( "El tamanyo de pila es: %d\n",getlengthstack(expStack) );
                if ( getlengthstack( expStack ) > 0 ){
                    pop_item( expStack, (element*)&recuperar );
                    exp = (expression *)recuperar;
                    if ( exp->opd1 == NULL ) {
                        //printf("Entro en la parte 1 \n");
                        exp->opd1 = malloc(sizeof(int));
                        *exp->opd1 = result;
                        status = END_OP1;
                    }else {
                        //printf("Entro en la parte 2 \n");
                        exp->opd2 = malloc(sizeof(int));
                        *exp->opd2 = result;
                        status = END_OP2;
                    }
                    //printf( "El tamaño de pila es: %d\n",getlengthstack(expStack) );
                }
                count++;
            } else { //If next element's an operator, result's placed as operand1
                free(exp->opd1);
                //free(exp->opt);
                free(exp->opd2);
                exp->opd1 = malloc(sizeof(int));
                *exp->opd1 = result;
                status = END_OP1;
            }
        }else {
            wblprintf(LOG_CRITICAL,"META","Error. Malformed expression. (>*o*)> \n");
            //printf("Me quede en: %c \n",currentElement[count]);
            //printf("El status es: %d \n",status);
            return MALFORMED_EXPRESION;
        }
    }
    //free(punteroMap);
    //hashmap_iterate_elements(map,&free_hashmap_data,NULL);
    //hashmap_free(map);
    free_stack(expStack, &free_stack_expression);
    return(result);
}

int count_meta_dependant_rules(char *expresion){
    char *start_element=expresion;
    int count=0;
    int num_params=0;
    int status=INIT;
    
    if(strlen(expresion)<=1) return num_params;
    
    while (start_element[count]!='\0'){
        if(status==INIT && isalpha(start_element[count]) ){
           status=BEGIN_RULE;
        }
        else{
            if(status==BEGIN_RULE && (start_element[count]==' ' || 
               start_element[count]=='(' || start_element[count]==')' ||
               isOperator(start_element[count]) ) ){
                status=INIT;
                num_params++;
            }
        }
        count++;
    }
    return num_params;  
}

int free_meta_item(element data){
    free((char *)data);
    return VECTOR_OK;
}

vector *parse_dependant_rules(char *expresion){
    int num_rules=0;
    vector *aux;
    char *start_element=expresion;
    char *begin=NULL, *end=NULL;
    int count=0;
    int processed_rules=0;
    int status=INIT;
    
    //printf("Expresion: %s\n",expresion);
    
    if( (num_rules=count_meta_dependant_rules(expresion)) ==0 ){
        wblprintf(LOG_CRITICAL,"META","Malformed meta expresion\n");
        return NULL;
    }
    
    if( (aux=new_vector(num_rules)) ==NULL){
        wblprintf(LOG_CRITICAL,"META","Not enougth memory\n");
        return NULL;
    }

    while( start_element[count]!='\0' ){
        if( status == INIT && isalpha(start_element[count]) ){
            status=BEGIN_RULE;
            begin=&start_element[count];
        }
        if( status == BEGIN_RULE && (start_element[count]==' ' || 
            start_element[count]=='(' || start_element[count]==')' ||
            isOperator(start_element[count]) ) ){
            status=INIT;
            end=&start_element[count];
            aux->v[processed_rules]=malloc(sizeof(char)*(end-begin+1));
            memcpy(aux->v[processed_rules],begin,sizeof(char)*(end-begin));
            ((char *)aux->v[processed_rules])[end-begin]='\0';
            processed_rules++;
        }
        
        count++;
    }
    
    if(status!=INIT){ 
        wblprintf(LOG_WARNING,"META","Malformed META expresion\n");
        free_meta_dependant_rules(aux);
        return NULL;
    }
    return aux;
}


void print_dependant_rules(vector *dep){
    int i=0;
    for(i=0;i<dep->size;i++){
        printf("[%d] - '%s'\n",i,(char *)dep->v[i]);
    }
}

void free_meta_dependant_rules(vector *v){
    free_vector(v,&free_meta_item);
}
 