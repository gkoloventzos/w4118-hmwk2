#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include "list.h"

void remove_from_start(struct node **head)
{
	struct node *temp;

	temp = *head;
	*head = (*head)->next;
	free(temp);
}

void free_node(struct node *n)
{
	free(n);
}

void free_all_nodes(struct node **head)
{
	struct node *cur_node = NULL;
	struct node *prev = NULL;

	if (head == NULL)
		return;

	cur_node = *head;
	while (cur_node) {
		prev = cur_node;
		cur_node = cur_node->next;
		free_node(prev);
	}
}

int prepend(struct node **head, pid_t data)
{
	struct node *new_node;

	/* Create a new node */
	new_node = malloc(sizeof(struct node));
	if (new_node == NULL)
		return 1;

	new_node->parent_id = data;
	new_node->next = NULL;
	if (*head != NULL) {
		new_node->next = *head;
		*head = new_node;
	} else {
		*head = new_node;
	}
	return 0;
}

pid_t get_data_from_start(struct node **head)
{
	if (*head != NULL)
		return (*head)->parent_id;
	return (pid_t)0;
}

void print(struct node *list)
{ while (list != NULL) {       /* Visit list elements up to the end */
    printf("%ld--> ", (long)list->parent_id);         /* Print current element */
    list = list->next;                        /* Go to next element */
  }
  printf("NULL\n");                            /* Print end of list */
}
