#include "Node.h"

Node::Node(type t, wstring& pattern, nodePtr left, nodePtr right) :
	type_(t),
	pattern_(pattern),
	left_(left),
	right_(right)
{
}


Node::~Node()
{

}

void  RangeNode::getRange(wstring& str) {
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] == '\\') { // 转义字符
			if (i + 1 < str.size()) { content_.insert(str[i + 1]); ++i; }
			else printErrorAndExit(L"单一的\\字符");
		}
		else if ((i + 2 < str.size()) && 
			(str[i] != L'-' && str[i + 1] == L'-' && str[i + 2] != L'-')) { // 这里表示一个范围
				assert(str[i] <= str[i + 2]);
				for (wchar_t c = str[i]; c <= str[i + 2]; c++)
					content_.insert(c);
				i = i + 2;
		}
		else content_.insert(str[i]);
	}
}

void RangeNode::parseRange(const wstring& pattern) {
	if (pattern[1] == L'^') {
		getRange(pattern.substr(2, pattern.size() - 3)); // 去掉最后的]
		notIncludeContent_ = true;  // 不用包含content_里面的内容
	}
	else {
		getRange(pattern.substr(1, pattern.size() - 2));
	}
}

RangeNode::RangeNode(wstring& pattern, list<setPtr>& allCharSets) : 
	Node(range, pattern),
	includeAll_(false),
	notIncludeContent_(false) // notIncludeContent_一般用于[^x],如果其为true,表示不包含x
{
	// 这里要做的事情就是将range放入到set之中
	if (pattern == L".") {
		includeAll_ = true;
	}
	else if (pattern.size() != 0 && pattern[0] == L'[') { // 现在开始解析[], []表达式包含这些形式:
		// [abc][a-c][^abc][^a-c]
		parseRange(pattern);
	}
	else if (pattern[0] == L'\\') {
		assert(pattern.size() == 2);
		if (pattern[1] == L'd')  parseRange(L"[0-9]");
		else if (pattern[1] == L'D') parseRange(L"[^0-9]");
		else if (pattern[1] == L'w') parseRange(L"[a-zA-Z0-9_]");
		else if (pattern[1] == L'W') parseRange(L"[^a-zA-Z0-9_]");
		else if (pattern[1] == L's') parseRange(L"[ \\t]");
		else assert(0);
	}
	else { // 接下来就是普通的字符
		assert(pattern.size() == 1);
		content_.insert(pattern[0]);
	}
	// content_表示set<wchar_t>
	if (pattern != L".") // 因为实际上,如果pattern为.的话,它的内容是空的.
		allCharSets.push_back(make_shared<charSet>(content_));  // 将字符集往全局的字符集中拷贝一份
}

ConcatNode::ConcatNode(wstring& pattern, nodePtr& l, nodePtr& r) :
	Node(concat, pattern, l, r)
{

}


AlterNode::AlterNode(wstring& pattern, nodePtr& l, nodePtr& r) :
	Node(alternative, pattern, l, r)
{

}

void RepeateNode::getRepeateTimes(wstring& str) {
	int t1 = 0, t2 = 0;
	size_t index = 0;
	for ( ; index < str.size(); ++index) {
		if (str[index] == L',') break;
		else if (isDigit(str[index])) {
			t1 = t1 * 10 + (str[index] - L'0');
		}
		else
			printErrorAndExit(str + L"重复运算符中包含了非数字");
	}
	minTimes_ = t1;
	++index; // 这里表示去除了,
	if (index == str.size()) { // 
		maxTimes_ = -1;
		infinity_ = true;
	}
	else {
		for (; index < str.size(); ++index) {
			if (isDigit(str[index])) 
			t2 = t2 * 10 + (str[index] - L'0');
			else
				printErrorAndExit(str + L"重复运算符中包含了非数字");
		}
		maxTimes_ = t2;
		infinity_ = false;
	}

}

RepeateNode::RepeateNode(wstring& pattern, nodePtr& sub) :
	Node(repeate, pattern, sub)
{
	// 在重复字符中,需要解析出重复的次数,重复一共有下面几种形式:
	// *, ?, {m,}, {,n}, {m,n}
	if (pattern.size() == 1) {
		if (pattern == L"*") { // 重复零次或者多次
			minTimes_ = 0; maxTimes_ = -1; infinity_ = true;
		}
		else if (pattern == L"?") {
			minTimes_ = 0; maxTimes_ = 1; infinity_ = false;
		}
		else if (pattern == L"+") { // 匹配一个或者多个
			minTimes_ = 1; maxTimes_ = -1; infinity_ = true;
		}
		else	assert(0);
	}
	else if (pattern[0] == L'{') { // 可以保证,有{,那么必定有}
		// 说实话,解析真的挺蛋疼的
		getRepeateTimes(pattern_.substr(1, pattern_.size() - 2));
	}
	else
		assert(0);
}

MethodNode::MethodNode(wstring& pattern, nodePtr& sub) :
	Node(method, pattern, sub)
{

}