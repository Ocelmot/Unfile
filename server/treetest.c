//Chrys Adams
#include "btree.h"
#include <stdio.h>
#include <string.h>



int main(int argc, char* argv[]){

	char a[]="\0\0\0\0\0\0\0\0\0\0\0\0\0a\0\0\0\0\0b";
	int i;
	for(i=0;i<161;i++){
		printf("%i", rsh20(a));
	}
	printf("\n");

	
	char b1[]="asdfghjkl;asdfghjkl;";
	char c1[]="qweqweqweqweqweqweqw";
	char d1[]="somehashlalalalalala";
	
	
	char b[]="asdfghjkl;asdfghjkl;";
	char c[]="asdfghjkl;asdfghjkl;";
	char d[]="qweqweqweqweqweqweqw";
	char e[]="asdfghjkl;asdfghjkl;";
	char f[]="somehashlalalalalala";
	
	
	


	struct btree t;
	printf("init\n");
	btree_init(&t);
	
	printf("start\n");

	increment(&t, b);
	increment(&t, c);
	increment(&t, d);
	increment(&t, e);
	increment(&t, f);
	
	printf("read\n");
	
	printf("%i\n", read(&t, b1));
	printf("%i\n", read(&t, c1));
	printf("%i\n", read(&t, d1));
	return 0;
}