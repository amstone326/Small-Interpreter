/*list.h header file */

typedef struct node_tag {
  char* argument;
  struct node_tag *next;
} node;

typedef struct {
  node *head;
  node *tail;
} list;

char *remove_front(list* the_list); //removes head of list
void add_end(list* the_list, char* arg_name); //adds to tail of list
void free_list(list *the_list);
list *create_list();


