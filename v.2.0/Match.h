#ifndef MATCH_H
#define MATCH_H
#include "Common.h"
#include "Status.h"

class Nfa;

/*
 * CaptureItem 用于记录捕获则内容,开始的位置和捕获的长度这两项必不可少,因为这样才能唯一确定一个子串.
 */
struct CaptureItem {
	wstring name; /* 名称 */
	size_t depth; /* 用于记录捕获这个元素的时候的堆栈的深度 */
	int startPos; /* 捕获的起始位置 */
	int len; /* 捕获的长度 */
	CaptureItem(wstring& name, int stackDepth, int startPos, int len) :
		name(name), depth(stackDepth), startPos(startPos), len(len)
	{}
};

struct CommandItem {
	Edge::type tp; /* 命令的类型 */
	wstring name; /* 用于记录一些参数信息,这个选项一般只有storage命令才使用 */
	int prevNotEndedCommandPos;  /* 上一个未完成的指令在堆栈中的位置 */
	int startPos; /* 用于记录产生这个命令时,字符串的起始位置 */

	CommandItem(Edge::type tp, wstring& name, int prevNotEndedCommandPos, int strCurrentPos) :
		tp(tp), name(name), prevNotEndedCommandPos(prevNotEndedCommandPos), startPos(strCurrentPos)
	{}
};

struct StatusItem {
	statusPtr stat; /* Nfa的指针 */
	size_t label; /* 当前边的序号 */
	size_t depth; /* 状态产生时堆栈的深度 */
	int startPos; /* 字符串的起始位置 */

	StatusItem(statusPtr& s, int lb, int depth, int sp) :
		stat(s), label(lb), depth(depth), startPos(sp)
	{}
};

class Match
{
	typedef shared_ptr<StatusItem> statusItemPtr;
public:
	Match(Nfa&, const wstring&);
	~Match();
public:
	bool greedyMatch();
	void handleEnd(CommandItem& item, int currentNotEndedCommandPos);
	bool handleMethodItem(edgePtr&, int&); /* 处理各种检查 */
	void printCaptureContent(); /* 用于输出捕获的内容 */
private:
	wstring matchContent_;
	Nfa& nfa_;
	stack<CaptureItem> storage_;
	vector<CommandItem> commands_;
	stack<statusItemPtr> status_;
	int prevNotEndedCommandPos_;
private:
	void restoreStack(statusItemPtr&); /* 重新恢复栈 */
	void clearCommandAndStatusStack(); /* 清空command和status两个栈 */
	void clearAllStack(); /* 清空所有的栈 */
};

#endif // !MATCH_H
