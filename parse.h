/*
 *parse.h
 */

#include <ctype.h>
#include <stdio.h>
#include "list.h"
#include "stack.h"

enum argument_type {ARITH, COMP, LOGICAL, STACK, FUNCTION_DEF, FUNCTION_CALL, IF_ELSE};
int parse_arguments(list *arg_list, FILE *out, stack *the_stack);
int parse_arg(char *argument, FILE *out, list *arg_list, stack *the_stack);
int parse_operator(char *argument, FILE *out, list *arg_list, stack *the_stack);
void create_base_functions(FILE *out);
void callee_preamble(FILE *out);
void callee_epilogue(FILE *out);
void write_ifelse_functions(FILE *out, list *arg_list, stack *the_stack);
