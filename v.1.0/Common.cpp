#include "Common.h"

void printErrorAndExit(const wstring& msg) {
	wcout << msg << endl;
	exit(-1);
}

int nextLabel() {
	static int count = 0;
	return ++count;
}

