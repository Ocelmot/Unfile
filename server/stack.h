#ifndef _STACK
#define _STACK

struct stack{
	int size;
	int index;
	void **stack;
};

void stack_init(struct stack *s, int size);
void stack_push(struct stack *s, void *e);
void *stack_pop(struct stack *s);

#endif