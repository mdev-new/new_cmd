#include "lexer.hpp"

LexedFile lex(HANDLE hFile) {
	if(hFile == NULL) return {nullptr, 0};

	int allocatedSize = 128*sizeof(Token), fileSize = GetFileSize(hFile, nullptr), toksCreated= 0, bytesRead = 0;
	Token *tokenbuffer = malloc(allocatedSize);
	char *buffer = malloc(fileSize + 1);
	ReadFile(hFile, buffer, fileSize, &bytesRead, NULL);

	#define PUTTOKEN(token, value) tokenbuffer[toksCreated++] = {token, value}; break

	for(int i = 0; i < bytesRead; i++) {
		if(toksCreated > 128) tokenbuffer = realloc(tokenbuffer, allocatedSize += 128*sizeof(Token));
		switch(buffer[i]) {
			case ' ': PUTTOKEN(TOK_SPACE, 0);
			case '\t': break;
			case '\n': break;
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
			case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': PUTTOKEN(TOK_NUMBER, buffer[i]);
			default: PUTTOKEN(TOK_UNDEFINED, buffer[i]);
		}
	}

	#undef PUTTOKEN
	return {tokenbuffer, toksCreated};
}