#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

list_p create_list(){
	list_p list = (list_p) malloc(sizeof(struct list));
	list->length = 0;
	list->first = NULL;
	list->last = NULL;
	list->destructor = free;
	return list;
}

list_iter_p list_iterator(list_p list, char init){
	list_iter_p iter = (list_iter_p)malloc(sizeof(struct list_iter));

	if(init==FRONT){
		iter->current = list->first;
	}
	else if(init==BACK){
		iter->current = list->last;
	}
	else
		return NULL;

	iter->started = 0;

	return iter;
}

void list_add(list_p list, void* data, int size){
	lnode_p node = (lnode_p)malloc(sizeof(struct linked_node));
	node->data = malloc(size);
	memcpy(node->data, data, size);
	
	if(list->first==NULL){
		node->prev = NULL;
		node->next = NULL;
		list->first = node;
		list->last = node;
	}
	else{
		list->last->next = node;
		node->prev = list->last;
		node->next = NULL;
		list->last = node;
	}
	list->length++;
}

inline void* list_current(list_iter_p iter) {
	if(iter->started && iter->current)
		return iter->current->data;

	return NULL;
}

inline void* list_next(list_iter_p iter) {
	if(!iter->started && iter->current) {
		iter->started = 1;
		return iter->current->data;
	}

	if(iter->current) {
		iter->current = iter->current->next;
		return list_current(iter);
	}

	return NULL;
}

void* list_prev(list_iter_p iter){
	if(!iter->started && iter->current){
		iter->started=1;
		return iter->current->data;
	}

	if(iter->current){
		iter->current = iter->current->prev;
		return list_current(iter);
	}

	return NULL;
}

void* list_first(list_p list){
	return list->first->data;
}

void* list_last(list_p list){
	return list->last->data;
}

void* list_pop(list_p list){
	lnode_p last = list->last;

	if(last == NULL)
		return NULL;

	void* data = last->data;

	if(last->prev) {
		list->last = last->prev;
		last->prev->next = NULL;
	}

	free(last); /////////////
	return data;
}

void* list_poll(list_p list){
	lnode_p first = list->first;

	if(!first)
		return NULL;

	void* data = first->data;

	if(first->next) {
		list->first = first->next;
		first->next->prev = NULL;
	}

	free(first); ////////////
	return data;
}

void list_remove(list_p list, void *data) {
	void *tmp = NULL;
	lnode_p cur = list->first;

	if(!cur) {
		return;
	}

	while(cur) {
		if(cur->data == data) {
			if(cur == list->first) {
				tmp = list_poll(list);
				break;
			}
			else if(cur == list->last) {
				tmp = list_pop(list);
				break;
			}
			else {
				tmp = list_remove_middle(cur);
				break;
			}
		}
		cur = cur->next;
	}

	if(tmp) {
		list->destructor(tmp); //////////////
		list->length--;
	}
}

void destroy_list(list_p list) {
	lnode_p cur = list->first;
	lnode_p next;

	while(cur) {
		next = cur->next;
		if(list->length != 0) {
			list->destructor(cur->data);
			free(cur);
		}
		cur = next;
	}

	free(list);
}

void* list_remove_middle(lnode_p node) {
	void *data = node->data;
	node->prev->next = node->next;
	node->next->prev = node->prev;

	free(node); //////////

	return data;
}
