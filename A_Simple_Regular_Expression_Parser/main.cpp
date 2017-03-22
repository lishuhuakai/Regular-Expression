#include <iostream>
#include "Lex.h"
#include "SyntaxTree.h"
using namespace std;

/*
 * 对正则表达式进行parse.
 */

int main() {
	//Lex lex(L"[0-9]*?|(?:<#sec>[0-9]ab+?)(?:<$sec>)+");
	Lex lex(L"a+b{0, 1}(?:jkljsldkfa)|(?:<#check>klj)\w\w[^kdsf]|(?=d{3, 5}?)");
//	Lex lex(L"abc{9, 10}?");
	//Lex lex(L"))))))");
	SyntaxTree tree(lex);
	//tree.buildSyntaxTree();
	tree.printTree();
	while (true) {
		tokenPtr p = lex.getNextToken();
		if (p->type == Token::kEOF) 
			break;
		wcout << *p << L"    " << p->type2String() << endl;
	}
	getchar();
}