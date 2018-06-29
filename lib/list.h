#include <string.h>
#include <stdlib.h>
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node_t {  //linked list node
	void *str;
	struct node_t *next;
} node_t;


node_t* list_add(node_t *list, void *str)  //add a new node to end of list, return ptr to it
{
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	new_node->str = str;
	new_node->next = NULL;
	if (list == NULL) return new_node;
	while (list->next != NULL) {
		list = list->next;
	}
	list->next = new_node;
	return new_node;
}


int list_findstr(node_t *list, char *str)  //return index of str in list
{
	int index = 0;
	while (list != NULL) {
		if (strcmp(list->str, str) == 0) return index;
		list = list->next;
		index++;
	}
	return -1;  //not found
}


node_t* list_insert(node_t *list, void *str, int pos)  //if pos is more than list bound, add new member to end of list
{
	node_t *prev;
	while (pos-- >= 0) {
		prev = list;
		list = list->next;
		if (list == NULL) break;
	}
	node_t *new_node = (node_t*)malloc(sizeof(node_t));
	new_node->str = str;
	
	new_node->next = list;
	prev->next = new_node;

	return new_node;
}


void list_free(node_t *list)
{
	node_t *tmp;
	while (list != NULL) {
		tmp = list->next;
		free(list);
		list = tmp;
	}

	return;
}
#endif