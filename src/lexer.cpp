#include "lexer.hpp"
#include <cstdlib>
#include <cstdio>

char *strndup(const char *str, size_t chars) {
    int n;

    char *buffer = (char *) malloc(chars +1);
    if (buffer)
    {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

uint64_t hash ( const char * text) {
  uint64_t h = 525201411107845655ull;
  for (;*text;++text) {
    h ^= *text;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

// process non single char tokens ("do", "for", etc)
// basically inherit the context
int processMultichar(char *end, char *start, int *toksCreated, Token *tokenbuffer, char *buffer) {
	char *token = strndup(&buffer[start - buffer], end - start);
	uint64_t hashed = hash(token);
	printf("CALL %d %d :: %s ; %ull\n", end - buffer, *toksCreated, token, hashed);
	int t = hashExtendedToken(token);
	free(token);
	return t;
}

LexedFile lex(char *buffer, int fileSize) {
	int allocatedSize = 128*sizeof(Token), toksCreated= 0, bytesRead = 0;
	Token *tokenbuffer = malloc(allocatedSize);

	// todo whacky
	#define PUTTOKEN(token, value) currentToken = token; tokenbuffer[toksCreated++] = {token, value}; break

	int numtok = 0, lasttoken = -1, i = 0, j = -1, currentToken = 0;

	while(i < fileSize) {
		if(toksCreated >= 128) tokenbuffer = realloc(tokenbuffer, allocatedSize += 128*sizeof(Token));

		switch(buffer[i]) {
		case ' ': PUTTOKEN(TOK_SPACE, 0);
		case '\t': PUTTOKEN(TOK_SPACE, 0);
		case '\n': PUTTOKEN(TOK_SPACE, 0);

		case '(': PUTTOKEN(TOK_OPENING_BRACKET, 0);
		case ')': PUTTOKEN(TOK_CLOSING_BRACKET, 0);
		case '+': PUTTOKEN(TOK_PLUS, 0);
		case '-': PUTTOKEN(TOK_MINUS, 0);
		case '*': PUTTOKEN(TOK_MULTIPLY, 0);
		case '/': PUTTOKEN(TOK_DIVIDE, 0);
		case '=': PUTTOKEN(TOK_EQUALS, 0);
		case '|': PUTTOKEN(TOK_PIPE, 0);
		case '&': PUTTOKEN(TOK_AND, 0);
		case '%': PUTTOKEN(TOK_PERCENT, 0);
		case '!': PUTTOKEN(TOK_LOGICALNOT, 0);
		case '~': PUTTOKEN(TOK_TILDE, 0);
		case '^': PUTTOKEN(TOK_XOR, 0);
		case ',': PUTTOKEN(TOK_COMMA, 0);
		case '>': PUTTOKEN(TOK_GT, 0);
		case '<': PUTTOKEN(TOK_LT, 0);
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': numtok = numtok * 10 + (buffer[i] - '0'); currentToken = TOK_NUMBER; break;
		default: if(j == -1) { j = i; printf("%d ", i); } currentToken = TOK_UNDEFINED; break;
		}

		if(lasttoken == TOK_UNDEFINED && currentToken != TOK_UNDEFINED) { PUTTOKEN(processMultichar(&buffer[i], &buffer[j], &toksCreated, tokenbuffer, buffer), 0); j = -1; }
		else if(lasttoken == TOK_NUMBER && currentToken != TOK_NUMBER) numtok = 0;
		lasttoken = currentToken;

		i++;
	}

	#undef PUTTOKEN

	return {tokenbuffer, toksCreated};
}
