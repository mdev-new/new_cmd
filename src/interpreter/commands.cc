#include "commands.hh"
#include "interpreter/lexer/token.hpp"

#include "standard.h"
#include "interpreter/nodes.hh"
#include "utilities/hashing.hpp"

// todo use windows functions on win and linux functions on linux
// lets not rely as much on stdlib

// -- HIGH PRIORITY --
// todo finish implementation
IFUN(doSet) {
	Node *current;
	for(int i = 0; i < callParams.params->size(); i++) {
		current = (*callParams.params)[i];
		if((current->type & BARETYPE) == Node::Type::Assign)
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
	printf("%s\n", (*callParams.params)[0]->evaluate());
	return 0;
}

IFUN(doGoto) {
	//todo check param type
	callParams.state->filepos = callParams.state->labels[TCAST(IdNode *, (*callParams.params)[0])->hash];
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
		if((*callParams.params)[0]->type == Node::Type::Number) color = TCAST(NumberNode *, (*callParams.params)[0])->num;
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
	const static char mapping[] = {
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
		if(((*callParams.params)[i]->type & BARETYPE) == Node::Type::Id || ((*callParams.params)[i]->type & BARETYPE) == Node::Type::String) {
			// todo replace with createfile, getfilesize & readfile
			FILE *f = fopen(TCAST(StringNode *, (*callParams.params)[i])->str, "r");
			fseek(f, 0, SEEK_END);
			int sz = ftell(f);
			rewind(f);

			char *buf = malloc((sz+1)*sizeof(char));
			fread(buf, sz, 1, f);
			buf[sz] = 0;
			fwrite(buf, 1, sz, callParams.fpWrite);

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

	//printf("pushed: %s\n", callParams.state->directoryStack.top());

	return 0;
}

IFUN(doPopd) {
	//printf("popping: %s\n", callParams.state->directoryStack.top());
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

	fwrite(str, 1, strlen(str), callParams.fpWrite);
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

//#ifdef _WIN64
extern IFUN(doInject);
//#endif

std::unordered_map<_hashtype_, std::pair<decltype(Token::type), CallPtr>, Hasher> multicharMapping = {
	fe("call", Token::Type::BuiltIn, doCall)
	fe("cd", Token::Type::BuiltIn, doChdir)
	fe("chdir", Token::Type::BuiltIn, doChdir)
	fe("cls", Token::Type::BuiltIn, doCls)
	fe("copy", Token::Type::BuiltIn, doCopy)
	fe("date", Token::Type::BuiltIn, doDate)
	fe("del", Token::Type::BuiltIn, doDel)
	fe("dir", Token::Type::BuiltIn, doDir)
	fe("echo", Token::Type::BuiltIn, doEcho)
	fe("erase", Token::Type::BuiltIn, doErase)
	fe("goto", Token::Type::BuiltIn, doGoto)
	fe("help", Token::Type::BuiltIn, doHelp)
	fe("label", Token::Type::BuiltIn, doLabel)
	fe("md", Token::Type::BuiltIn, doMkdir)
	fe("mkdir", Token::Type::BuiltIn, doMkdir)
	fe("move", Token::Type::BuiltIn, doMove)
	fe("path", Token::Type::BuiltIn, doPath)
	fe("pause", Token::Type::BuiltIn, doPause)
	fe("prompt", Token::Type::BuiltIn, doPrompt)
	fe("rem", Token::Type::BuiltIn, doRem)
	fe("ren", Token::Type::BuiltIn, doRename)
	fe("rename", Token::Type::BuiltIn, doRename)
	fe("rd", Token::Type::BuiltIn, doRmdir)
	fe("rmdir", Token::Type::BuiltIn, doRmdir)
	fe("set", Token::Type::BuiltIn, doSet)
	fe("shift", Token::Type::BuiltIn, doShift)
	fe("start", Token::Type::BuiltIn, doStart)
	fe("time", Token::Type::BuiltIn, doTime)
	fe("title", Token::Type::BuiltIn, doTitle)
	fe("type", Token::Type::BuiltIn, doType)
	fe("verify", Token::Type::BuiltIn, doVerify)
	fe("ver", Token::Type::BuiltIn, doVer)
	fe("vol", Token::Type::BuiltIn, doVol)
	fe("endlocal", Token::Type::BuiltIn, doEndlocal)
	fe("setlocal", Token::Type::BuiltIn, doSetlocal)
	fe("pushd", Token::Type::BuiltIn, doPushd)
	fe("popd", Token::Type::BuiltIn, doPopd)
	fe("assoc", Token::Type::BuiltIn, doAssoc)
	fe("color", Token::Type::BuiltIn, doColor)
	fe("ftype", Token::Type::BuiltIn, doFtype)
	fe("more", Token::Type::BuiltIn, doMore)
	fe("choice", Token::Type::BuiltIn, doChoice)
	fe("mklink", Token::Type::BuiltIn, doMklink)
	fe("exit", Token::Type::BuiltIn, doExit)
	fe("break", Token::Type::BuiltIn, doBreak)

//#ifdef _WIN64 // currently only Windows x64 supported, sorry
	fe("extend", Token::Type::BuiltIn, doInject)
//#endif
};
