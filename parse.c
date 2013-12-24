#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#define MAX_STRING_LEN 1000*sizeof(char)

int branch_label_count = 0;
int if_count = 0;

void callee_epilogue(FILE *out) {
  char* copy_RV;
  char* restore_SP;
  char* restore_FP;
  char* restore_RA;
  char* jump_to_R7;
  char* to_write;
  char* free_three_slots;

  to_write = malloc(MAX_STRING_LEN);
  copy_RV = "LDR R7 R6 #0\nSTR R7 R5 #2\n";
  restore_SP = "ADD R6 R5 #0\n"; //SP moves down stack to FP 
  restore_FP = "LDR R5 R6 #0\n"; //b/c R6 points to the caller's FP
  restore_RA = "LDR R7 R6 #1\n"; //1 below R6 in stack stores RA
  free_three_slots = "ADD R6 R6 #3\n";
  jump_to_R7 = "JMPR R7\n\n";

  strcpy(to_write, copy_RV);
  strcat(to_write, restore_SP);
  strcat(to_write, restore_FP);
  strcat(to_write, restore_RA);
  strcat(to_write, free_three_slots);
  strcat(to_write, jump_to_R7);
  fwrite(to_write, sizeof(char), strlen(to_write), out);
  free(to_write);
}

void callee_preamble(FILE *out) {
  char* three_stack_slots;
  char* save_RA;
  char* save_FP;
  char* set_FP;
  char* to_write;
  
  to_write = malloc(MAX_STRING_LEN);
  three_stack_slots = "ADD R6 R6 #-3\n";
  save_RA = "STR R7 R6 #1\n";
  save_FP = "STR R5 R6 #0\n";
  set_FP = "ADD R5 R6 #0\n"; //sets FP to SP

  strcpy(to_write, three_stack_slots);
  strcat(to_write, save_RA);
  strcat(to_write, save_FP);
  strcat(to_write, set_FP);
  fwrite(to_write, sizeof(char), strlen(to_write), out);
  free(to_write);
}

int parse_arguments(list *arg_list, FILE *out, stack *the_stack) {
  char *next_arg;
  int parse_fail;
 
  while (arg_list->head) {
    next_arg = remove_front(arg_list);
    parse_fail = parse_arg(next_arg, out, arg_list, the_stack);
    if (parse_fail) {
      return 1;
    }
    free(next_arg);
  }

  return 0;
}

int parse_arg(char *argument, FILE *out, list *arg_list, stack *the_stack) {
  char *store_local;
  char *decimal_base;
  char *hex_base;
  char *to_write;
  char *hex_hiconst_base;
  char *dec_hiconst_base;
  int i;
  long value;
  int lower_8_bits;
  char *const_num_string;
  int upper_8_bits;
  char *hiconst_num_string;
  unsigned int N;
  int invalid_operator;

  N = strlen(argument);
  to_write = malloc(MAX_STRING_LEN);
  decimal_base = "CONST R7 #";
  hex_base = "CONST R7 ";
  dec_hiconst_base = "\nHICONST R7 #";
  hex_hiconst_base = "\nHICONST R7 ";
  store_local = "\nSTR R7 R6 #-1 \nADD R6 R6 #-1\n"; //puts value in R7 at top of stack and then moves SP up 1

  //check if arg is a hex literal
  if (argument[0] == '0' && argument[1] == 'x') {
    for (i = 2; i < N; i++) {
      if (!isxdigit(argument[i])) {
	printf("parse_argument fails because of invalid hex literal");
	free(to_write);
	return 1;
      }
    }
    
    value = strtol(argument, NULL, 0);
    if (value < -256 || value > 255) {
      const_num_string = malloc(MAX_STRING_LEN);
      hiconst_num_string = malloc(MAX_STRING_LEN);
      lower_8_bits = value & 0xFF;
      upper_8_bits = (value >> 8) & 0xFF;
      sprintf(const_num_string, "%d", lower_8_bits);
      sprintf(hiconst_num_string, "%d", upper_8_bits);
      strcpy(to_write, hex_base);
      strcat(to_write, const_num_string);
      strcat(to_write, hex_hiconst_base);
      strcat(to_write, hiconst_num_string);
      free(const_num_string);
      free(hiconst_num_string);
    }
    else {
      strcpy(to_write, hex_base);
      strcat(to_write, argument);
    }
    strcat(to_write, store_local);    
    fwrite(to_write, sizeof(char), strlen(to_write), out);
  }

  //check if arg is a decimal literal
  else if ((argument[0] == '-' && N > 1) || isdigit(argument[0])) {
    for (i = 1; i < N; i++) {
      if (!isdigit(argument[i])) {
	printf("parse_argument fails because of invalid decimal literal");
	free(to_write);
	return 1;
      }
    }
    value = atoi(argument);
    if (value < -256 || value > 255) {
      const_num_string = malloc(MAX_STRING_LEN);
      hiconst_num_string = malloc(MAX_STRING_LEN);
      lower_8_bits = value & 0xFF;
      upper_8_bits = (value >> 8) & 0xFF;
      sprintf(const_num_string, "%d", lower_8_bits);
      sprintf(hiconst_num_string, "%d", upper_8_bits);
      strcpy(to_write, decimal_base);
      strcat(to_write, const_num_string);
      strcat(to_write, dec_hiconst_base);
      strcat(to_write, hiconst_num_string);
      free(const_num_string);
      free(hiconst_num_string);
    }
    else {
      strcpy(to_write, decimal_base);
      strcat(to_write, argument);
    }
    strcat(to_write, store_local);    
    fwrite(to_write, sizeof(char), strlen(to_write), out);
  }

  else {
    invalid_operator = parse_operator(argument, out, arg_list, the_stack);
    if (invalid_operator) {
      printf("parse_argument fails because of invalid operator: %s\n", argument);
      free(to_write);
      return 1;
    }
  }

  free(to_write);
  return 0;
}

enum argument_type get_type(char *argument) {
  if (strcmp(argument, "+") == 0) {
    return ARITH;
  }
  if (strcmp(argument, "-") == 0) {
    return ARITH;
  }
  if (strcmp(argument, "*") == 0) {
    return ARITH;
  }
  if (strcmp(argument, "/") == 0) {
    return ARITH;
  }
  if (strcmp(argument, "/") == 0) {
    return ARITH;
  }
  if (strcmp(argument, "%") == 0) {
    return ARITH;
  }
  if (strcmp(argument, "lt") == 0) {
    return COMP;
  }
  if (strcmp(argument, "le") == 0) {
    return COMP;
  }
  if (strcmp(argument, "eq") == 0) {
    return COMP;
  }
  if (strcmp(argument, "ge") == 0) {
    return COMP;
  }
  if (strcmp(argument, "gt") == 0) {
    return COMP;
  }
  if (strcmp(argument, "and") == 0) {
    return LOGICAL;
  }
  if (strcmp(argument, "or") == 0) {
    return LOGICAL;
  }
  if (strcmp(argument, "not") == 0) {
    return LOGICAL;
  } 
  if (strcmp(argument, "drop") == 0) {
    return STACK;
  }
  if (strcmp(argument, "dup") == 0) {
    return STACK;
  }
  if (strcmp(argument, "swap") == 0) {
    return STACK;
  }
  if (strcmp(argument, "rot") == 0) {
    return STACK;
  }
  if (strcmp(argument, "pick") == 0) {
    return STACK;
  }
  if (strcmp(argument, "defun") == 0) {
    return FUNCTION_DEF;
  }
  if (strcmp(argument, "if") == 0) {
    return IF_ELSE;
  }
  return FUNCTION_CALL;
}

int parse_operator(char *argument, FILE *out, list* arg_list, stack *the_stack) {
  enum argument_type t;
  char *to_write;
  char *next_arg;
  char *add_string;
  char *sub_string;
  char *mul_string;
  char *div_string;
  char *mod_string;
  char *and_string;
  char *or_string;
  char *not_string;
  char *comp_string;
  char *lt_string;
  char *le_string;
  char *eq_string;
  char *ge_string;
  char *gt_string;
  char *get_two_args;
  char *get_third_arg;
  char *get_one_arg;
  char *store_result;
  char *drop_string;
  char *dup_string;
  char *swap_string;
  char *rot_string;
  char *pick_string;
  char *branch_label_base;
  char *falign_string;
  char *load_1;  
  char *load_0;
  char *jsr_base;
  char *cmp_store_result;
  char *branch_label_num;
  char *else_base;
  char *ifelse_num;
  char *comp_to_zero;
  char *after_fun_call;
 
  ifelse_num = malloc(MAX_STRING_LEN);
  sprintf(ifelse_num, "%d\n", if_count);
  after_fun_call = "ADD R6 R6 #-1\n";
  comp_to_zero = "CONST R0 #0\nCMP R1 R0\n";
  else_base = "BRz ELSE_";
  jsr_base = "JSR ";
  falign_string = ".FALIGN\n";
  add_string = "ADD R1 R1 R2\n";
  sub_string = "SUB R1 R1 R2\n";
  mul_string = "MUL R1 R1 R2\n";
  div_string = "DIV R1 R1 R2\n";
  mod_string = "MOD R1 R1 R2\n";
  and_string = "AND R1 R1 R2\n";
  or_string = "OR R1 R1 R2\n";
  not_string = "NOT R1 R1 R2\n";
  comp_string = "CMP R1 R2\n";
  lt_string = "BRn ";
  le_string = "BRnz ";
  eq_string = "BRz ";
  ge_string = "BRzp ";
  gt_string = "BRp ";
  load_1 = "CONST R0 #1\n";
  load_0 = "CONST R0 #0\n";
  cmp_store_result = "STR R0 R6 #-1\nADD R6 R6 #-1\n";
  drop_string = "ADD R6 R6 #1\n";
  dup_string = "LDR R1 R6 #0\nSTR R1 R6 #-1\nADD R6 R6 #-1\n";
  swap_string = "STR R1 R6 #-1\nADD R6 R6 #-1\nSTR R2 R6 #-1\nADD R6 R6 #-1\n";
  rot_string = "STR R2 R6 #-1\nADD R6 R6 #-1\nSTR R1 R6 #-1\nADD R6 R6 #-1\nSTR R3 R6 #-1\nADD R6 R6 #-1\n";
  pick_string = "ADD R1 R6 R1\nLDR R1 R1 #0\n";
  get_one_arg = "LDR R1 R6 #0\nADD R6 R6 #1\n";
  get_two_args = "LDR R1 R6 #0\nADD R6 R6 #1\nLDR R2 R6 #0\nADD R6 R6 #1\n";  
  get_third_arg = "LDR R3 R6 #0\nADD R6 R6 #1\n";
  store_result = "STR R1 R6 #-1\nADD R6 R6 #-1\n";
  to_write = malloc(MAX_STRING_LEN);
  branch_label_num = malloc(MAX_STRING_LEN);
  branch_label_base = malloc(MAX_STRING_LEN);
  strcpy(branch_label_base, "AFTERBRANCH_");
  t = get_type(argument);
  next_arg = malloc(MAX_STRING_LEN);
  
  if (t == ARITH || t == LOGICAL) {
    //1) Remove the top 2 args from the stack
     strcpy(to_write, get_two_args);

    //2) Determine operation and add string
    if (strcmp(argument, "+") == 0) {
      strcat(to_write, add_string);
    }
    else if (strcmp(argument, "-") == 0) {
      strcat(to_write, sub_string);
    }
    else if (strcmp(argument, "*") == 0) {
      strcat(to_write, mul_string);
    }
    else if (strcmp(argument, "/") == 0) {
      strcat(to_write, div_string);
    }
    else if (strcmp(argument, "%") == 0) {
      strcat(to_write, mod_string);
    }
    else if (strcmp(argument, "and") == 0) {
      strcat(to_write, and_string);
    }
    else if (strcmp(argument, "or") == 0) {
      strcat(to_write, or_string);
    }
    else { //not
      strcat(to_write, not_string);
    }

    //3) store result back onto stack
    strcat(to_write, store_result);
    fwrite(to_write, sizeof(char), strlen(to_write), out);
  }
  
  else if (t == COMP) {
    //1) Put 2 args at top of stack into R1 and R2, compare R1 to R2
    strcpy(to_write, load_1); 
    strcat(to_write, get_two_args);
    strcat(to_write, comp_string);

    if (strcmp(argument, "lt") == 0) {
      strcat(to_write, lt_string);
    }
    else if (strcmp(argument, "le") == 0) {
      strcat(to_write, le_string);
    }
    else if (strcmp(argument, "eq") == 0) {
      strcat(to_write, eq_string);
    }
    else if (strcmp(argument, "gt") == 0) {
      strcat(to_write, gt_string);      
    }
    else { //ge
      strcat(to_write, ge_string);
    }
    sprintf(branch_label_num, "%d\n", branch_label_count++);
    strcat(branch_label_base, branch_label_num);
    
    strcat(to_write, branch_label_base);
    strcat(to_write, load_0);
    strcat(to_write, branch_label_base);
    strcat(to_write, cmp_store_result);
    fwrite(to_write, sizeof(char), strlen(to_write), out);
  }

  else if (t == STACK) {
    if (strcmp(argument, "drop") == 0) {
      strcpy(to_write, drop_string);
    }
    else if (strcmp(argument, "dup") == 0) {
      strcpy(to_write, dup_string);
    }
    else if (strcmp(argument, "swap") == 0) {
      strcpy(to_write, get_two_args);
      strcat(to_write, swap_string);
    }
    else if (strcmp(argument, "rot") == 0) {
      strcpy(to_write, get_two_args);
      strcat(to_write, get_third_arg);
      strcat(to_write, rot_string);
    }
    else { //pick
      strcpy(to_write, get_one_arg);
      strcat(to_write, pick_string);
      strcat(to_write, store_result);
    }
    fwrite(to_write, sizeof(char), strlen(to_write), out);
  }

  else if (t == FUNCTION_DEF) {
    fwrite(falign_string, sizeof(char), strlen(falign_string), out);
    free(next_arg);
    next_arg = remove_front(arg_list);
    strcat(next_arg, "\n");
    fwrite(next_arg, sizeof(char), strlen(next_arg), out); //write out function name as label
    free(next_arg);
    callee_preamble(out);
    next_arg = remove_front(arg_list);
    while (strcmp(next_arg, "return")) { //while next_arg != "return"
      parse_arg(next_arg, out, arg_list, the_stack);
      free(next_arg);
      next_arg = remove_front(arg_list);
    }
    callee_epilogue(out);
  }

  else if (t == FUNCTION_CALL) {
    strcpy(to_write, jsr_base);
    strcat(to_write, argument);
    strcat(to_write, "\n");
    strcat(to_write, after_fun_call);
    fwrite(to_write, sizeof(char), strlen(to_write), out);
  }

  else { //if token
    push(the_stack, if_count);
    fwrite(get_one_arg, sizeof(char), strlen(get_one_arg), out);
    fwrite(comp_to_zero, sizeof(char), strlen(comp_to_zero), out);
    fwrite(else_base, sizeof(char), strlen(else_base), out);
    fwrite(ifelse_num, sizeof(char), strlen(ifelse_num), out);
    write_ifelse_functions(out, arg_list, the_stack);
  }
  
  free(ifelse_num);
  free(branch_label_base);
  free(branch_label_num);
  free(next_arg);
  free(to_write);
  return 0;
}

void write_ifelse_functions(FILE *out, list* arg_list, stack *the_stack) {
  char *if_num_string;
  int else_num;
  char *else_num_string;
  char *if_label;
  char *else_label;
  char *BRnzp;
  char *after_else;
  char *next_arg;

  //if section uses current if_count and then increments it
  if_num_string = malloc(MAX_STRING_LEN);
  sprintf(if_num_string, "%d\n", if_count++);
  if_label = malloc(MAX_STRING_LEN);
  strcpy(if_label, "IF_");
  strcat(if_label, if_num_string);
  BRnzp = malloc(MAX_STRING_LEN);
  strcpy(BRnzp, "BRnzp AFTERELSE_");
  strcat(BRnzp, if_num_string);

  //else section uses number from off the stack
  else_num_string = malloc(MAX_STRING_LEN);
  else_num = pop(the_stack);
  sprintf(else_num_string, "%d\n", else_num);
  else_label = malloc(MAX_STRING_LEN);
  strcpy(else_label, "ELSE_");
  strcat(else_label, else_num_string);
  after_else = malloc(MAX_STRING_LEN);
  strcpy(after_else, "AFTERELSE_");
  strcat(after_else, else_num_string);

  //write if section
  fwrite(if_label, sizeof(char), strlen(if_label), out);
  next_arg = remove_front(arg_list);
  while (strcmp(next_arg, "else") && strcmp(next_arg, "endif")) { //while next_arg != "else" or "endif"
    parse_arg(next_arg, out, arg_list, the_stack);
    free(next_arg);
    next_arg = remove_front(arg_list);
  }
  fwrite(BRnzp, sizeof(char), strlen(BRnzp), out);

  //write else section
  fwrite(else_label, sizeof(char), strlen(else_label), out);
  if (strcmp(next_arg, "else") == 0) {
    free(next_arg);
    next_arg = remove_front(arg_list);
    while(strcmp(next_arg, "endif")) { //while next_arg != "endif"
      parse_arg(next_arg, out, arg_list, the_stack);
      free(next_arg);
      next_arg = remove_front(arg_list);
    }
  }
  fwrite(after_else, sizeof(char), strlen(after_else), out);

  //frees
  free(next_arg);
  free(if_num_string);
  free(else_num_string);
  free(if_label);
  free(else_label);
  free(BRnzp);
  free(after_else);
}
