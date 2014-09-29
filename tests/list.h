#ifndef _LIST_H_
#define _LIST_H_

#include <sys/types.h>

struct node {
	struct node *next;
	pid_t parent_id;
};

void remove_from_start(struct node **head);
void free_node(struct node *n);
void free_all_nodes(struct node **head);
int prepend(struct node **head, pid_t data);
pid_t get_data_from_start(struct node **head);

#endif
