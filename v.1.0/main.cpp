#include "Lex.h"
#include "SyntaxTree.h"
#include "Nfa.h"
#include "Dfa.h"
#include <iostream>
#include <stack>
using namespace std;

int main() {
	_wsetlocale(LC_ALL, L"chs"); 
	//Lex lex("\\\\\\d");
	//Lex lex(L"(a|b)*abb");
	//Lex lex(L"(a*)(b)*|((c)(d))*");
	//Lex lex(L"(ab)?cd");
	//Lex lex(L"a?");
	//Lex lex(L"(?:98|2000)");
	//Lex lex("\\(((?:<#markup>\\w+)(\\s+(?:<#attribute>\\w+)=(?:<#value>\"[^\"]+\"))?)*\\)");
	//Lex lex(L"\\((?:<#markup>\\w+)(\\s+(?:<#attribute>\\w+)=(?:<#value>\"[^\"]+\"))*\\)");
	//Lex lex(L"abc(a|b|c)*cba");
	//Lex lex(L"(\\s+)");
	//Lex lex(L"a{3,6}bc");
	Lex lex(L"[^abcd]efg");
	//Lex lex(L"ab");
	SyntaxTree tree(lex);
	//tree.printTree();
	EpsNfa epsNfa(tree);
	Nfa nfa(epsNfa);
	Dfa dfa(nfa);
	//machine.buildDFA();
	//Lex lex("\.\\\*\(\)\|");
	//Lex lex("[a-z_]{3,6}");
	//bool ans = dfa.match(L"abcaacba");
	wstring str;
	while (1) {
		wcin >> str;
		cout << dfa.match(str) << endl;
	}
	getchar();
}