//Chrys Adams

#include "btree.h"

#include <string.h>

int records = 0;

//#include <pthread.h>

//int rsh20(char str[]);
void node_init(struct node* n);
int isempty20(char str[]);

void btree_init(struct btree* t){
	t->root=(struct node*)malloc(sizeof(struct node));
	node_init(t->root);
}

int btree_inc(struct btree *tree, char hash []){
	
	char hash2[20];
	memcpy(hash2,hash,20);
	
	struct node* n=tree->root;
	
	while (!isempty20(hash)){
		int r = rsh20(hash);
		if (r==0){
			if(n->left==NULL){
				n->left=(struct node*)malloc(sizeof(struct node));
				node_init(n->left);
			}
			n=n->left;
		}else{
			if(n->right==NULL){
				n->right=(struct node*)malloc(sizeof(struct node));
				node_init(n->right);
			}
			n=n->right;
		}
	}
	n->spotted++;
	memcpy(n->hash,hash2,20);
	return n->spotted;
}
void btree_set(struct btree *tree, char hash [], int num){
	
	char hash2[20];
	memcpy(hash2,hash,20);
	
	struct node* n=tree->root;
	
	while (!isempty20(hash)){
		int r = rsh20(hash);
		if (r==0){
			if(n->left==NULL){
				n->left=(struct node*)malloc(sizeof(struct node));
				node_init(n->left);
			}
			n=n->left;
		}else{
			if(n->right==NULL){
				n->right=(struct node*)malloc(sizeof(struct node));
				node_init(n->right);
			}
			n=n->right;
		}
	}
	n->spotted=num;
	memcpy(n->hash,hash2,20);
	return;
}
int btree_get(struct btree *tree, char hash[]){
	
	struct node* n=tree->root;
	
	while (!isempty20(hash)){
		int r = rsh20(hash);
		if (r==0){
			if(n->left==NULL){
				return 0;
			}
			n=n->left;
		}else{
			if(n->right==NULL){
				return 0;
			}
			n=n->right;
		}
	}
	return n->spotted;
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
	//n->mutex=PTHREAD_MUTEX_INITIALIZER;
	n->spotted=0;
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








