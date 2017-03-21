#include "SyntaxTree.h"
#include "Node.h"


SyntaxTree::SyntaxTree(Lex& lex) :
	lex_(lex)
{
}


SyntaxTree::~SyntaxTree()
{
}

tokenPtr SyntaxTree::getNextToken() {
	previousToken_ = currentToken_; /* 记录前一个token的内容 */
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

/*
 * shouldInsertConcat 用于判断是否应该插入Concat操作符,也就是连接操作符.
 */
bool SyntaxTree::shouldInsertConcat() { 
	// 还有新的情况的话,可以直接在里面添加
	// kRepeate包括*, +, {}三种情况,一视同仁
#define PREVIOUS_NORMAL ((previousToken_->type == Token::kCheck) || (previousToken_->type == Token::kNormal))
#define CURRENT_NORMAL ((currentToken_->type == Token::kCheck) || (currentToken_->type == Token::kNormal))
#define PREVIOUS_LBRACKET ((previousToken_->type == Token::kLBracket) || (previousToken_->type == Token::kMethod))
#define CURRENT_LBRACKET ((currentToken_->type == Token::kLBracket) || (currentToken_->type == Token::kMethod))
#define PREVIOUS_REPEATE (previousToken_->type == Token::kRepeate)
#define PREVIOUS_RBRACKET (previousToken_->type == Token::kRBracket)
	if (   (PREVIOUS_NORMAL && CURRENT_NORMAL) 
		|| (PREVIOUS_REPEATE && CURRENT_NORMAL)
		|| (PREVIOUS_REPEATE && CURRENT_LBRACKET)
		|| (PREVIOUS_NORMAL && CURRENT_LBRACKET) 
		|| (PREVIOUS_RBRACKET && CURRENT_NORMAL)	
		|| (PREVIOUS_RBRACKET && CURRENT_LBRACKET))
		return true;
	return false;
}

// +, *, {}这3种其实都表示重复的次数,其中+和*是{}的语法糖.
// [], .其实和普通的字符区别不大,只是它们选择的面更加宽广,如果我们抽象一下,它们几乎没有区别
const char priority[7][7] = { 
	// 一般而言, <代表先计算比较的运算符, >代表先计算栈顶的运算符, x代表正则式语法错误
	// <代表入栈, >代表先计算栈顶的运算符
	// m代表方法,方法和(是同优先级, c代表concat,表示连接
	// {}代表重复,包括({}, +, *三种语法糖)?代表非贪婪,{}和?优先级相同
	// 这里需要说明一下的是,?只能放在重复后面
    // (    )	 |    {}   c    m    #
	{ '<', '=', '<', '<', '<', '<', 'x' }, // ( 栈顶元素是(,那么一般而言比较的符号要压栈
	{ 'x', '>', '>', '>', '>', 'x', '>' }, // ) 栈顶元素是)
	{ '<', '>', '>', '<', '<', '<', '>' }, // |
	{ '>', '>', '>', 'x', '>', '>', '>' }, // {}
	{ '<', '>', '>', '<', '>', '<', '>' }, // c
	{ '<', '=', '<', '<', '<', '<', 'x' }, // m
	{ '<', 'x', '<', '<', '<', '<', '=' }, // #
};

/*
 * initContext 构建语法树之前应该调用这个函数,它清空操作符和操作数栈,并对以前遗留的数据进行清理.
 */
void SyntaxTree::initContext() {
	while (!operands_.empty()) operands_.pop();
	while (!operators_.empty()) operators_.pop();
	lex_.clear();
}

/*
 * clearContext 构建语法树之后也应该调用这个函数,它也做清理工作,这和initContext有功能上的重叠. to fix.
 */
void SyntaxTree::clearContext() {
	while (!operands_.empty()) operands_.pop();
	while (!operators_.empty()) operators_.pop();
	currentToken_.reset();
	previousToken_.reset();
}

operandPtr SyntaxTree::buildSyntaxTree() {
	initContext(); /* 初始化上下文的信息 */
	currentToken_ = make_shared<Token>(Token::kOrigin, L"#");
	operators_.push(currentToken_); /* 首先在操作符栈压入一个# */
	currentToken_ = getNextToken();
	while (!operators_.empty()) {
		if (currentToken_->type == Token::kEOF) {
			/* 所有的字符都已经读取完毕了,那么接下来要做的事情很简单,那就是将操作符栈中所有的东西都消除掉 */
			currentToken_ = make_shared<Token>(Token::kOrigin, L"#");
		}

		if (shouldInsertConcat()) { /* 判断是否应该插入连接符号 */
			insertToken(currentToken_); /* 缓存命令,首先插入Concat(连接)符 */
			currentToken_ = make_shared<Token>(Token::kConcat, L"~",  2);
		}
		switch (currentToken_->type)
		{
		case Token::kNormal: /* 如果是操作数 */
			operands_.push(make_shared<RangeNode>(currentToken_->content, allCharSets_));
			currentToken_ = getNextToken();
			break;
		case Token::kCheck:
			operands_.push(make_shared<MethodNode>(currentToken_->content));
			currentToken_ = getNextToken();
			break;
		default:	/* 如果是操作符 */
			Token::tokenType l = operators_.top()->type, r = currentToken_->type;
			switch (priority[l][r]) {
			case '>': /* 栈顶元素优先级比较高 */
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
			case '<': /* 栈顶元素优先级低,那么操作符入栈 */
			{
				operandPtr t1;
				if (currentToken_->operators == 1) { /* 单目运算符,现在就必须就计算 */
					t1 = operands_.top(); operands_.pop();
					if (currentToken_->type == Token::kRepeate) { /* 包含 *, +, {}三种情况 */
						operands_.push(make_shared<RepeateNode>(currentToken_->content, t1));
					}
				}
				else {
					operators_.push(currentToken_);
				}
				currentToken_ = getNextToken();
				break;
			}
			case '=': /* 两者的优先级相等,一般是脱去括号 */
			{
				tokenPtr top = operators_.top();
				operators_.pop();
				if (top->type == Token::kMethod) { 
					/* 虽然方法(包括(?, (?:等)虽然和(同优先级,但是对于方法,我们需要插入一个新的节点 */
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

/*
 * printTree 附加的功能,用于输出构建的语法树,生成了一个gv文件,你可以在graphviz中打开,方便调试.
 */
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

/*
 * drawSubTree 绘制子树
 */
wstring SyntaxTree::drawSubTree(operandPtr& ptr, wstring& labels, wstring& relations) {
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
