#include "Common.h"

void printErrorAndExit(const wstring& msg) {
	wcout << msg << endl;
	exit(-1);
}

int nextLabel() {
	static int count = 0;
	return ++count;
}

int nextLabel2() {
	static int count2 = 0;
	return ++count2;
}
