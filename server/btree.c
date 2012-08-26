//Chrys Adams

#include "btree.h"

#include <string.h>

int records = 0;

void node_init(struct node* n);
int isempty20(char str[]);

void btree_init(struct btree* t){
	t->root=(struct node*)malloc(sizeof(struct node));
	node_init(t->root);
}

int btree_inc(struct btree *tree, char hash []){
	pthread_mutex_t *lock;
	char hash2[20];
	memcpy(hash2,hash,20);
	
	struct node* n=tree->root;
	pthread_mutex_lock(n->mutex);
	
	while (!isempty20(hash)){
		int r = rsh20(hash);
		
		if (r==0){
			if(n->left==NULL){
				n->left=(struct node*)malloc(sizeof(struct node));
				node_init(n->left);
			}
			lock=n->mutex;
			n=n->left;
			pthread_mutex_lock(n->mutex);
			pthread_mutex_unlock(lock);
		}else{
			if(n->right==NULL){
				n->right=(struct node*)malloc(sizeof(struct node));
				node_init(n->right);
			}
			lock=n->mutex;
			n=n->right;
			pthread_mutex_lock(n->mutex);
			pthread_mutex_unlock(lock);
		}
	}
	n->spotted++;
	memcpy(n->hash,hash2,20);
	pthread_mutex_unlock(n->mutex);
	int result = n->spotted;
	return result;
}
void btree_set(struct btree *tree, char hash [], int num){
	pthread_mutex_t *lock;
	char hash2[20];
	memcpy(hash2,hash,20);
	
	struct node* n=tree->root;
	pthread_mutex_lock(n->mutex);
	
	while (!isempty20(hash)){
		int r = rsh20(hash);
		
		if (r==0){
			if(n->left==NULL){
				n->left=(struct node*)malloc(sizeof(struct node));
				node_init(n->left);
			}
			lock=n->mutex;
			n=n->left;
			pthread_mutex_lock(n->mutex);
			pthread_mutex_unlock(lock);
		}else{
			if(n->right==NULL){
				n->right=(struct node*)malloc(sizeof(struct node));
				node_init(n->right);
			}
			lock=n->mutex;
			n=n->right;
			pthread_mutex_lock(n->mutex);
			pthread_mutex_unlock(lock);
		}
	}
	n->spotted=num;
	memcpy(n->hash,hash2,20);
	pthread_mutex_unlock(n->mutex);
	return;
}
int btree_get(struct btree *tree, char hash[]){
	pthread_mutex_t *lock;
	struct node* n=tree->root;
	pthread_mutex_lock(n->mutex);
	while (!isempty20(hash)){
		int r = rsh20(hash);
		if (r==0){
			if(n->left==NULL){
				pthread_mutex_unlock(n->mutex);
				return 0;
			}
			lock=n->mutex;
			n=n->left;
			pthread_mutex_lock(n->mutex);
			pthread_mutex_unlock(lock);
		}else{
			if(n->right==NULL){
				pthread_mutex_unlock(n->mutex);
				return 0;
			}
			lock=n->mutex;
			n=n->right;
			pthread_mutex_lock(n->mutex);
			pthread_mutex_unlock(lock);
		}
	}
	int result = n->spotted;
	pthread_mutex_unlock(n->mutex);
	return result;
}
int isempty20(char str[]){
	int i;
	for(i=0;i<20;i++){
		if(str[i]!=0)
			return 0;
	}
	return 1;
}
int rsh20(char str[]){
	unsigned char *o;
	o = (unsigned char *) str;
	int result=o[19]%2;
	int i;
	for(i=19;i>0;i--){
		o[i]=o[i]>>1;
		o[i]=o[i] + ((o[i-1]%2)*128);
	}
	o[0]=o[0]>>1;
	return result;
}

void node_init(struct node* n){
	n->right=NULL;
	n->left=NULL;
	n->mutex=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(n->mutex,NULL);
	n->spotted=0;
	return;
}
void node_free(struct node* n){
	pthread_mutex_destroy(n->mutex);
	free(n->mutex);
	return;
}
void node_print(struct node* n){
	printf("Node Address: %ld\n",(long)n);
	printf("Node spotted: %d\n",n->spotted);
	printf("Node left: %ld\n",(long)n->left);
	printf("Node right: %ld\n",(long)n->right);
	return;
}
void btree_save(struct btree *tree, char filename[]){
	FILE *file = fopen(filename,"wb");
	struct stack s;
	struct node *n;
	stack_init(&s,160);
	stack_push(&s, tree->root);
	int i=0;
	while(s.index!=0){
		n=(struct node*) stack_pop(&s);
		if(n->spotted>0){
			fwrite(&(n->spotted),4,1,file);
			fwrite(&(n->hash),20,1,file);
			i++;
		}
		if(n->right!=NULL){
			stack_push(&s,n->right);
		}
		if(n->left!=NULL){
			stack_push(&s,n->left);
		}
	}
	printf("records written: %d\n",i);
	fflush(file);
	fclose(file);
	return;
}
void btree_load(struct btree *tree, char filename[]){
	FILE *file = fopen(filename,"rb");
	char buffer[24];
	int i=0;
	if(file==NULL){
		return;
	}
	while(fread(buffer,1,24,file)==24){
		btree_set(tree,&(buffer[4]),(*(int*)(buffer)));
		i++;
	}
		printf("records read: %d\n",i);
	fclose(file);
	return;
}
void btree_free(struct btree *tree){
	struct stack s;
	struct node *n;
	stack_init(&s,160);
	stack_push(&s, tree->root);
	while(s.index!=0){
		n=(struct node*) stack_pop(&s);
		if(n->right!=NULL){
			stack_push(&s,n->right);
		}
		if(n->left!=NULL){
			stack_push(&s,n->left);
		}
		node_free(n);
		free(n);
	}
	stack_free(&s);
	return;
}







