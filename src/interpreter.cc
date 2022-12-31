#include "interpreter.hh"
#include <unistd.h>
#include <stack>

// this thing MUST be optimized into oblivion
// if we're losing time in lexer or parser, doesnt really matter
/// (well, if we're losing time in the evaluate() and such methods it does matter)
// they mostly run only once, on startup
// this, however, will run in a loop.
// this project's future depends on the speed (and also extensibility)

#define IFUN(name) int name(CallParams callParams)
#define fe(x,y,z) {hash(x), std::make_pair(y,z)},

std::unordered_map<size_t, long> labelMap;
std::stack<char*> directoryStack;

// todo finish implementation
IFUN(doSet) {
	Node *current;
	for(int i = 0; i < callParams.noParams; i++) {
		current = callParams.params[i];
		if((current->type & BARETYPE) == IN(INODE_ASSIGN))
			TCAST(AssignNode *, current)->assign();
	}

	return 0;
}

IFUN(doCall) {
	return 0;
}

IFUN(doCd) {
	return 0;
}

IFUN(doChdir) {
	return 0;
}

IFUN(doCls) {
	return 0;
}

IFUN(doCopy) {
	return 0;
}

IFUN(doCtty) {
	return 0;
}

IFUN(doDate) {
	return 0;
}

IFUN(doDel) {
	return 0;
}

IFUN(doDir) {
	return 0;
}

IFUN(doEcho) {
	return 0;
}

IFUN(doErase) {
	return 0;
}

IFUN(doGoto) {
	return 0;
}

IFUN(doHelp) {
	return 0;
}

IFUN(doLabel) {
	return 0;
}

IFUN(doMkdir) {
	return 0;
}

IFUN(doMove) {
	return 0;
}

IFUN(doPath) {
	return 0;
}

IFUN(doPause) {
	return 0;
}

IFUN(doPrompt) {
	return 0;
}

IFUN(doRename) {
	return 0;
}

IFUN(doRmdir) {
	return 0;
}

IFUN(doShift) {
	return 0;
}

IFUN(doStart) {
	return 0;
}

IFUN(doTime) {
	return 0;
}

IFUN(doTitle) {
	//SetConsoleTitleA(TCAST(StringNode *, callParams.params[0])->str);
	return 0;
}

IFUN(doType) {
	for(int i = 0; i < callParams.noParams; i++) {
		if((callParams.params[i]->type & BARETYPE) == LN(LNODE_ID) || (callParams.params[i]->type & BARETYPE) == LN(LNODE_STRING)) {
			FILE *f = fopen(TCAST(StringNode *, callParams.params[i])->str, "r");

			char *line = NULL;
			size_t len;
			ssize_t nread;
			while((nread = getline(&line, &len, f)) != -1) {
				fwrite(line, nread, 1, stdout);
			}

			free(line);
			fclose(f);
		}
	}

	return 0;
}

IFUN(doVerify) {
	return 0;
}

IFUN(doVer) {
	printf("cmd_new version %d dev alpha\n", 1);
	return 0;
}

IFUN(doVol) {
	return 0;
}

IFUN(doEndlocal) {
	return 0;
}

IFUN(doSetlocal) {
	return 0;
}

IFUN(doPushd) {
	char *dir = TCAST(StringNode *, callParams.params[0])->str;
	directoryStack.push(dir);
	chdir(dir);
	return 0;
}

IFUN(doPopd) {
	chdir(directoryStack.top());
	directoryStack.pop();
	return 0;
}

IFUN(doAssoc) {
	return 0;
}

IFUN(doColor) {
	return 0;
}

IFUN(doFtype) {
	return 0;
}

IFUN(doMore) {
	return 0;
}

IFUN(doChoice) {
	return 0;
}

IFUN(doMklink) {
	return 0;
}

IFUN(doExit) {
	return 0;
}

// functions that do.. nothing
IFUN(doRem) {
	return 0;
}

IFUN(doBreak) {
	return 0;
}


#ifdef _WIN64
extern IFUN(doInject);
#endif

std::unordered_map<size_t, std::pair<uint16_t, CallPtr>, Hasher> multicharMapping = {
	fe("do", TOK_DO, nullptr)
	fe("else", TOK_ELSE, nullptr)
	fe("in", TOK_IN, nullptr)

	fe("call", TOK_BUILTIN, doCall)
	fe("cd", TOK_BUILTIN, doCd)
	fe("chdir", TOK_BUILTIN, doChdir)
	fe("cls", TOK_BUILTIN, doCls)
	fe("copy", TOK_BUILTIN, doCopy)
	fe("ctty", TOK_BUILTIN, doCtty)
	fe("date", TOK_BUILTIN, doDate)
	fe("del", TOK_BUILTIN, doDel)
	fe("dir", TOK_BUILTIN, doDir)
	fe("echo", TOK_BUILTIN, doEcho)
	fe("erase", TOK_BUILTIN, doErase)
	fe("for", TOK_FOR, nullptr)
	fe("goto", TOK_BUILTIN, doGoto)
	fe("help", TOK_BUILTIN, doHelp)
	fe("if", TOK_IF, nullptr)
	fe("label", TOK_BUILTIN, doLabel)
	fe("md", TOK_BUILTIN, doMkdir)
	fe("mkdir", TOK_BUILTIN, doMkdir)
	fe("move", TOK_BUILTIN, doMove)
	fe("path", TOK_BUILTIN, doPath)
	fe("pause", TOK_BUILTIN, doPause)
	fe("prompt", TOK_BUILTIN, doPrompt)
	fe("rem", TOK_BUILTIN, doRem)
	fe("ren", TOK_BUILTIN, doRename)
	fe("rename", TOK_BUILTIN, doRename)
	fe("rd", TOK_BUILTIN, doRmdir)
	fe("rmdir", TOK_BUILTIN, doRmdir)
	fe("set", TOK_BUILTIN, doSet)
	fe("shift", TOK_BUILTIN, doShift)
	fe("start", TOK_BUILTIN, doStart)
	fe("time", TOK_BUILTIN, doTime)
	fe("title", TOK_BUILTIN, doTitle)
	fe("type", TOK_BUILTIN, doType)
	fe("verify", TOK_BUILTIN, doVerify)
	fe("ver", TOK_BUILTIN, doVer)
	fe("vol", TOK_BUILTIN, doVol)
	fe("endlocal", TOK_BUILTIN, doEndlocal)
	fe("setlocal", TOK_BUILTIN, doSetlocal)
	fe("pushd", TOK_BUILTIN, doPushd)
	fe("popd", TOK_BUILTIN, doPopd)
	fe("assoc", TOK_BUILTIN, doAssoc)
	fe("color", TOK_BUILTIN, doColor)
	fe("ftype", TOK_BUILTIN, doFtype)
	fe("more", TOK_BUILTIN, doMore)
	fe("choice", TOK_BUILTIN, doChoice)
	fe("mklink", TOK_BUILTIN, doMklink)
	fe("exit", TOK_BUILTIN, doExit)
	fe("break", TOK_BUILTIN, doBreak)

#ifdef _WIN64 // currently only Windows x64 supported, sorry
	fe("extend", TOK_BUILTIN, doInject)
#endif
};

void RegisterCommand(char *cmd, CallPtr func) {
	multicharMapping[hash(cmd)] = std::make_pair(TOK_BUILTIN, func);
}

Interpreter::Interpreter(char *buffer, size_t size)
 : parser(buffer, size)
{
	parser.parse();
}

extern char *itoa_(int i);

int Interpreter::interpret() {

	// loop over root nodes
	Node *current;
	for(long i = 0; i < this->nodes.size(); i++) {
		current = nodes[i];
		switch(current->type) {
		case LN(LNODE_CALL): {
			auto retcode = TCAST(CallNode*, current)->execute();
			setenv("errorlevel", itoa_(retcode), true);
		} break;
		}
	}

	return 0;
}
