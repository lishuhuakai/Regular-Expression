#include "SyntaxTree.h"
#include "Node.h"


SyntaxTree::SyntaxTree(Lex& lex) :
	lex_(lex)
{
}


SyntaxTree::~SyntaxTree()
{
}

tokenPtr SyntaxTree::getNextToken() { // 这仅仅是一个缓冲层
	previousToken_ = currentToken_; // 记录前一个token
	if (!tokens_.empty()) {
		tokenPtr t = tokens_.front();
		tokens_.pop_front();
		return t;
	}
	else
		return lex_.getNextToken();
}

void SyntaxTree::insertToken(tokenPtr& t) {
	tokens_.push_front(t); // 在前面插入一条token
}

bool SyntaxTree::shouldInsertConcat() { // 判断是否应该插入+token
	// 还有新的情况的话,可以直接在里面添加
	// kRepeate包括*, ?, {}三种情况,一视同仁
	if (   (previousToken_->type == Token::kNormal && currentToken_->type == Token::kNormal)		// aa
		|| (previousToken_->type == Token::kNormal && (currentToken_->type == Token::kLBracket || 
			currentToken_->type == Token::kMethod))													// a(
		|| (previousToken_->type == Token::kRBracket && currentToken_->type == Token::kNormal)		// )a
		|| (previousToken_->type == Token::kRBracket && (currentToken_->type == Token::kLBracket ||
			currentToken_->type == Token::kMethod))													// )(
		|| (previousToken_->type == Token::kRepeate && currentToken_->type == Token::kNormal)		// *a
		|| (previousToken_->type == Token::kRepeate && (currentToken_->type == Token::kLBracket ||
			currentToken_->type == Token::kMethod)))												// *(
		return true;
	return false;
}

// ?, *, {}这3种其实都表示重复的次数,其实可以合并成为一个符号
// [], .其实和普通的字符区别不大,只是它们选择的面更加宽广,如果我们抽象一下,它们几乎没有区别
const char priority[7][7] = { 
	// 一般而言, <代表先计算比较的运算符, >代表先计算栈顶的运算符, x代表正则式语法错误
	// <代表入栈, >代表先计算栈顶的运算符
	// m代表方法,方法和(是同优先级
    // (    )	 |    {}   +    m    #
	{ '<', '=', '<', '<', '<', '<', 'x' }, // ( 栈顶元素是(,那么一般而言比较的符号要压栈
	{ 'x', '>', '>', '>', '>', 'x', '>' }, // ) 栈顶元素是), 
	{ '<', '>', '>', '<', '<', '<', '>' }, // |
	{ '>', '>', '>', 'x', '>', '>', '>' }, // {}
	{ '<', '>', '>', '<', '>', '<', '>' }, // +
	{ '<', '=', '<', '<', '<', '<', 'x' }, // m
	{ '<', 'x', '<', '<', '<', '<', '=' }, // #
};

void SyntaxTree::initContext() {
	while (!operands_.empty()) operands_.pop();
	while (!operators_.empty()) operators_.pop();
	lex_.clear();
}

void SyntaxTree::clearContext() {
	while (!operands_.empty()) operands_.pop();
	while (!operators_.empty()) operators_.pop();
	currentToken_.reset();
	previousToken_.reset();
}

operandPtr SyntaxTree::buildSyntaxTree() {
	initContext(); // 初始化上下文的信息
	currentToken_ = make_shared<Token>(Token::kOrigin, L"#");
	operators_.push(currentToken_); // 首先压入一个底部
	currentToken_ = getNextToken();
	while (!operators_.empty()) {

		if (currentToken_->type == Token::kEOF) {
			// 所有的字符都已经读取完毕了,那么接下来要做的事情很简单,那就是将operator中所有的东西都消除掉
			currentToken_ = make_shared<Token>(Token::kOrigin, L"#");
		}

		if (shouldInsertConcat()) { // 判断是否应该插入连接符号
			insertToken(currentToken_); // 缓存命令,首先插入+
			currentToken_ = make_shared<Token>(Token::kConcat, L"+",  2);
		}
		switch (currentToken_->type)
		{
		case Token::kNormal: // 如果是操作数
			operands_.push(make_shared<RangeNode>(currentToken_->content, allCharSets_));
			currentToken_ = getNextToken();
			break;
		default:	// 如果是操作符
			Token::tokenType l = operators_.top()->type, r = currentToken_->type;
			switch (priority[l][r]) {
			case '>': // 栈顶元素优先级比较高
			{
				tokenPtr top = operators_.top();
				operators_.pop();
				operandPtr t1, t2;
				if (top->operators == 2) {
					t1 = operands_.top(); operands_.pop();
					t2 = operands_.top(); operands_.pop();
					if (top->type == Token::kVerticalBar) { // |
						operands_.push(make_shared<AlterNode>(top->content, t2, t1));
					}
					else if (top->type == Token::kConcat) { // +
						operands_.push(make_shared<ConcatNode>(top->content, t2, t1));
					}
				}
				break;
			}
			case '<': // 栈顶元素优先级低,那么操作符入栈
			{
				operandPtr t1;
				if (currentToken_->operators == 1) { // 单目运算符,现在就必须就计算
					t1 = operands_.top(); operands_.pop();
					if (currentToken_->type == Token::kRepeate) { // 包含*, ?, {}三种情况
						operands_.push(make_shared<RepeateNode>(currentToken_->content, t1));
					}
				}
				else {
					operators_.push(currentToken_);
				}
				currentToken_ = getNextToken();
				break;
			}
			case '=': // 两者的优先级相等,一般是脱去括号
			{
				tokenPtr top = operators_.top();
				operators_.pop();
				if (top->type == Token::kMethod) { // 虽然方法(包括(?, (?:等)虽然和(同优先级,但是对于方法,我们需要插入一个新的节点
					operandPtr t1 = operands_.top(); operands_.pop();
					operands_.push(make_shared<MethodNode>(top->content, t1));
				}
				currentToken_ = getNextToken();
				break;
			}
			default:
				wcout << L"正则表达式错误" << endl;

				break;
			}
		}
	}
	if (operands_.size() != 1) {
		wcout << L"正则表达式错误" << endl;
	}
	operandPtr res = operands_.top();
	clearContext();
	return res;
}


void SyntaxTree::printTree() {
	wofstream file("SyntaxTree.gv", wios::out);
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

wstring SyntaxTree::drawSubTree(operandPtr& ptr, wstring& labels, wstring& relations) {
	wstring lb = getRandomLabel();
	labels = labels + lb + L"[label=\"" + ptr->pattern_ + L"\"];\n";
	switch (ptr->type_) {
	case Node::range:
		return lb; // 返回节点的名称
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
	}
	return labels;
}

wstring SyntaxTree::getRandomLabel() { // 获取一个随机的,但是每次都不一样的label
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
