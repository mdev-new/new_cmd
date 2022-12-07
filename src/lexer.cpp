#include "lexer.hpp"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <utility>

char *strndup(const char *str, size_t chars) {
    int n;

    char *buffer = (char *) malloc(chars +1);
    if (buffer) {
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

// todo upgrade gcc & consteval
constexpr uint64_t consthash(const char *text) {
  uint64_t h = 525201411107845655ull;
  for (int i = 0;text[i];++i) {
    h ^= text[i];
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

// todo use custom hash
//todo find a better way not dependant on stl
std::unordered_map<uint64_t, char> multicharMapping = {
	{consthash("for"), TOK_FOR},
	{consthash("do"), TOK_DO},
	{consthash("set"), TOK_SET},
	{consthash("dir"), TOK_DIR},
	{consthash("if"), TOK_IF},
};

struct processedMultichar {
	int token;
	char *ptr;
	int size;
};

processedMultichar processMultichar(int start, int end, char *buffer) {
	char *token = strndup(buffer+start, end - start);
	uint64_t hashed = hash(token);
	printf("CALL %d :: %s ; %llu \n", end - start, token, hashed);
	try {
		int ret = multicharMapping.at(hashed);
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
	#define logstatus printf("%d %d %d %d %d %d %d %d\n", allocatedSize, toksCreated, numtok, lasttoken, currentToken, tempUsed, i, j);

	int numtok = 0, lasttoken = -1, i = 0, j = -1, currentToken = 0;
	Token temptoken = {0, 0, 0};
	bool tempUsed = false;

	// todo for some reason we're lexing numerical ops as follows: TOK_PLUS TOK_NUMBER TOK_NUMBER
	// or with multiple +'s something like this: TOK_PLUS TOK_NUMBER TOK_PLUS TOK_NUMBER TOK_NUMBER
	// maybe fix? idk
	//defo fix
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
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9': numtok = numtok * 10 + (buffer[i] - '0'); currentToken = TOK_NUMBER; break;
		default: if(j == -1) { j = i; } currentToken = TOK_UNDEFINED; break;
		}

		if(lasttoken == TOK_UNDEFINED && currentToken != TOK_UNDEFINED) {
			auto processed = processMultichar(j, i, buffer);
			logstatus;
			if(processed.token != TOK_UNDEFINED) { PUTTOKENNB(processed.token, 0); }
			else { PUTTOKENNB(TOK_UNDEFINED, processed.ptr); tokenbuffer[toksCreated-1].additionalData = processed.size; }
			j = -1;
		}
		else if(lasttoken == TOK_NUMBER && currentToken != TOK_NUMBER) { PUTTOKENNB(TOK_NUMBER, numtok); numtok = 0; }
		lasttoken = currentToken;

		if(tempUsed) {
			tokenbuffer[toksCreated++] = {temptoken.token, temptoken.value, temptoken.additionalData};
			logstatus;
			tempUsed = false;
		}

		i++;
	}

	#undef PUTTOKEN
	#undef PUTTOKENNB

	return {tokenbuffer, toksCreated};
}
