#ifndef _BTREE
#define _BTREE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "stack.h"

struct node{
	struct node* right;
	struct node* left;
	pthread_mutex_t *mutex;
	int spotted;
	char hash[20];
};

struct btree{
	struct node* root;
};


int rsh20(char str[]);
void btree_init(struct btree* t);
int btree_inc(struct btree *tree, char hash[]);
void btree_set(struct btree *tree, char hash [], int num);
int btree_get(struct btree *tree, char hash[]);
void btree_save(struct btree *tree, char filename[]);
void btree_load(struct btree *tree, char filename[]);

#endif