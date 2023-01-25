#include "shared.hh"
#include "lexer.hpp"

#include "win.hh"
#include "nodes.hh"

// todo use windows functions on win and linux functions on linux
// lets not rely as much on stdlib

// -- HIGH PRIORITY --
// todo finish implementation
IFUN(doSet) {
	Node *current;
	for(int i = 0; i < callParams.params->size(); i++) {
		current = (*callParams.params)[i];
		if((current->type & BARETYPE) == NodeType::Assign)
			TCAST(AssignNode *, current)->assign();
	}

	return 0;
}

IFUN(doCall) {
	return 0;
}

// only absolute paths atm, sorry
IFUN(doChdir) {
	chdir(TCAST(StringNode *, (*callParams.params)[0])->str);
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
#else
	printf("\e[1;1H\e[2J");
#endif
	return 0;
}

IFUN(doEcho) {
	return 0;
}

IFUN(doGoto) {
	//todo check param type
	callParams.state->filepos = callParams.state->labels[_hashfunc_(TCAST(IdNode *, (*callParams.params)[0])->str)];
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
	do ret = ret * 10 + (hex << a++);
	while(hex && (hex /= 10));
	return ret;
}

IFUN(doColor) {
	unsigned char color = 0x0F;

	if(callParams.params->size() != 0) {
		if((*callParams.params)[0]->type == NodeType::Number) color = TCAST(NumberNode *, (*callParams.params)[0])->num;
		else color = strtol(TCAST(IdNode *, (*callParams.params)[0])->str, NULL, 16);
	}

#ifdef _WIN64
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwWritten;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
		FillConsoleOutputAttribute(hStdOut, color, csbi.dwSize.Y*csbi.dwSize.X, {0, 0}, &dwWritten);
		SetConsoleTextAttribute(hStdOut, color);
		return 0;
	}

	return 1;
#else
	#warning "doColor needs testing"
	const static short mapping[] = {
		0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0x37,
		0x90, 0x94, 0x92, 0x96, 0x91, 0x95, 0x93, 0x97
	};
	//printf("setting color: bg: %x fg: %x\n", mapping[color >> 4] + 0x10, mapping[color & 0xF] & 0xFF);
	printf("\x1B[0;%x;%xm", mapping[color >> 4] + 0x10, mapping[color & 0xF] & 0xFF);
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
	SetConsoleTitleA(TCAST(StringNode *, (*callParams.params)[0])->str);
#else
	printf("\033]0;%s\007",TCAST(StringNode *, (*callParams.params)[0])->str); 
#endif
	return 0;
}

IFUN(doType) {
	for(int i = 0; i < callParams.params->size(); i++) {
		if(((*callParams.params)[i]->type & BARETYPE) == NodeType::Id || ((*callParams.params)[i]->type & BARETYPE) == NodeType::String) {
			// todo replace with createfile, getfilesize & readfile
			FILE *f = fopen(TCAST(StringNode *, (*callParams.params)[i])->str, "r");
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
	char *arg = TCAST(StringNode *, (*callParams.params)[0])->str;

#ifdef _WIN64
	bool isFullPath = arg[1] == ':' || arg[0] == '\\';
#else
	bool isFullPath = arg[0] == '/';
#endif

	if(isFullPath) {
		callParams.state->directoryStack.push(strdup(arg));
		chdir(arg);
	} else {
		char dir[PATH_MAX] = {0};
		getcwd(dir, sizeof(dir));
		strcat(dir, "/");
		strcat(dir, arg);

		callParams.state->directoryStack.push(strdup(dir));
		chdir(dir);
	}

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

#define fe(x,y,z) {consthash(x), { y,z } },
std::unordered_map<_hashtype_, std::pair<decltype(Token::type), CallPtr>, Hasher> multicharMapping = {
	fe("do", TokenType::Do, nullptr)
	fe("else", TokenType::Else, nullptr)
	fe("in", TokenType::In, nullptr)
	fe("for", TokenType::For, nullptr)
	fe("if", TokenType::If, nullptr)

	fe("gtr", TokenType::Gtr, nullptr)
	fe("geq", TokenType::Geq, nullptr)
	fe("lss", TokenType::Lss, nullptr)
	fe("leq", TokenType::Leq, nullptr)
	fe("equ", TokenType::Equ, nullptr)
	fe("neq", TokenType::Neq, nullptr)

	fe("call", TokenType::BuiltIn, doCall)
	fe("cd", TokenType::BuiltIn, doChdir)
	fe("chdir", TokenType::BuiltIn, doChdir)
	fe("cls", TokenType::BuiltIn, doCls)
	fe("copy", TokenType::BuiltIn, doCopy)
	fe("date", TokenType::BuiltIn, doDate)
	fe("del", TokenType::BuiltIn, doDel)
	fe("dir", TokenType::BuiltIn, doDir)
	fe("echo", TokenType::BuiltIn, doEcho)
	fe("erase", TokenType::BuiltIn, doErase)
	fe("goto", TokenType::BuiltIn, doGoto)
	fe("help", TokenType::BuiltIn, doHelp)
	fe("label", TokenType::BuiltIn, doLabel)
	fe("md", TokenType::BuiltIn, doMkdir)
	fe("mkdir", TokenType::BuiltIn, doMkdir)
	fe("move", TokenType::BuiltIn, doMove)
	fe("path", TokenType::BuiltIn, doPath)
	fe("pause", TokenType::BuiltIn, doPause)
	fe("prompt", TokenType::BuiltIn, doPrompt)
	fe("rem", TokenType::BuiltIn, doRem)
	fe("ren", TokenType::BuiltIn, doRename)
	fe("rename", TokenType::BuiltIn, doRename)
	fe("rd", TokenType::BuiltIn, doRmdir)
	fe("rmdir", TokenType::BuiltIn, doRmdir)
	fe("set", TokenType::BuiltIn, doSet)
	fe("shift", TokenType::BuiltIn, doShift)
	fe("start", TokenType::BuiltIn, doStart)
	fe("time", TokenType::BuiltIn, doTime)
	fe("title", TokenType::BuiltIn, doTitle)
	fe("type", TokenType::BuiltIn, doType)
	fe("verify", TokenType::BuiltIn, doVerify)
	fe("ver", TokenType::BuiltIn, doVer)
	fe("vol", TokenType::BuiltIn, doVol)
	fe("endlocal", TokenType::BuiltIn, doEndlocal)
	fe("setlocal", TokenType::BuiltIn, doSetlocal)
	fe("pushd", TokenType::BuiltIn, doPushd)
	fe("popd", TokenType::BuiltIn, doPopd)
	fe("assoc", TokenType::BuiltIn, doAssoc)
	fe("color", TokenType::BuiltIn, doColor)
	fe("ftype", TokenType::BuiltIn, doFtype)
	fe("more", TokenType::BuiltIn, doMore)
	fe("choice", TokenType::BuiltIn, doChoice)
	fe("mklink", TokenType::BuiltIn, doMklink)
	fe("exit", TokenType::BuiltIn, doExit)
	fe("break", TokenType::BuiltIn, doBreak)

#ifdef _WIN64 // currently only Windows x64 supported, sorry
	fe("extend", TokenType::BuiltIn, doInject)
#endif
};

void RegisterCommand(char *cmd, CallPtr func) {
	multicharMapping[_hashfunc_(cmd)] = std::make_pair(TokenType::BuiltIn, func);
}