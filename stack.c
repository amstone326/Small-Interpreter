#include "stack.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

void free_stack(stack *s) {
  stack_elt *elt = s->top;
  stack_elt *holder = NULL;
  while (elt) {
    holder = elt->beneath;
    free(elt);
    elt = holder;
  }
  free(s);
}

//returns an array of 2 ints: the first is the node's value, the second is the reg # it is in in PennSim
int pop(stack *s) { 
  stack_elt *popped;
  int to_return;

  if (s->top == NULL) {
    perror("Error: Pop called on empty stack. Imbalanced operations in code.");
    return INT_MIN;
  }
  else {
    popped = s->top;
    s->top = popped->beneath;
    to_return = popped->value;
    free(popped);
    return to_return;
  }
}

void push(stack *s, int new_value) { 
  stack_elt *new_elt;
  stack_elt *curr_top;

  curr_top = s->top;
  new_elt = malloc(sizeof(stack_elt));
  
  new_elt->value = new_value;
  if (curr_top) {
    new_elt->beneath = s->top;
  }
  else {
    new_elt->beneath = NULL;
  }

  s->top = new_elt;
}

//return but don't remove the top element
int poll(stack *s) {
  return s->top->value;
}
