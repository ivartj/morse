#include "getline.h"
#include <stdlib.h>

ssize_t int_getline(char **bufp, size_t *n, FILE *fp)
{
	ssize_t len;
	size_t cap;
	int c;
	char *buf;

	if(feof(fp))
		return -1;

	len = 0;
	buf = *bufp;
	if(buf != NULL)
		cap = *n;
	else
		cap = 0;

	while(1) {
		c = fgetc(fp);
		if(c == EOF)
			break;
		if(len == cap) {
			if(cap == 0)
				cap = 256;
			else
				cap *= 2;
			buf = (char *)realloc(buf, cap);
		}
		buf[len++] = c;
		if(c == '\n')
			break;
	}
	if(len == cap) {
		if(cap == 0)
			cap = 256;
		else
			cap *= 2;
		buf = realloc(buf, cap);
	}
	buf[len] = '\0';

	*n = cap;
	*bufp = buf;

	return len;
}
