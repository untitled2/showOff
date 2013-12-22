/*
 * Warning!
 * Tested only with gcc on linux.
 * Can result in undefined behaviour with other compilers / os's,
 * even if compilation goes without explicit errors.
 */

#include "apriori.h"

list_p freq_sets; /* contains lists of frequent sets */

/* Function definitions. */

inline char *my_strdup(const char *s) {
	char *p = malloc(strlen(s) + 1);
	if(p)
		strcpy(p, s);
	else
		return NULL;
	
	return p;
}

void print_usage(FILE *stream, int exit_code, const char* const program_name) {
	fprintf(stream, "Usage: %s OPTIONS\n", program_name);
	fprintf(stream,
			" -i  --input file	Mandatory argument. Input file containing transactions.\n"
			" -s  --support N	Minimum support threshold (%%).\n"
			" -h  --help		Print this usage information.\n"
	);

	exit(exit_code);
}

int isnumeric(const char *str) {
	while(*str)
		if(!isdigit(*(str++)))
			return 0;

	return 1;
}

int read_transactions(const char* const input_filename, list_p tractions) {
	FILE *in;
	char buf[LINE_MAX];
	int nl; /* a possible newline char */

	if(!(in = fopen(input_filename, "r"))) {
		perror("opening input file:");
		return 0;
	}

	/* Get transactions from file
		 and put them in data structure */
	while(fgets(buf, LINE_MAX, in)) {
		nl = strlen(buf)-1;
		/* Remove newline character, we don't need it. */
		if(buf[nl] == '\n')
			buf[nl] = '\0';

		/* Add transaction to list. */
		list_add(tractions, (void*)buf, strlen(buf)+1);
	}

	if(in)
		fclose(in);

	return 1;
}

int find_frequent_items(list_p tractions, list_p items, int min_sup) {
	int match;
	char *it, *tr_cp;
	const char* const delims = " .,:;";
	set_p new_set, cur_set;

	list_iter_p iter = list_iterator(tractions, FRONT);
	list_iter_p it_iter;

	while(list_next(iter)) {
		tr_cp = my_strdup((char*)list_current(iter));
		if(!tr_cp) {
			perror("malloc");
			exit(1);
		}
		/* Get first token from transaction. */
		it = strtok(tr_cp, delims);

		/* Go thru all items. */
		while(it) {
			/* Go thru list of items, increment sup_cnt of duplicate items. */
			it_iter = list_iterator(items, FRONT);
			do {
				match = 0;
				cur_set = (set_p)list_current(it_iter);
				if(cur_set && (!strcmp(cur_set->item, it))) {
					cur_set->sup_cnt++;
					match = 1;
					break;
				}
			} while(list_next(it_iter));
			free(it_iter);

			/* If not a duplicate, add to list. */
			if(!match) {
				/* Create item. */
				new_set = (set_p) malloc(sizeof(struct set));
				new_set->item = my_strdup(it);
				new_set->sup_cnt = 1;

				list_add(items, (void*)new_set, sizeof(*new_set));
				//free(new_set->item);
				free(new_set);
			}
			/* Get next item. */
			it = strtok(NULL, delims);
		}
		free(tr_cp);
	}
	free(iter);

	/* Remove items, that are not frequent. */
	if(!remove_infrequent_items(items, min_sup, tractions->length))
		return 0;

	return 1;
}

int remove_infrequent_items(list_p items, int min_sup, int tr_cnt) {
	if(!items->length)
		return 0;

	float sup; /* support in % */
	int cur_sup_cnt;
	set_p cur;
	list_iter_p iter = list_iterator(items, FRONT);

	/* Remove unfrequent items. */
	while(list_next(iter)) {
		cur = (set_p)list_current(iter);
		cur_sup_cnt = cur->sup_cnt;
		sup = ((float)cur_sup_cnt / (float)tr_cnt) * 100;

		if(sup < min_sup) {
			list_remove(items, (void*)list_current(iter));
		}
	}
	free(iter);

	if(!items->length)
		return 0;

	return 1;
}

void find_subsets(list_p items, int set_size, list_p sets, char *arr[], int q, int r) {
	int i;
	set_p new_set;
	char sub[LINE_MAX] = "";
	list_iter_p iter;

	if(q == set_size) {
		for(i=0; i<set_size; i++) {
			if(strlen(sub))
				strcat(sub, ",");
			strcat(sub, arr[i]);
		}
		new_set = (set_p) malloc(sizeof(struct set));
		if(!new_set) {
			perror("malloc:");
			exit(1);
		}

		list_add(sets, (void*)new_set, sizeof(*new_set));
		free(new_set);

		((set_p)list_last(sets))->item = my_strdup(sub);
		((set_p)list_last(sets))->sup_cnt = 0;
		*sub = '\0';
	}
	else {
		i = 0;
		iter = list_iterator(items, FRONT);
		while(i != r) {
			list_next(iter);
			i++;
		}
		while(list_next(iter)) {
			arr[q]=((set_p)list_current(iter))->item;
			find_subsets(items, set_size, sets, arr, q+1, i+1); /* recursion */
			i++;
		}
		free(iter);
	}
}

void remove_infrequent_subsets(list_p items, list_p sets, list_p prev_freq_sets, int size) {
	list_p subsets = create_list();
	list_iter_p sub_iter;
	list_iter_p pset_iter = list_iterator(prev_freq_sets, FRONT);
	char *arr[items->length + 1];

	/* Find all the size-1 subsets of sets. */
	find_subsets(items, size, subsets, arr, 0, 0);

	/* Check if each generated size-1 subset is frequent. */
	while(list_next(pset_iter)) {
		sub_iter = list_iterator(subsets, FRONT);
		while(list_next(sub_iter)) {
			if(!strcmp(((set_p)list_current(pset_iter))->item, ((set_p)list_current(sub_iter))->item)) {
				list_remove(subsets, (void*)list_current(sub_iter));
			}
		}
		free(sub_iter);
	}
	free(pset_iter);

	/* Now remove items from sets, that contain items in subsets. */
	remove_unused(sets, subsets);

	destroy_list(subsets);
}

void remove_unused(list_p from, list_p in) {
	list_iter_p from_iter = list_iterator(from, FRONT);
	list_iter_p in_iter;
	int found;

	if(from->length && in->length) {
		while(list_next(from_iter)) {
			in_iter = list_iterator(in, FRONT);
			if(list_current(in_iter) != NULL) {
				found = 0;
				while(list_next(in_iter)) {
					if(strstr(((set_p)list_current(from_iter))->item, ((set_p)list_current(in_iter))->item)) {
						found = 1;
						break;
					}
				}
				if(!found) {
					list_remove(from, (void*)list_current(from_iter));
				}
			}
			free(in_iter);
		}
	}
	free(from_iter);
}

list_p find_frequent_sets(list_p tractions, list_p items, int min_sup, int set_size) {
	list_p sets = create_list(); /* list containing subsets */
	char *arr[items->length + 1];

	/* First find unions. */
	find_subsets(items, set_size, sets, arr, 0, 0);

	/* Now, remove infrequent subsets from unions,
		 leaving only potential candidates. */
	if(set_size > 2 && (list_p)list_last(freq_sets))
		remove_infrequent_subsets(items, sets, (list_p)list_last(freq_sets), set_size-1);

	/* Then calculate their support count. */
	if(!sets->length)
		return NULL;

	list_iter_p tr_iter = list_iterator(tractions, FRONT);
	list_iter_p set_iter; 
	char *it, *set_cp, *tr;
	int cnt;
	set_p set;

	// for each transaction
	while(list_next(tr_iter)) {
		tr = (char*)list_current(tr_iter);
		// for each set
		set_iter = list_iterator(sets, FRONT);
		while(list_next(set_iter)) {
			set = (set_p)list_current(set_iter);
			set_cp = my_strdup(set->item);
			if(!set_cp) {
				perror("malloc:");
				exit(1);
			}
			it = strtok(set_cp, ",");
			cnt = 0;
			while(it) {
				/* Searching item it in transaction tr. */
				if(strstr(tr, it))
					cnt++;

				it = strtok(NULL, ",");
			}
			if(cnt == set_size)
				set->sup_cnt++;

			free(set_cp);
		}
		free(set_iter);
	}
	free(tr_iter);

	/* Finally eliminate unfrequent subsets.
		 Or return NULL when no more frequent subsets can be found */
	if(!remove_infrequent_items(sets, min_sup, tractions->length))
		return NULL;

	return sets;
}

int main(int argc, char *argv[]) {
	int next_option;
	int input_file_flag = 0;
	int min_sup = 50;
	const char *input_filename;
	list_p tractions = create_list(); /* list containing transactions */
	list_p items = create_list(); /* list containing items (struct set)*/
	freq_sets = create_list(); /* contains lists of frequent sets */
	list_iter_p iter; /* iterator */

	/* String listing valid short options. */
	const char* const short_options = "hi:s:";

	/* Array describing valid long options. */
	const struct option long_options[] = {
		{"help",				0,	NULL,	'h'},
		{"input",				1,	NULL,	'i'},
		{"support",			1,	NULL,	's'},
		{ NULL,					0,	NULL,	 0 }
	};

	/* Parse command line parameters. */
	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch(next_option) {
			case 'h':
				print_usage(stdout, 0, argv[0]);
			case 'i':
				input_filename = optarg;
				input_file_flag = 1;
				break;
			case 's':
				if(isnumeric(optarg))
					min_sup = atoi(optarg);
				break;
			case '?': /* Invalid option. */
				print_usage(stderr, 1, argv[0]);
			case -1: /* No more options. */
				if(!input_file_flag)
					print_usage(stderr, 1, argv[0]);
				break;
			default:
				abort();
		}
	} while(next_option != -1);

	/* Check if min_sup / min_conf specified correctly. */
	if(min_sup < TH_MIN || min_sup > TH_MAX) {
		printf("Minimum support dropped to default: 50.\n");
		min_sup = 50;
	}
	else
		printf("min_sup: %d\n", min_sup);

	printf("file: %s\n", input_filename);

	/* Read transactions from file. */
	if(!read_transactions(input_filename, tractions))
		return 1;

	/* Print transactions for testing purposes.
		 DO NOT USE THIS ON LARGE FILES. */
	/*
	if(tractions->length) {
		iter = list_iterator(tractions, FRONT);
		printf("transactions:\n");
		while(list_next(iter))
			printf("%s\n", (char*)list_current(iter));
		free(iter);
	}
	*/

	/* Find frequent items. */
	if(!find_frequent_items(tractions, items, min_sup)) {
		printf("No frequent items found.\n");
		return 0;
	}

	/* Print frequent items for testing purposes. */
	if(items->length) {
		iter = list_iterator(items, FRONT);
		printf("\nfrequent items:\n");
		while(list_next(iter)) {
			printf("%s\t", ((set_p)list_current(iter))->item);
			printf("%d\n", ((set_p)list_current(iter))->sup_cnt);
		}
		free(iter);
	}

	/* Make and print frequent n-sets for n >= 2. */
	list_p sets;
	int set_size = 2;
	while((sets = find_frequent_sets(tractions, items, min_sup, set_size))) {
		/* Print found frequent sets. */
		if(sets->length) {
			list_iter_p iter = list_iterator(sets, FRONT);
			printf("\nfrequent %d-itemsets:\n", set_size);
			while(list_next(iter)) {
				printf("%s\t", ((set_p)list_current(iter))->item);
				printf("%d\n", ((set_p)list_current(iter))->sup_cnt);
			}
			free(iter);
		}

		/* Add new sets into global list of sets. */
		list_add(freq_sets, (void*)sets, sizeof(*sets));
		free(sets);

		/* Free unneeded memory. */
		if(freq_sets->length > 2)
			destroy_list(list_poll(freq_sets));

		/* Get rid of items, not used in constructing new unions.
			 This is done because new unions are made utilizing k-subset
			 algorith, which, in turn, uses all the items. */
		remove_unused(items, (list_p)list_last(freq_sets));

		set_size++;
	}

	/* Let operating system free memory in a faster and more correct way. */
	/* Or free memory by hand with this code. */
	/*
	if(tractions)
		destroy_list(tractions);
	if(items)
		destroy_list(items);

	iter = list_iterator(freq_sets, FRONT);
	while(list_next(iter))
		destroy_list((list_p)list_current(iter));
	free(iter);
	*/

	return 0;
}
