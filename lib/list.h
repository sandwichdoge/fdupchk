#include <string.h>
#include <stdlib.h>
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node_t {  //linked list node
	void *str;
	struct node_t *next;
} node_t;


//add a new node to front of list, return ptr to it
node_t* list_add_front(node_t *head, void *str)
{
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	new_node->str = str;
	new_node->next = head;
	return new_node;
}


node_t* list_add(node_t *list, void *str)  //add a new node to end of list, return ptr to it
{
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	if (new_node == NULL) return NULL; //out of memory

	new_node->str = str;
	new_node->next = NULL;
	if (list == NULL) return new_node;
	while (list->next != NULL) {
		list = list->next;
	}
	list->next = new_node;
	return new_node;
}


node_t* list_findstr(node_t *list, char *str)  //return index of str in list
{
	int index = 0;
	while (list != NULL) {
		if (strcmp(list->str, str) == 0) return list;
		list = list->next;
		++index;
	}
	return NULL;  //not found
}


//insert a new node after head
int list_insert(node_t *head, void *str)
{
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	if (new_node == NULL) return -1; //out of memory
	new_node->str = str; //define new_node data

	node_t *nxt = head->next;
	head->next = new_node;
	new_node->next = nxt;

	return 0;
}


//free all nodes after head
void list_free(node_t *head)
{
	node_t *tmp;
	while (head != NULL) {
		tmp = head->next;
		free(head);
		head = tmp;
	}

	return;
}
#endif