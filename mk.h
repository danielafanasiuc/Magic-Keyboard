#ifndef _MK_H
#define _MK_H

#include <errno.h>
#include <assert.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)

#define ALPHABET_SIZE 26
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"
#define MAX_OPERATION 50
#define MAX_KEY 256
#define MAX_FILENAME 1024

typedef struct trie_node_t trie_node_t;
struct trie_node_t {
	// Value associated with key (set if end_of_word = 1)
	int frequency;

    // the shortest path to a word
	int shortest_path;

	// 1 if current node marks the end of a word, 0 otherwise
	int end_of_word;

	trie_node_t **children;
	int n_children;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	// Trie-Specific, alphabet properties
	int alphabet_size;
	char *alphabet;
};

trie_node_t *trie_create_node(void);
trie_t *trie_create(int alphabet_size, char *alphabet);
void __trie_insert(trie_node_t *node, char *key);
// helper function to insert in trie
void trie_insert(trie_t *trie, char *key);

int get_minimum_child(trie_node_t *node);

// helper function to remove from trie
int __trie_remove(trie_node_t *node, char *key);
void trie_remove(trie_t *trie, char *key);
// helper function to free trie
void __trie_free(trie_node_t *node);
void trie_free(trie_t *trie);

void load_file(char *filename, trie_t *trie);

// helper function to autocorrect
void __autocorrect(trie_node_t *node, char *key, char *similar_key, int k,
				   int offset, int *ok);
void autocorrect(trie_t *trie, char *key, int k);

void find_smallest_word(trie_node_t *node);
void first_criteria(trie_node_t *node, char *init_prefix,
					char *prefix, int *ok);

void find_shortest_word(trie_node_t *node);
void second_criteria(trie_node_t *node, char *init_prefix,
					 char *prefix, int *ok);

void get_max_freq(trie_node_t *node, int *max_freq);
void find_biggest_freq(trie_node_t *node, int max_freq,
					   int *is_ok_write, char *word);
void third_criteria(trie_node_t *node, char *init_prefix,
					char *prefix, int *ok);

void autocomplete(trie_t *trie, char *prefix, int criteria);

#endif /* _MK_H */
