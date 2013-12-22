#ifndef __LIBDS_LIST_H__
#define __LIBDS_LIST_H__

/* A C implementation of a doubly-linked list. Contains void pointer values.
   Can be used as a LIFO stack of FIFO queue. */

#define FRONT 0
#define BACK 1

struct linked_node{
	void* data;
	struct linked_node* next;
	struct linked_node* prev;
};

typedef struct linked_node* lnode_p;

struct list{
	int length;
	lnode_p first;
	lnode_p last;
	void (*destructor)(void*);
};

typedef struct list * list_p;

struct list_iter{
	lnode_p current;
	char started;
};

typedef struct list_iter * list_iter_p;

/* Create a linked_list object. This pointer is created on the heap and must be
   cleared with a call to destroy_list to avoid memory leaks. */
list_p create_list();

/* Create a list_iter object for the linked_list list. The flag init can be 
   either FRONT or BACK and indicates whether to start the iterator from the first
   or last item in the list. */
list_iter_p list_iterator(list_p list, char init);

/* Add an item with the given value and size to the back of the list. 
   The data is copied by value, so the original pointer must be freed if it
   was allocated on the heap. */
void list_add(list_p list, void* data, int size);

/* Get data of the first element of the list. */
void* list_first(list_p list);

/* Get data of the last element of the list. */
void* list_last(list_p list);

/* Removes the last item in the list (LIFO order) and returns the data stored 
   there. The data returned must be freed later in order to remain memory safe. */
void* list_pop(list_p list);

/* Removes the first item in the list (FIFO order) and returns the data stored 
   there. The data return must be freed later in order to remain memory safe. */
void* list_poll(list_p list);

/* Remove item from the middle of the list. */
void* list_remove_middle(lnode_p node);

/* Convenience function for completely destroying an item in the list. */
void list_remove(list_p list, void *data);

/* Completely free the data associated with the list. */
void destroy_list(list_p list);

/* Return the data held by the current item pointed to by the iterator. */
void* list_current(list_iter_p list);

/* Advances the iterator to the next item in the list and returns the data 
   stored there. */
void* list_next(list_iter_p list);

/* Advances the iterator to the previous item in the list and returns the data 
   stored there. */
void* list_prev(list_iter_p list);

#endif
