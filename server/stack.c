//Chrys Adams
#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

void stack_init(struct stack *s, int size){
	s->size=size;
	s->index=0;
	s->stack=malloc(size*sizeof(void*));
}
void stack_push(struct stack *s, void *e){
	if(s->index>s->size){
		printf("Stack overflow!\n");
		return;
	}
	s->stack[s->index]=e;
	s->index++;
	return;
}
void *stack_pop(struct stack *s){
	if(s->size==0){
		return 0;
	}
	s->index--;
	return s->stack[s->index];
}