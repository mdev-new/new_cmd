#include "lexer.hpp"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>
#include "interpreter.hh"

/// !! IF AND WHEN IT MOSTLY WORKS !!
/// !! DO. NOT. TOUCH. !!

// current bugs with this:
// 1. modulo & logical not operatos fuck everything up (what a great choice to use operator also as a variable access, right?)

// its quite mad that this isnt implemented with a class, despite being in c++

char *strndup(const char *str, size_t chars) {
    int n;

    char *buffer = (char *) malloc(chars +1);
    if (buffer) {
        for (n = 0; ((n < chars) && (str[n] != 0)) ; n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

struct processedMultichar {
	int token;
	char *ptr;
	int size;
};

// todo probably move to parser
processedMultichar processMultichar(int start, int end, char *buffer) {
	char *token = strndup(buffer+start, end - start);
	uint64_t hashed = hash(token);
	//printf("CALL %d :: %s ; %llu \n", end - start, token, hashed);
	try {
		int ret = multicharMapping.at(hashed).first;
		if(ret == TOK_BUILTIN) return {.token = TOK_BUILTIN, .ptr = token, .size = end - start};
		free(token);
		return {.token = ret, .ptr = nullptr, .size = 0};
	}
	catch(...) { return {.token = TOK_UNDEFINED, .ptr = token, .size = end - start}; }
}

LexedFile lex(char *buffer, int fileSize) {
	int allocatedSize = 128*sizeof(Token), toksCreated= 0;
	Token *tokenbuffer = malloc(allocatedSize);

	#define PUTTOKEN(token, value)   temptoken = {token, value, 0}; tempUsed = true; currentToken = token; break
	#define PUTTOKENNB(token, value) tokenbuffer[toksCreated++] = (Token){token, value, 0}
	//#define logstatus printf("STAT;; %d %d %d %d %d %d %d %d %d\n", allocatedSize, toksCreated, numtok, lasttoken, currentToken, tempUsed, i, j, inString);
	#define logstatus
	#define IFLAST(x,y,z) (x == z && y != z)

	int numtok = 0, lasttoken = -1, i = 0, j = -1, currentToken = 0;
	Token temptoken = {0, 0, 0};
	bool tempUsed = false, inString = false;

	while(i < fileSize) {
		if(toksCreated > 127) tokenbuffer = realloc(tokenbuffer, allocatedSize += 128*sizeof(Token));

		switch(buffer[i]) {
		case ' ' :
		case '\t':
		case '\n': currentToken = TOK_SPACE; break;

		case '(':
		case ')':
		case '+':
		case '-':
		case '*':
		case '/':
		case '=':
		case '|':
		case '&':
		case '~':
		case '^':
		case ',':
		case '>':
		case '<':
		case ':': PUTTOKEN(buffer[i], 0);
		case '"': inString = !inString; PUTTOKEN(TOK_QUOTE, 0);
		case '%': inString = !inString; PUTTOKEN(TOK_PERCENT, 0);
		case '!': inString = !inString; PUTTOKEN(TOK_LOGICALNOT, 0);
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': if(j != -1) {currentToken = TOK_UNDEFINED; } else { numtok = numtok * 10 + (buffer[i] - '0'); currentToken = TOK_NUMBER; } break;
		default: if(j == -1) { j = i; } currentToken = (lasttoken == TOK_SWITCH)? TOK_SWITCH : TOK_UNDEFINED; break;
		}

		if(inString) { if(j == -1) j = i+1; currentToken = TOK_UNDEFINED; }
		if(currentToken == TOK_UNDEFINED && lasttoken == TOK_DIVIDE) { currentToken = TOK_SWITCH; toksCreated -= 1; j = i-1; }

		if(IFLAST(lasttoken,currentToken,TOK_UNDEFINED) || IFLAST(lasttoken,currentToken,TOK_SWITCH)) {
			auto processed = processMultichar(j, i, buffer);
			logstatus;
			if(processed.token != TOK_UNDEFINED && processed.token != TOK_BUILTIN) { PUTTOKENNB(processed.token, 0); }
			else { PUTTOKENNB(processed.token? processed.token : lasttoken, processed.ptr); tokenbuffer[toksCreated-1].additionalData = processed.size; }
			j = -1;
		}
		else if(IFLAST(lasttoken,currentToken,TOK_NUMBER) && !inString) { logstatus; PUTTOKENNB(TOK_NUMBER, numtok); numtok = 0; }
		lasttoken = currentToken;

		if(tempUsed) {
			tokenbuffer[toksCreated++] = temptoken;
			logstatus;
			tempUsed = false;
		}

		i++;
	}

	#undef PUTTOKEN
	#undef PUTTOKENNB
	#undef logstatus

	return {tokenbuffer, toksCreated};
}
