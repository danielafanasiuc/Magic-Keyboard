#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mk.h"

trie_node_t *trie_create_node(void)
{
	trie_node_t *node = (trie_node_t *)malloc(sizeof(trie_node_t));
	DIE(!node, "Cannot allocate trie node!");

	node->frequency = 0;
	node->shortest_path = 0x7FFFFFFF;
	node->end_of_word = 0;
	node->n_children = 0;

	node->children = malloc(sizeof(trie_node_t *) * ALPHABET_SIZE);
	DIE(!node->children, "Cannot allocate node's children!");

	for (int i = 0; i < ALPHABET_SIZE; ++i)
		node->children[i] = NULL;

	return node;
}

trie_t *trie_create(int alphabet_size, char *alphabet)
{
	trie_t *trie = (trie_t *)malloc(sizeof(trie_t));
	DIE(!trie, "Cannot allocate trie!");

	trie->root = NULL;
	trie->alphabet_size = alphabet_size;
	trie->alphabet = alphabet;

	return trie;
}

void __trie_insert(trie_node_t *node, char *key)
{
	// if must insert the value
	if (strlen(key) == 0) {
		node->end_of_word = 1;
		node->shortest_path = 0;
		node->frequency++;

		return;
	}

	int remaped_letter = key[0] - 'a';
	// update the shortest path if needed
	if (node->shortest_path > (int)strlen(key))
		node->shortest_path = strlen(key);

	if (!node->children[remaped_letter]) {
		node->children[remaped_letter] = trie_create_node();
		node->n_children++;
	}

	__trie_insert(node->children[remaped_letter], key + 1);
}

void trie_insert(trie_t *trie, char *key)
{
	if (!trie || strlen(key) == 0)
		return;

	if (!trie->root)
		trie->root = trie_create_node();

	__trie_insert(trie->root, key);
}

int get_minimum_child(trie_node_t *node)
{
	int minimum = 0x7FFFFFFF;
	for (int i = 0; i < ALPHABET_SIZE; ++i) {
		if (node->children[i])
			if (node->children[i]->shortest_path < minimum)
				minimum = node->children[i]->shortest_path;
	}
	return minimum;
}

int __trie_remove(trie_node_t *node, char *key)
{
	if (strlen(key) == 0) {
		node->frequency = 0;

		if (node->end_of_word == 1) {
			node->end_of_word = 0;
			return (node->n_children == 0) ? 1 : 0;
		}
		return 0;
	}

	int remaped_letter = key[0] - 'a';

	if (node->children[remaped_letter] &&
		__trie_remove(node->children[remaped_letter], key + 1)) {
		free(node->children[remaped_letter]->children);
		free(node->children[remaped_letter]);
		node->children[remaped_letter] = NULL;

		node->n_children--;

		// if end of word, shortest path is 0
		// otherwise, get the shortest path of all children and increment
		if (node->end_of_word == 1)
			node->shortest_path = 0;
		else
			node->shortest_path = get_minimum_child(node) + 1;

		if (node->n_children == 0 && node->end_of_word == 0)
			return 1;

		return 0;
	}

	// if end of word, shortest path is 0
	// otherwise, get the shortest path of all children and increment
	if (node->end_of_word == 1)
		node->shortest_path = 0;
	else
		node->shortest_path = get_minimum_child(node) + 1;

	return 0;
}

void trie_remove(trie_t *trie, char *key)
{
	if (!trie || strlen(key) == 0)
		return;

	__trie_remove(trie->root, key);
}

void __trie_free(trie_node_t *node)
{
	for (int i = 0; i < ALPHABET_SIZE; ++i) {
		if (node->children[i]) {
			__trie_free(node->children[i]);
			node->children[i] = NULL;
		}
	}

	free(node->children);
	free(node);
}

void trie_free(trie_t *trie)
{
	__trie_free(trie->root);
	free(trie);
}

void load_file(char *filename, trie_t *trie)
{
	FILE *input_file = fopen(filename, "rt");
	DIE(!input_file, "Cannot open file!");

	char key[MAX_KEY];
	while (fscanf(input_file, "%s", key) == 1)
		trie_insert(trie, key);

	fclose(input_file);
}

void __autocorrect(trie_node_t *node, char *key, char *similar_key, int k,
				   int offset, int *ok)
{
	// if k < 0, break the recursivity
	if (k < 0)
		return;

	// break recursivity when size of key is 0
	if (strlen(key) == 0) {
		if (node->end_of_word == 1) {
			similar_key[0] = '\0';
			// if a similar key is found, print it
			printf("%s\n", similar_key - offset);
			*ok = 1;
		}
		return;
	}

	int remaped_letter = key[0] - 'a';

	for (int i = 0; i < ALPHABET_SIZE; ++i) {
		if (node->children[i]) {
			similar_key[0] = (char)i + 'a';
			if (i != remaped_letter)
				__autocorrect(node->children[i], key + 1, similar_key + 1,
							  k - 1, offset + 1, ok);
			else
				__autocorrect(node->children[i], key + 1, similar_key + 1,
							  k, offset + 1, ok);
		}
	}
}

void autocorrect(trie_t *trie, char *key, int k)
{
	if (!trie || !trie->root || strlen(key) == 0)
		return;

	char *similar_key = calloc((strlen(key) + 1), sizeof(char));
	DIE(!similar_key, "Cannot allocate similar key!");

	int ok = 0;

	__autocorrect(trie->root, key, similar_key, k, 0, &ok);

	if (ok == 0)
		printf("No words found\n");

	free(similar_key);
}

void find_smallest_word(trie_node_t *node)
{
	if (node->end_of_word == 1) {
		printf("\n");
		return;
	}

	for (int i = 0; i < ALPHABET_SIZE; ++i) {
		if (node->children[i]) {
			printf("%c", (char)i + 'a');
			find_smallest_word(node->children[i]);
			break;
		}
	}
}

void first_criteria(trie_node_t *node, char *init_prefix,
					char *prefix, int *ok)
{
	if (strlen(prefix) == 0) {
		printf("%s", init_prefix);
		find_smallest_word(node);
	} else {
		int remaped_letter = prefix[0] - 'a';

		if (!node->children[remaped_letter]) {
			*ok = 1;
			return;
		}

		first_criteria(node->children[remaped_letter],
					   init_prefix, prefix + 1, ok);
	}
}

void find_shortest_word(trie_node_t *node)
{
	if (node->end_of_word == 1) {
		printf("\n");
		return;
	}

	// get the minimum on each level
	for (int i = 0; i < ALPHABET_SIZE; ++i) {
		if (node->children[i] &&
			node->children[i]->shortest_path == node->shortest_path - 1) {
			printf("%c", (char)i + 'a');
			find_shortest_word(node->children[i]);
			break;
		}
	}
}

void second_criteria(trie_node_t *node, char *init_prefix,
					 char *prefix, int *ok)
{
	if (strlen(prefix) == 0) {
		printf("%s", init_prefix);
		find_shortest_word(node);
	} else {
		int remaped_letter = prefix[0] - 'a';

		if (!node->children[remaped_letter]) {
			*ok = 1;
			return;
		}

		second_criteria(node->children[remaped_letter], init_prefix,
						prefix + 1, ok);
	}
}

void get_max_freq(trie_node_t *node, int *max_freq)
{
	if (node->end_of_word == 1) {
		if (node->frequency > *max_freq)
			*max_freq = node->frequency;
	}

	for (int i = 0; i < ALPHABET_SIZE; ++i)
		if (node->children[i])
			get_max_freq(node->children[i], max_freq);
}

void find_biggest_freq(trie_node_t *node, int max_freq,
					   int *is_ok_write, char *word)
{
	if (node->end_of_word == 1) {
		word[0] = '\0';
		// when first word found with the biggest frequency
		// set is_ok_write to 0 to finish the search
		if (node->frequency == max_freq)
			*is_ok_write = 0;
	}

	for (int i = 0; i < ALPHABET_SIZE; ++i) {
		// if is_ok_write is set on 1, don't search anymore
		if (node->children[i] && *is_ok_write == 1) {
			word[0] = (char)i + 'a';
			find_biggest_freq(node->children[i], max_freq,
							  is_ok_write, word + 1);
		}
	}
}

void third_criteria(trie_node_t *node, char *init_prefix,
					char *prefix, int *ok)
{
	if (strlen(prefix) == 0) {
		printf("%s", init_prefix);
		int is_ok_write = 1;
		char word[MAX_KEY];
		int max_freq = 0;
		// get the maximum frequency of a word
		get_max_freq(node, &max_freq);
		// find first word with the biggest frequency
		find_biggest_freq(node, max_freq, &is_ok_write, word);
		printf("%s\n", word);
	} else {
		int remaped_letter = prefix[0] - 'a';

		if (!node->children[remaped_letter]) {
			*ok = 1;
			return;
		}

		third_criteria(node->children[remaped_letter],
					   init_prefix, prefix + 1, ok);
	}
}

void autocomplete(trie_t *trie, char *prefix, int criteria)
{
	if (!trie || !trie->root || strlen(prefix) == 0)
		return;

	if (criteria == 1 || criteria == 0) {
		int ok = 0;
		first_criteria(trie->root, prefix, prefix, &ok);
		if (ok == 1)
			printf("No words found\n");
	}

	if (criteria == 2 || criteria == 0) {
		int ok = 0;
		second_criteria(trie->root, prefix, prefix, &ok);
		if (ok == 1)
			printf("No words found\n");
	}

	if (criteria == 3 || criteria == 0) {
		int ok = 0;
		third_criteria(trie->root, prefix, prefix, &ok);
		if (ok == 1)
			printf("No words found\n");
	}
}

int main(void)
{
	char alphabet[] = ALPHABET;
	trie_t *trie = trie_create(ALPHABET_SIZE, alphabet);

	while (1) {
		char operation[MAX_OPERATION];
		scanf("%s", operation);

		if (strcmp("INSERT", operation) == 0) {
			char key[MAX_KEY];
			scanf("%s", key);
			trie_insert(trie, key);
		} else if (strcmp("LOAD", operation) == 0) {
			char filename[MAX_FILENAME];
			scanf("%s", filename);
			load_file(filename, trie);
		} else if (strcmp("REMOVE", operation) == 0) {
			char key[MAX_KEY];
			scanf("%s", key);
			trie_remove(trie, key);
		} else if (strcmp("AUTOCORRECT", operation) == 0) {
			char key[MAX_KEY];
			scanf("%s", key);
			int k;
			scanf("%d", &k);
			autocorrect(trie, key, k);
		} else if (strcmp("AUTOCOMPLETE", operation) == 0) {
			char prefix[MAX_KEY];
			scanf("%s", prefix);
			int criteria;
			scanf("%d", &criteria);
			autocomplete(trie, prefix, criteria);
		} else if (strcmp("EXIT", operation) == 0) {
			break;
		}
	}
	// free everything
	trie_free(trie);

	return 0;
}
