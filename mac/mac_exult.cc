#include <stdlib.h>
#include <string.h>


char*	strdup( const char *x )
{
	char	*ret = (char*)malloc(strlen(x)+1);
	
	strcpy(ret, x);
	
	return ret;
}
