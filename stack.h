/*
 *stack.h
 */

typedef struct stack_elt_tag {
  int value;
  struct stack_elt_tag *beneath;
} stack_elt;

typedef struct {
  stack_elt *top;
} stack;

int pop(stack *s); 
void push(stack *s, int new_value);
void free_stack(stack *s);
int poll(stack *s);  
