#include "interpreter.hh"
#include "shared.hh"
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <climits>
#include "win.hh"

// optimize as much as possible

// todo use windows functions on win and linux functions on linux
// do not rely as much on stdlib

#define IFUN(name) int name(CallParams callParams)
#define fe(x,y,z) {_hashfunc_(x), std::make_pair(y,z)},

[[gnu::noinline]] constexpr uint64_t hash64(const char *text) {
	uint64_t h = 525201411107845655ull, i = 0;
	while (text[i++]) {
		h = (h ^ text[i]) * 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

[[gnu::noinline]] constexpr uint32_t hash32(const char* data) {
	uint32_t h = 3323198485ul, i = 0;
	char c = 0;
	while (c = data[i++]) {
		h ^= c * 0x5bd1e995;
		h ^= h >> 15;
	}
	return h;
}

// -- HIGH PRIORITY --
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

// only absolute paths atm, sorry
IFUN(doChdir) {
	chdir(TCAST(StringNode *, callParams.params[0])->str);
	return 0;
}

IFUN(doCls) {
#ifdef _WIN64
	CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenInfo;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(handle, &ConsoleScreenInfo);

	SMALL_RECT ScrollRect = {
		.Left = 0,
		.Top = 0,
		.Right =  ConsoleScreenInfo.dwSize.X,
		.Bottom = ConsoleScreenInfo.dwSize.Y
	};

	CHAR_INFO chinfo;
	chinfo.Char.AsciiChar = ' ';
	chinfo.Attributes = ConsoleScreenInfo.wAttributes;

	ScrollConsoleScreenBuffer(handle, &ScrollRect, NULL, { 0, (SHORT)(0 - ConsoleScreenInfo.dwSize.Y) }, &chinfo);
	SetConsoleCursorPosition(/*GetStdHandle(STD_OUTPUT_HANDLE)*/handle, {0, 0});
#endif
	return 0;
}

IFUN(doEcho) {
	return 0;
}

IFUN(doGoto) {
	//todo check param type
	callParams.state->filepos = callParams.state->labels[_hashfunc_(TCAST(IdNode *, callParams.params[0])->str)];
	return 0;
}

IFUN(doStart) {
	return 0;
}

IFUN(doEndlocal) {
	return 0;
}

IFUN(doSetlocal) {
	return 0;
}

int hexValToDec(int hex) {
	int a = 0, ret = 0;
	do
		ret = ret * 10 + (hex << a++);
	while(hex && (hex /= 10));
	return ret;
}

IFUN(doColor) {
#ifdef _WIN64
	WORD color = 0x70;

	if(callParams.params[0]->type == LN(LNODE_NUMBER)) color = hexValToDec(TCAST(NumberNode *, callParams.params[0])->num);
	else color = strtol(TCAST(StringNode *, callParams.params[0])->str, NULL, 16);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwWritten;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
		FillConsoleOutputAttribute(hStdOut, color, csbi.dwSize.Y*csbi.dwSize.X, {0, 0}, &dwWritten);
		SetConsoleTextAttribute(hStdOut, color);
		return 0;
	}

	return 1;

#endif
	return 0;
}

IFUN(doExit) {
	return 0;
}

// -- MID PRIORITY --

IFUN(doCopy) {
	return 0;
}

IFUN(doDel) {
	return 0;
}

IFUN(doErase) {
	return 0;
}

IFUN(doMkdir) {
	return 0;
}

IFUN(doDir) {
	return 0;
}

IFUN(doMove) {
	return 0;
}

IFUN(doRename) {
	return 0;
}

IFUN(doRmdir) {
	return 0;
}

// -- LOW PRIORITY --

IFUN(doDate) {
	return 0;
}

IFUN(doTime) {
	return 0;
}

IFUN(doPause) {
#ifdef _WIN64
	printf("Press any key to continue...");

	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	getch();
	printf("\n");

#endif
	return 0;
}

IFUN(doShift) {
	return 0;
}

IFUN(doTitle) {
#ifdef _WIN64
	// this will NOT work
	SetConsoleTitleA(TCAST(StringNode *, callParams.params[0])->str);
#endif
	return 0;
}

IFUN(doType) {
	for(int i = 0; i < callParams.noParams; i++) {
		if((callParams.params[i]->type & BARETYPE) == LN(LNODE_ID) || (callParams.params[i]->type & BARETYPE) == LN(LNODE_STRING)) {
			// todo replace with createfile, getfilesize & readfile
			FILE *f = fopen(TCAST(StringNode *, callParams.params[i])->str, "r");
			fseek(f, 0, SEEK_END);
			int sz = ftell(f);
			rewind(f);

			char *buf = malloc((sz+1)*sizeof(char));
			fread(buf, sz, 1, f);
			buf[sz] = 0;
			write(1, buf, sz);
			write(1, "\n", 1);

			free(buf);
			fclose(f);
		}
	}

	return 0;
}

IFUN(doPushd) {
	char fullpath[PATH_MAX] = {0};
	realpath(TCAST(StringNode *, callParams.params[0])->str, fullpath);
	callParams.state->directoryStack.push(strdup(fullpath));
	chdir(fullpath);
	return 0;
}

IFUN(doPopd) {
	chdir(callParams.state->directoryStack.top());
	callParams.state->directoryStack.pop();
	return 0;
}

IFUN(doAssoc) {
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

// -- VERY LOW PRIORITY --

IFUN(doHelp) {
	char *str = 
	"If you installed this, you already know how to use cmd.exe and you are only going after speed.\n"
	"So I am not going to waste time by writing documentation. Have a nice day!\n";

	write(1, str, strlen(str));
	return 0;
}

IFUN(doVer) {
	printf("cmd_new version %d dev alpha\n", 1);
	return 0;
}

IFUN(doRem) {
	return 0;
}

IFUN(doBreak) {
	return 0;
}

IFUN(doPrompt) {
	return 0;
}

IFUN(doVerify) {
	return 0;
}

IFUN(doVol) {
	return 0;
}

IFUN(doLabel) {
	return 0;
}

IFUN(doPath) {
	return 0;
}

#ifdef _WIN64
extern IFUN(doInject);
#endif

std::unordered_map<_hashtype_, std::pair<uint8_t, CallPtr>, Hasher> multicharMapping = {
	fe("do", TOK_DO, nullptr)
	fe("else", TOK_ELSE, nullptr)
	fe("in", TOK_IN, nullptr)
	fe("for", TOK_FOR, nullptr)
	fe("if", TOK_IF, nullptr)

	fe("call", TOK_BUILTIN, doCall)
	fe("cd", TOK_BUILTIN, doChdir)
	fe("chdir", TOK_BUILTIN, doChdir)
	fe("cls", TOK_BUILTIN, doCls)
	fe("copy", TOK_BUILTIN, doCopy)
	fe("date", TOK_BUILTIN, doDate)
	fe("del", TOK_BUILTIN, doDel)
	fe("dir", TOK_BUILTIN, doDir)
	fe("echo", TOK_BUILTIN, doEcho)
	fe("erase", TOK_BUILTIN, doErase)
	fe("goto", TOK_BUILTIN, doGoto)
	fe("help", TOK_BUILTIN, doHelp)
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
	multicharMapping[_hashfunc_(cmd)] = std::make_pair(TOK_BUILTIN, func);
}

void RegisterLabel(InterpreterState *state, LabelNode *n) {
	state->labels[_hashfunc_(n->str)] = n->pos;
}

Interpreter::Interpreter(char *buffer, size_t size)
 : parser(buffer, size)
{
	parser.parse();
	state.buffer = buffer;
	state.bufferSize = size;
}

extern char *itoa_(int i);

int Interpreter::interpret() {

	// loop over root nodes
	Node *current;
	//int &i = this->state.filepos;
	for(; this->state.filepos < this->nodes.size(); this->state.filepos++) {
		current = nodes[this->state.filepos];
		switch(current->type) {
		case LN(LNODE_CALL): {
			auto retcode = TCAST(CallNode*, current)->execute(&this->state);
			setenv("errorlevel", itoa_(retcode), true);
			break;
		};

		case LN(LNODE_LABEL): {
			TCAST(LabelNode *, current)->_register_(&this->state);
			break;
		}
		}
	}

	return 0;
}
