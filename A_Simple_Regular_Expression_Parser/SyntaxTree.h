#ifndef SYNTAXTREE_H_
#define SYNTAXTREE_H_
#include "Node.h"
#include "Lex.h"
#include <deque>
using namespace std;
class Lex;

class SyntaxTree
{
public:
	SyntaxTree(Lex&);
	nodePtr buildSyntaxTree();
	~SyntaxTree();
	void SyntaxTree::printTree();
private:
	Lex& lex_;
	tokenPtr currentPos_;
	deque<tokenPtr> tokens_; /* 用于记录Token */
private:
	bool isBoundary(tokenPtr&);
	bool is(tokenPtr&, Token::tokenType);
	tokenPtr getNextToken();
	tokenPtr peekToken(int pos);		/* 事先查看Token */
	nodePtr parseRegularExpression();
	nodePtr parseItems();
	nodePtr parseItem();
	nodePtr parseMethod();
	wstring getRandomLabel();
	wstring drawSubTree(nodePtr& ptr, wstring& labels, wstring& relations);
};

#endif // !SYNTEXTREE_H_
