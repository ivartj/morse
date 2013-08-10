#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <getopt.h>
#include "getline.h"

uint8_t codelen(uint8_t s);
uint8_t shortnote(uint8_t s);
uint8_t longnote(uint8_t s);
void decode(void);
void encode(void);
void printcode(uint8_t c);
void usage(FILE *out);

int reverse = 0;
char *infile = NULL;
FILE *in = NULL;
char *outfile = NULL;
FILE *out = NULL;

unsigned char morse[] = {
	[0xa0] = 'a', // 1010 0000
	[0x78] = 'b', // 0111 1000
	[0x58] = 'c', // 0101 1000
	[0x70] = 'd', // 0111 0000
	[0xc0] = 'e', // 1100 0000
	[0xd8] = 'f', // 1101 1000
	[0x30] = 'g', // 0011 0000
	[0xf8] = 'h', // 1111 1000
	[0xe0] = 'i', // 1110 0000
	[0x88] = 'j', // 1000 1000
	[0x50] = 'k', // 0101 0000
	[0xb8] = 'l', // 1011 1000
	[0x20] = 'm', // 0010 0000
	[0x60] = 'n', // 0110 0000
	[0x10] = 'o', // 0001 0000
	[0x98] = 'p', // 1001 1000
	[0x28] = 'q', // 0010 1000
	[0xb0] = 'r', // 1011 0000
	[0xf0] = 's', // 1111 0000
	[0x40] = 't', // 0100 0000
	[0xd0] = 'u', // 1101 0000
	[0xe8] = 'v', // 1110 1000
	[0x90] = 'w', // 1001 0000
	[0x68] = 'x', // 0110 1000
	[0x48] = 'y', // 0100 1000
	[0x38] = 'z', // 0011 1000

	[0x84] = '.', // 1000 0100
	[0xc4] = ',', // 1100 0100
	[0xe4] = '?', // 1110 0100
};

uint8_t codelen(uint8_t s)
{
	uint8_t i, hit;

	hit = 0;

	for(i = 0; i < 8; i++) {

		if((s << i) & 0xFF) {
			hit = i;
		}
	}


	return hit;
}

uint8_t shortnote(uint8_t s)
{
	uint8_t mask;
	uint8_t len;

	len = codelen(s);
	mask = 0xFF << (8 - len);
	s = s & mask;
	s |= 0xc0 >> len;
	return s;
}

uint8_t longnote(uint8_t s)
{
	uint8_t mask;
	uint8_t len;


	len = codelen(s);
	mask = 0xFF << (8 - len);
	s = s & mask;
	s |= 0x40 >> len;

	return s;
}

void decode(void)
{
	char *line = NULL;
	size_t linecap = 0;
	ssize_t len;
	int i;
	uint8_t s = 0x80;
	char c;

	while((len = int_getline(&line, &linecap, in)) > 0) {
		s = 0x80;
		for(i = 0; i < len; i++) {
			c = line[i];
			switch(c) {
			case '.':
				s = shortnote(s);
				break;
			case '-':
				s = longnote(s);
				break;
			case '/':
				fputc(' ', out);
				break;
			default:
				if(s != 0x80) {
					if(morse[s])
						fputc(morse[s], out);
					else
						fputc('?', out);
				}
				s = 0x80;
				break;
			}
		}
		fputc('\n', out);
	}

	free(line);
}

void encode(void)
{
	char *line = NULL;
	size_t linelen = 0;
	ssize_t len;
	int i, j;
	uint8_t c;

	while((len = int_getline(&line, &linelen, in)) > 0) {
		for(i = 0; i < len; i++) {
			c = line[i];
			if(c >= 'A' && c <= 'Z')
				c -= 'A' - 'a';
			switch(line[i]) {
			case ' ':
				fprintf(out, "/ ");
				break;
			default:
				for(j = 0; j < sizeof(morse); j++)
					if(morse[j] == c) {
						printcode(j);
						break;
					}
			}
		}

		fputc('\n', out);
	}

	free(line);
}

void printcode(uint8_t c)
{
	int i;
	int len;

	len = codelen(c);
	for(i = 0; i < len; i++) {
		if(0x80 & (c << i))
			fputc('.', out);
		else
			fputc('-', out);
	}
	fputc(' ', out);
}

void usage(FILE *out)
{
	fprintf(out, "usage: morse [-r]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "reverse", no_argument, NULL, 'r' },
		{ "out", required_argument, NULL, 'o' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "hro:", longopts, NULL)) != -1)
	switch(c) {
	case 'o':
		outfile = optarg;
		break;
	case 'r':
		reverse = 1;
		break;
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		break;
	case 1:
		infile = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void openfiles(void)
{
	if(infile != NULL) {
		in = fopen(infile, "r");
		if(in == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
	} else
		in = stdin;

	if(outfile != NULL) {
		out = fopen(outfile, "w");
		if(out == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
	} else
		out = stdout;
}

int main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openfiles();
	if(reverse)
		decode();
	else
		encode();

	exit(EXIT_SUCCESS);
}
