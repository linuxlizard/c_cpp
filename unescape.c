// strip all ANSI/VT100/VT220/&etc terminal control characters from a file.
// Originally created to clean up a tmux logfile.
//
// TODO beware UTF8
//
// David Poole 20220203 davep@mbuf.com
//
// https://espterm.github.io/docs/VT100%20escape%20codes.html

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define NUL 0x00
#define BS 0x08
#define LF 0x0a
#define CR 0x0d
#define ESC 0x1b
#define DEL 0x7f

#define MAX_LINE 1024

typedef enum {
	STATE_INIT = 1,
	STATE_LEFT_BRACKET,
	STATE_HASH,
	STATE_NUMBER,
	STATE_QUESTION,
	STATE_PAREN,
	STATE_EAT_1,
	STATE_DONE
} parse_state;

int eat_escape_sequence(FILE *infile)
{
	int n;
	parse_state state = STATE_INIT;

	while(state != STATE_DONE) {
		n = fgetc(infile);
		if (n == EOF) {
			return EOF;
		}

		switch (state) {
			case STATE_INIT:
				// three or more character escape sequences
				if (n=='[') {
					state = STATE_LEFT_BRACKET;
				}
				else if (n=='#') {
					state = STATE_HASH;
				}
				else {
					// two character escape sequence; eat the next char
					state = STATE_DONE;
				}
				break;

			case STATE_LEFT_BRACKET:
				if (strchr("1234567890", n)) {
					state = STATE_NUMBER;
				}
				else if ( n == '?') {
					state = STATE_QUESTION;
				}
				else if ( n == '(' || n == ')' ) {
					state = STATE_PAREN;
				}
				else {
					// otherwise we have a 3-character sequence so just eat the next char and be done
					state = STATE_DONE;
				}
				break;

			case STATE_QUESTION:
				if (strchr("1234567890", n)) {
					state = STATE_NUMBER;
				}
				else {
					state = STATE_EAT_1;
				}
				break;

			case STATE_EAT_1:
			case STATE_HASH:
			case STATE_PAREN:
				state = STATE_DONE;
				break;

			case STATE_NUMBER:
				if (strchr("1234567890", n)) {
					state = STATE_NUMBER;
				}
				else if (n == ';') {
					state = STATE_NUMBER;
				}
				else {
					state = STATE_DONE;
				}
				break;

			case STATE_DONE:
				break;

			default:
				assert(0);
		}
	}

	return n;
}

// +1 for terminating NULL to make a C printable string
static int line[MAX_LINE+1];
static int end=0;

static bool push(int n)
{
	line[end++] = n;
	if ( (end+1) >= MAX_LINE) {
		// buffer full
		return true;
	}
	return false;
}

static void reset(void)
{
	end = 0;
}

static void dump(FILE *stream)
{
	for (int i=0 ; i<end ; i++) {
		fputc(line[i], stream);
	}
}

static void backspace(void)
{
	if (end > 0) {
		end --;
	}
}

int main(void)
{
//	FILE *infile = fopen("/home/dpoole/tmp/fragattack.log", "r");
	FILE *infile = stdin;
	FILE *outfile = stdout;
	int n;

	while(1) {
		n = fgetc(infile);
		if (n == EOF) {
			break;
		}
		if (n == ESC) {
			n = eat_escape_sequence(infile);
			if (n == EOF) {
				break;
			}
		}
		else if (n == CR) {
			// eat successive CRCRCR
		}
		else if (n == LF) {
			push('\n');
		}
		else if (n == BS) {
			backspace();
		}
		else {
			if (push(n)) {
				dump(outfile);
				reset();
			}
		}
	}
	dump(outfile);
	return EXIT_SUCCESS;
}

