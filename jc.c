#include "parse.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_LINE_LENGTH 1000*sizeof(char)

int main(int argc, char** argv) {
  FILE *the_file;
  FILE *output_file;
  char *next_arg;
  char *in_filename;
  int filename_length;
  char *out_filename;
  char *next_line;
  list *argument_list;
  int some_arguments_invalid;
  char *code_header;
  stack *the_stack;
  char *asm_tag;
  
  the_stack = malloc(sizeof(stack));
  code_header = ".CODE\n\n";
  asm_tag = ".asm";
  in_filename = malloc(MAX_LINE_LENGTH);
  strcpy(in_filename, argv[1]);
  the_file = fopen(in_filename, "r");
  if (the_file == NULL) {
    perror("Error opening input file.\n");
    return 1;
  }
  filename_length = strlen(in_filename);
  out_filename = malloc(filename_length + 4*sizeof(char));
  strncpy(out_filename, in_filename, (filename_length - 2));
  strcat(out_filename, asm_tag);
  output_file = fopen(out_filename, "w");
  if (output_file == NULL) {
    perror("Error opening output file.\n");
    return 1;
  }

  argument_list = create_list();
  next_line = malloc(MAX_LINE_LENGTH);
  
  while (!feof(the_file)) {
    fgets(next_line, MAX_LINE_LENGTH, the_file);
    if (strcmp(next_line, "\n") == 0) {
      continue;
    }
    next_arg = strtok(next_line, " \n\t");
    while (next_arg) { //while strtok does not return NULL   
      if (next_arg[0] == ';') {
        break; //stop processing this line if reach a ';'                                          
      }
      else {
	add_end(argument_list, next_arg);
      }
      next_arg = strtok(NULL, " \n\t");
    }
  }

  fclose(the_file);
  free(in_filename);
  free(next_line);
  free(next_arg);

  fwrite(code_header, sizeof(char), strlen(code_header), output_file);

  some_arguments_invalid = parse_arguments(argument_list, output_file, the_stack);
  if (some_arguments_invalid) {
    printf("COMPILE ERROR: Some input arguments are invalid.\n");
  }
  
  //free list and output file at very end
  free(out_filename);
  free_list(argument_list);
  free_stack(the_stack);
  fclose(output_file);
  return 0;
}  
