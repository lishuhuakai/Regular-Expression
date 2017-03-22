#include "SyntaxTree.h"
#include "Lex.h"
#include <fstream>
#include <assert.h>

SyntaxTree::SyntaxTree(Lex& lex) :
	lex_(lex)
{
}


/*
 * buildSyntaxTree 从这个函数开始,自上而下parse正则表达式,构建解析树.
 */
nodePtr SyntaxTree::buildSyntaxTree()
{
	currentPos_ = getNextToken();
	tokenPtr t1 = currentPos_;
	bool lb = false;
	if (currentPos_->type == Token::kBoundary) {
		currentPos_ = getNextToken();
	}
	auto tree = parseRegularExpression();
	assert(currentPos_->type == Token::kEOF); /* 如果不等于的话,表示正则式语法有错 */
	return tree;
}

SyntaxTree::~SyntaxTree()
{

}

tokenPtr SyntaxTree::getNextToken()
{
	if (!tokens_.empty()) {
		auto ptr = tokens_.front();
		tokens_.pop_front();
		return ptr;
	}
	else
		return lex_.getNextToken();
}

/*
 * peekToken 用于偷窥位于currentPos_之后的第pos个token.
 */
tokenPtr SyntaxTree::peekToken(int pos)
{
	assert(pos >= 0);
	if (pos == 0) return currentPos_;
	while (tokens_.size() < pos)
		tokens_.push_back(lex_.getNextToken());

	return tokens_.at(pos - 1);
}

bool SyntaxTree::isBoundary(tokenPtr& t)
{
	if (t->type == Token::kBoundary) {
		currentPos_ = getNextToken();
	}
	/* 总可以返回true */
	return true;
}

bool SyntaxTree::is(tokenPtr& ptr, Token::tokenType t) {
	if (ptr->type == t) {
		return true;
	}
	else
		return  false;
}

nodePtr SyntaxTree::parseRegularExpression() {
	vector<nodePtr> nodes;
	vector<char> relations;
	while (true) {
		nodePtr sub = parseItems();
		if (sub) {
			nodes.push_back(sub);
			if (is(currentPos_, Token::kAlter)) {
				currentPos_ = getNextToken();
				relations.push_back('|');
			}
			else {
				relations.push_back('+');
			}
		}
		else
			break;
	}
	/* 接下来从左至右构建树 */
	int size = nodes.size();
	if (size == 0)
		return nodePtr();
	else if (size == 1)
		return nodes[0];
	else {
		auto n1 = nodes[0];
		nodePtr n2;
		for (int i = 1, j = 0; i < size; ++i, ++j) {
			n2 = nodes[i];
			if (relations[j] == '+') {
				n1 = make_shared<ConcatNode>(L"Concat(+)", n1, n2);
			}
			else {
				n1 = make_shared<AlterNode>(L"Alternative(|)", n1, n2);
			}
		}
		return n1;
	}
}

nodePtr SyntaxTree::parseItems() {
	auto ltree = parseItem();
	if (ltree) {
		bool isRepeate = is(currentPos_, Token::kRepeate);
		if (isRepeate && is(peekToken(1), Token::kGreedy)) {
			nodePtr sub = make_shared<RepeateNode>(currentPos_->content, ltree);
			currentPos_ = getNextToken(); currentPos_ = getNextToken();
			return make_shared<MethodNode>(L"?", sub);
		}
		else if (isRepeate) {
			auto token = currentPos_;
			currentPos_ = getNextToken();
			return make_shared<RepeateNode>(token->content, ltree);
		}
		else
			return ltree;
	}
	return nodePtr(); /* 返回一个空指针 */
}

nodePtr SyntaxTree::parseItem() {
	if (peekToken(0)->type == Token::kRange) {
		auto token = currentPos_;
		currentPos_ = getNextToken();
		return make_shared<RangeNode>(token->content);
	}
	else if (currentPos_->type == Token::kLBracket) {
		currentPos_ = getNextToken();
		auto token = parseItem();
		if (token && currentPos_->type == Token::kRBracket) {
			currentPos_ = getNextToken();
			return token;
		}
		assert(0); /* 否则就是语法错误了. */
	}
	else if (currentPos_->type == Token::kMethodLBracket) {
		currentPos_ = getNextToken();
		auto node = parseMethod();
		if (node && currentPos_->type == Token::kRBracket) {
			currentPos_ = getNextToken();
			return node;
		}
		assert(0);
	}
	return nodePtr();  /* 返回一个空指针 */
}

nodePtr SyntaxTree::parseMethod() {
	if (currentPos_->type == Token::kEqual) { /* 正向匹配 */
		currentPos_ = getNextToken();
		auto sub = parseRegularExpression();
		if (sub)
			return make_shared<MethodNode>(L"Positive Match", sub);
		assert(0);
	}
	else if (currentPos_->type == Token::kExClamation) { /* 反向匹配 */
		currentPos_ = getNextToken();
		auto sub = parseRegularExpression();
		if (sub)
			return make_shared<MethodNode>(L"Reverse Match", sub);
		assert(0);
	}
	else if (currentPos_->type == Token::kColon) { /* 命名或者匿名捕获 */
		currentPos_ = getNextToken();
		if (currentPos_->type == Token::kCaptureName) { /* 命名捕获 */
			auto token = currentPos_;
			currentPos_ = getNextToken();
			auto sub = parseRegularExpression();
			if (sub)
				return make_shared<MethodNode>(token->content, sub);
			assert(0);
		}
		else if (currentPos_->type == Token::kCheckName) { /* 检查 */
			auto token = currentPos_;
			currentPos_ = getNextToken();
			return make_shared<MethodNode>(token->content);
		}
		auto sub = parseRegularExpression();
		if (sub)
			return make_shared<MethodNode>(L"Anonymous Capture()", sub);
		assert(0);
	}
}


/*
* printTree 附加的功能,用于输出构建的语法树,生成了一个gv文件,你可以在graphviz中打开,方便调试.
*/
void SyntaxTree::printTree() {
	wofstream file("SyntaxTree.gv", wios::out);
	file.imbue(std::locale("chs")); //设置为中文
	auto t = buildSyntaxTree();
	wstring labels;
	wstring relations;
	if (file.is_open()) {
		file << L"digraph syntaxTree {" << endl;
		drawSubTree(t, labels, relations);
		file << labels.c_str();
		file << relations.c_str();
		file << L"}" << endl;
	}
	file.close();
}

/*
* drawSubTree 绘制子树
*/
wstring SyntaxTree::drawSubTree(nodePtr& ptr, wstring& labels, wstring& relations) {
	wstring lb = getRandomLabel();
	labels = labels + lb + L"[label=\"" + ptr->pattern_ + L"\"];\n";
	wcout << ptr->pattern_ << endl;
	switch (ptr->type_) {
	case Node::range:
		return lb; /* 返回节点的名称 */
	case Node::repeate:
	{
		wstring lt = drawSubTree(ptr->left_, labels, relations); // 左子树
		relations = relations + lb + L"->" + lt + L";\n";
		return lb;
	}
	case Node::alternative:
	case Node::concat:
	{
		wstring lt = drawSubTree(ptr->left_, labels, relations);
		wstring rt = drawSubTree(ptr->right_, labels, relations);
		relations = relations + lb + L"->" + lt + L";\n";
		relations = relations + lb + L"->" + rt + L";\n";
		return lb;
	}
	case Node::method:
	{
		if (ptr->left_) {
			wstring lt = drawSubTree(ptr->left_, labels, relations);
			relations = relations + lb + L"->" + lt + L";\n";
		}
		return lb;
	}
	}
	return labels;
}

/*
* getRaomLabel  获取一个随机的,但是每次都不一样的label,实际上也不是每次都不一样,只是有一个周期而已.
*/
wstring SyntaxTree::getRandomLabel() {
	wstring label;
	static wchar_t first = L'a';
	static wchar_t mid = L'A';
	static wchar_t num1 = L'0';
	static wchar_t num2 = L'1';
	label += first; first++;
	label += mid; mid++;
	label += num1; num1++;
	label += num2; num2++;
	if (first > L'z') first = L'a';
	if (mid > L'Z') mid = L'A';
	if (num1 > L'9') num1 = L'0';
	if (num2 > L'9') num2 = L'0';
	return label;
}

