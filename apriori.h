#ifndef __APRIORI_H__
#define __APRIORI_H__

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

#include "list.h"

#define TH_MIN 0
#define TH_MAX 100
#define LINE_MAX 256


/* Data structure for item set. */
struct set {
	char *item; /* item name */
	int sup_cnt; /* support */
};

typedef struct set * set_p;

/* Function declarations. */

/* String duplication function for portability.
	 Makes a copy of s on the heap and returns pointer to it. */
char *my_strdup(const char *s);

/* Print usage information to stream, specified by caller. */
void print_usage(FILE *stream, int exit_code, const char* const program_name);

/* Check if a string contains a number, return 1 if it does,
	 0 otherwise. */
int isnumeric(const char *str);

/* Read transactions from a file and put them in linked list. */
int read_transactions(const char* const input_filename, list_p tractions);

/* Pick up frequent items and their support counts from transactions. */
int find_frequent_items(list_p tractions, list_p items, int min_sup);

/* Help function, find_frequent_items() calls this to eliminate infrequent items. */
int remove_infrequent_items(list_p items, int min_sup, int tr_cnt);

/* Remove sets or items from list 'from', that occur in list 'in'. */
void remove_unused(list_p from, list_p in);

/* Find unions of given size. */
void find_unions(list_p list, int size);

/* Find subsets of given size. */
void find_subsets(list_p items, int set_size, list_p sets, char *arr[], int q, int r);

/* Find frequent itemsets of given size and based on minimum support. */
list_p find_frequent_sets(list_p tractions, list_p items, int min_sup, int set_size);

#endif
