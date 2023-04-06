%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"

struct query query = {0};
struct extended_comparator* comparator;
size_t value_type;
size_t size = 0;

void print_tree();
void set_current_operation(uint8_t operation);
void set_current_value(uint64_t value, double float_value, char* field);
void append_value_setting(uint64_t value, double float_value, char* field);
void switch_filter();
void set_comparator();
void set_command(uint8_t command);
void* malloc_my(size_t size_of_struct);
void print_ram();
%}

%union {
uint64_t number;
char* string;
float float_number;
}

%token DB
%token FIND INSERT DELETE UPDATE
%token <string> PARENT STRING
%token SET OR
%token LT LTE GT GTE NE CONT
%token OPBRACE CLBRACE
%token OPCBRACE CLCBRACE
%token OPSQBRACE CLSQBRACE
%token COLON DOLLAR COMMA QUOTE
%token <number> FALSE TRUE INT_NUMBER
%token <float_number> FLOAT_NUMBER
%type <number> boolean value operation comparator

%%

print: query {print_tree();};

query:  DB FIND OPBRACE OPCBRACE filters CLCBRACE CLBRACE {set_command(0);}
        |
        DB DELETE OPBRACE OPCBRACE filters CLCBRACE CLBRACE {set_command(1);}
        |
        DB INSERT OPBRACE parent COMMA values_definition CLBRACE {set_command(2);}
        |
        DB UPDATE OPBRACE OPCBRACE filters CLCBRACE COMMA OPCBRACE DOLLAR SET COLON values_definition CLCBRACE CLBRACE {set_command(3);}
	    ;

parent: OPCBRACE PARENT COLON INT_NUMBER CLCBRACE {set_current_operation(0);
                                                   value_type = INT_TYPE;
                                                   set_current_value($4, 0, "parent");
                                                   switch_filter();
                                                  };

values_definition : OPCBRACE set_values CLCBRACE;

filters:    filter {switch_filter();}
	        | 
            filter COMMA filters {switch_filter();};

filter: STRING COLON value {set_current_operation(0);
                            float value;
                            if (value_type == FLOAT_TYPE) {
                                memcpy(&value, &$3, sizeof(uint64_t));
                                set_current_value(0, value, $1);
                            } else {
                                set_current_value($3, 0, $1);
                            }
                           }
	    |
	    PARENT COLON value {set_current_operation(0); set_current_value($3, 0, "parent");}
        |                                
	    STRING COLON operation {set_current_value($3, 0, $1);}
	    |
	    DOLLAR OR COLON OPSQBRACE filter COMMA filter CLSQBRACE {set_comparator();}
	    ;

operation: OPCBRACE DOLLAR comparator COLON value CLCBRACE {set_current_operation($3); $$ = $5;};
                                                              
set_values: set_value
	        |
	        set_value COMMA set_values

set_value: STRING COLON value {if (value_type == FLOAT_TYPE) {
                               float value;
                               memcpy(&value, &$3, sizeof(uint64_t));
                               append_value_setting(0, value, $1);
                               } else {
                                append_value_setting($3, 0, $1);
                               }
                              };
	    

value:  boolean {value_type = INT_TYPE; $$ = $1;}
	    |
	    INT_NUMBER {value_type = INT_TYPE; $$ = $1;}
        |
	    FLOAT_NUMBER {value_type = FLOAT_TYPE; memcpy(&$$, &$1, sizeof(uint64_t));}
        |
	    QUOTE STRING QUOTE {value_type = STRING_TYPE; $$ = $2;}
	    ;

boolean:    TRUE {$$ = 1;}
            |
            FALSE {$$ = 0;}
            ;

comparator: LT {$$ = 1;}
       	    |
            LTE {$$ = 2;}
            |
            GT {$$ = 3;}
            |
            GTE {$$ = 4;}
            |
            NE {$$ = 5;}
            |
            CONT {$$ = 6;}
            ;
%%

void yyerror(char *s) { 
    fprintf(stderr, "%s\n", s); 
}

void* malloc_my(size_t size_of_struct) {
    size += size_of_struct;
    return malloc(size_of_struct);
}

void append_value_setting(uint64_t value, double float_value, char* field) {
    struct value_setting* value_setting = malloc_my(sizeof(struct value_setting));
    struct field_value_pair field_value = {.field = field, .val_type = value_type};
    field_value.real_value = float_value;
    field_value.int_value = value;
    value_setting->field_value = field_value;
    value_setting->next = query.settings;
    query.settings = value_setting;
}

void set_current_operation(uint8_t operation) {
    struct extended_comparator* temp = malloc_my(sizeof(struct extended_comparator));
    temp->next = comparator;
    temp->operation = operation;
    comparator = temp;
}

void set_current_value(uint64_t value, double float_value, char* field) {
    struct field_value_pair field_value = {.field = field, .val_type = value_type};
    field_value.real_value = float_value;
    field_value.int_value = value;
    comparator->field_value = field_value;
}

void switch_filter() {
    struct filter* filter = malloc_my(sizeof(struct filter));
    struct comparator* temp = malloc_my(sizeof(struct comparator));
    filter->next = query.filters;
    if (comparator->connected) {
        temp->next = malloc_my(sizeof(struct comparator));
        temp->next->operation = comparator->connected->operation;
        temp->next->field_value = comparator->connected->field_value;
    }
    temp->operation = comparator->operation;
    temp->field_value = comparator->field_value;
    if (query.filters) {
        query.filters->comparator_list = temp;
    } else {
        filter->comparator_list = temp;
        query.filters = filter;
        filter = malloc_my(sizeof(struct filter));
        filter->next = query.filters;
    }
    comparator = comparator->next;
    query.filters = filter;
}

void set_comparator() {
    struct extended_comparator* temp = NULL;
    temp = comparator->next->next;
    comparator->connected = comparator->next;
    comparator->next = temp;
}

void set_command(uint8_t command) {
    query.command = command;
}

void print_ram() {
    printf("%zu bytes of RAM has been used\n", size);
}

void print_tree() {
    printf("command: %x\n", query.command);
    size_t filter_count = 0;
    size_t comparator_count = 0;
    printf(" filters:");
    while (query.filters) {
        if (query.filters->comparator_list) printf("  filter %zu:\n", filter_count++);
        while (query.filters->comparator_list) {
            char *field = query.filters->comparator_list->field_value.field;
            uint64_t value = query.filters->comparator_list->field_value.int_value;
            float float_value = query.filters->comparator_list->field_value.real_value;
            printf("   comparator %zu:\n", comparator_count++);
            printf("    field: %s\n    operation type: %d\n", field, query.filters->comparator_list->operation);
            switch (query.filters->comparator_list->field_value.val_type) {
                case STRING_TYPE:
                    printf("    value: %s\n", value);
                    break;
                case INT_TYPE:
                    printf("    value: %d\n", value);
                    break;
                case FLOAT_TYPE:
                    printf("    value: %f\n", float_value);
                    break;
            }
            query.filters->comparator_list = query.filters->comparator_list->next;
        }
        printf("\n");
        comparator_count = 0;
        query.filters = query.filters->next;
    }
    if (query.settings) {
        printf(" settings: \n");
    }
    while (query.settings) {
        printf("  field: %s\n", query.settings->field_value.field);
        switch (query.settings->field_value.val_type) {
            case STRING_TYPE:
                printf("  value: %s\n", query.settings->field_value.int_value);
                break;
            case INT_TYPE:
                printf("  value: %lu\n", query.settings->field_value.int_value);
                break;
            case FLOAT_TYPE:
                printf("  value: %f\n", query.settings->field_value.real_value);
                break;
        }
        printf("\n");
        query.settings = query.settings->next;
    }
    print_ram();
}