#include "lexer.hpp"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>
#include <queue>
#include "interpreter.hh"

/// !! IF AND WHEN IT MOSTLY WORKS !!
/// !! DO. NOT. TOUCH. !!
/// !! THIS. CODE. IS. A. HELL. OF. A. MESS. !!
/// i wish and hope ANYONE doesn't have to read (and understand) this after me.

// current bugs with this:
// 1. modulo & logical not operatos fuck everything up (what a great choice to use operator also as a variable access, right?)

// its quite mad that this isnt implemented with a class, despite being in c++

// this parser also has a few pros:
//  1. doesnt unnecessairly loop when parsing numbers/multichar identifiers

/// happy reading :)

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
		if(ret == TOK_BUILTIN)
			return {.token = TOK_BUILTIN, .ptr = token, .size = end - start};
		free(token);
		return {.token = ret, .ptr = nullptr, .size = 0};
	}
	catch(...) { return {.token = TOK_UNDEFINED, .ptr = token, .size = end - start}; }
}

LexedFile lex(char *buffer, int fileSize) {
	int allocatedSize = 128*sizeof(Token), toksCreated= 0;
	Token *tokenbuffer = malloc(allocatedSize);

	#define PUTTOKEN(token, value)   temptoken = {token, value, 0}; tempUsed = true; currentToken = token; break
	#define PUTTOKENNB(token, value) tokenbuffer[toksCreated++] = (Token){token, value, 0};
	//#define logstatus printf("STAT;; %d %d %d %d %d %d %d %d %d\n", allocatedSize, toksCreated, numtok, lasttoken, currentToken, tempUsed, i, j, inString);
	#define logstatus
	#define IFLAST(x,y,z) (x == z && y != z)

	int numtok = 0, lasttoken = -1, i = 0, j = -1, currentToken = 0, lastusefultoken, prelastusefultoken = 0, parenLevel = 0;
	Token temptoken = {0, 0, 0};
	bool tempUsed = false, inString = false;

	// todo report num ops in this format
	// TOK_ADD TOK_NUMBER TOK_NUMBER
	// or
	// TOK_ADD TOK_NUMBER TOK_OPENING_PAREN TOK_MUL TOK_NUMBER TOK_NUMBER TOK_TOK_CLOSING_PAREN
	// aka + x (* y z)

	//std::queue<Token> q;

	while(i <= fileSize) {
		if(toksCreated > 127) tokenbuffer = realloc(tokenbuffer, allocatedSize += 128*sizeof(Token));

		switch(buffer[i]) {
		case ' ' :
		case '\t':
		case '\r': currentToken = TOK_WS_SEPARATOR; break;
		case '\n': PUTTOKEN(TOK_WS_SEPARATOR, 0);
		case '\0': currentToken = TOK_EOF; break;

		case '(': /*parenLevel++;*/ PUTTOKEN(TOK_OPENING_PAREN, 0);
		case ')': /*parenLevel--;*/ PUTTOKEN(TOK_CLOSING_PAREN, 0);
		case '+': PUTTOKEN(TOK_PLUS, 0);
		case '-': PUTTOKEN(TOK_MINUS, 0);
		case '*': PUTTOKEN(TOK_MULTIPLY, 0);
		case '/': PUTTOKEN(TOK_DIVIDE, 0);
		case '=': PUTTOKEN(TOK_EQUALS, 0);
		case '|': PUTTOKEN(TOK_PIPE, 0);
		case '&': PUTTOKEN(TOK_AND, 0);
		case '~': PUTTOKEN(TOK_TILDE, 0);
		case '^': PUTTOKEN(TOK_XOR, 0);
		case ',': PUTTOKEN(TOK_COMMA, 0);
		case '>': PUTTOKEN(TOK_GT, 0);
		case '<': PUTTOKEN(TOK_LT, 0);
		case ':': PUTTOKEN(TOK_COLON, 0);
		case '"': inString = !inString; PUTTOKEN(TOK_QUOTE, 0);
		case '%': PUTTOKEN(TOK_PERCENT, 0); /// todo handle env vars right in lexer
		case '!': PUTTOKEN(TOK_LOGICALNOT, 0);
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': if(j != -1) {currentToken = lasttoken; } else { numtok = numtok * 10 + (buffer[i] - '0'); currentToken = TOK_NUMBER; } break;
		default: if(j == -1) { j = i; } currentToken = (lasttoken == TOK_SWITCH)? TOK_SWITCH : TOK_UNDEFINED; break;
		}

		if(inString) { if(j == -1) j = i+1; currentToken = TOK_UNDEFINED; }
		if(currentToken == TOK_UNDEFINED && (lasttoken == TOK_DIVIDE || lasttoken == TOK_MINUS)) { currentToken = TOK_SWITCH; toksCreated -= 1; j = i; }

		if(currentToken == TOK_MINUS && lastusefultoken == TOK_UNDEFINED) { currentToken = TOK_SWITCH; j = i; tempUsed = false; }

		if(IFLAST(lasttoken,currentToken,TOK_UNDEFINED) || IFLAST(lasttoken,currentToken,TOK_SWITCH)) {
			auto processed = processMultichar(j, i, buffer);
			logstatus;
			if(processed.token != TOK_UNDEFINED && processed.token != TOK_BUILTIN) { PUTTOKENNB(processed.token, 0); }
			else { PUTTOKENNB(processed.token? processed.token : lasttoken, processed.ptr); tokenbuffer[toksCreated-1].additionalData = processed.size; }
			j = -1;
		}
		else if(IFLAST(lasttoken,currentToken,TOK_NUMBER) && !inString) {
			logstatus;
			// if(!parenLevel) { PUTTOKENNB(TOK_NUMBER, numtok); numtok = 0; }
			// else { q.push((Token){TOK_NUMBER, numtok, 0}); numtok = 0; };
			PUTTOKENNB(TOK_NUMBER, numtok); numtok = 0;
		}

		// todo write sorting algorithm for number expressions

		//if(currentToken == TOK_CLOSING_PAREN) {
		//	while(!q.empty()) { tokenbuffer[toksCreated++] = (Token){q.front().token, q.front().value, 0}; q.pop(); }
		//}

		if(tempUsed) {
			tokenbuffer[toksCreated++] = temptoken;
			logstatus;
			tempUsed = false;
		}

		lasttoken = currentToken;
		if(currentToken != TOK_WS_SEPARATOR) lastusefultoken = currentToken;

		//if(lasttoken >= TOK_PLUS && lasttoken <= TOK_AND && currentToken == TOK_NUMBER)
		//	std::swap(tokenbuffer[toksCreated-2], tokenbuffer[toksCreated-1]);

		i++;
	}

	#undef PUTTOKEN
	#undef PUTTOKENNB
	#undef logstatus

	// atleast make the leaks smaller
	//tokenbuffer = realloc(tokenbuffer, toksCreated*sizeof(Token));

	// for(int i = 0; i < toksCreated; i++) printf("%llu %llu %llu\n", tokenbuffer[i].token, tokenbuffer[i].value, tokenbuffer[i].additionalData);

	return {tokenbuffer, toksCreated};
}
