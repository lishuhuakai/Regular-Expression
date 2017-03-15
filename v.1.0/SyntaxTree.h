#pragma once
#include <deque>
#include <stack>
#include "lex.h"
using namespace std;
class Node;

typedef shared_ptr<Node> operandPtr;
class SyntaxTree
{
public:
	SyntaxTree(Lex&);
	~SyntaxTree();
public:
	Lex& lex_;
	deque<tokenPtr> tokens_;		// 用于传递命令
	stack<operandPtr> operands_;	// 操作数栈
	stack<tokenPtr> operators_;		// 操作符栈
	list<setPtr> allCharSets_;		// 用于记录正则式中出现的字符集合
public:
	tokenPtr getNextToken();
	void insertToken(tokenPtr&);
	operandPtr buildSyntaxTree();
	void printTree();  // 绘制一棵树

private:
	tokenPtr currentToken_;
	tokenPtr previousToken_;
	bool shouldInsertConcat();
	void initContext(); // 初始化环境
	void clearContext(); // 清空环境
	wstring drawSubTree(operandPtr&, wstring&, wstring&);
	wstring getRandomLabel();
};

