#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

list *create_list() {
  list* the_list;
  the_list = malloc(sizeof(list));
  the_list->head = NULL;
  the_list->tail = NULL;
  return the_list;
}

//adds to end
void add_end(list* the_list, char* arg_name) {
  node* new_elt;
  new_elt = malloc(sizeof(node));
  //allocate space to store name                                     
  new_elt->argument = malloc(strlen(arg_name) + 1); //+1 is for null termination
  strcpy (new_elt->argument, arg_name); //copies the string from input arg to elt->name                         
  //add element to list                                                        
  new_elt->next = NULL;
  if (the_list->head) {
    the_list->tail->next = new_elt;
    the_list->tail = new_elt;
  }
  else {
    the_list->head = new_elt;
    the_list->tail = new_elt;
  }
}

//removes from front
char *remove_front(list* the_list) {
  node *removed = the_list->head;
  char *to_return;
  the_list->head = removed->next;
  if (the_list->head == NULL) {
    the_list->tail = NULL;
  }
  to_return = malloc(strlen(removed->argument)+10); 
  strcpy(to_return, removed->argument);
  free(removed->argument);
  free(removed);
  return to_return;
}

void free_list(list* the_list) {
  node* n = the_list->head;
  node* holder = NULL;
  while (n != NULL) {
    holder = n->next;
    free(n->argument);
    free(n);
    n = holder;
  }
  free(the_list->head);
  free(the_list->tail);
  free(the_list);
}
