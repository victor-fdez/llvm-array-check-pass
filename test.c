
#include <stdlib.h>
#include <stdio.h>

#include "LibArrayCheck.h"

int main(int argc, char** argv)
{
	int b[20];
	int c[10];
	arrayAccess(10);
	int *d = (int*)malloc(sizeof(int)*10);
	arrayAccess(9);
	int *e = (int*)malloc(sizeof(int)*5);
	return *e + *d;
}
