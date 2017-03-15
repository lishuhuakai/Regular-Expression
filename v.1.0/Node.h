#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <memory>
#include "common.h"
using namespace std;

// Node类作为接下来所有类的一个父类,起到了非常重要的作用
class Node;
class RangeNode;
class ConcatNode;
class MethodNode;
class AlterNode;
class RepeateNode;

typedef shared_ptr<Node> nodePtr;
class Node
{
public:
	typedef shared_ptr<Node> nodePtr;
	enum type {range, concat, alternative, repeate, method};
public:
	class IVisitor {
	public:
		virtual void visit(RangeNode&) = 0;
		virtual void visit(AlterNode&) = 0;
		virtual void visit(ConcatNode&) = 0;
		virtual void visit(MethodNode&) = 0;
		virtual void visit(RepeateNode&) = 0;
	};
public:
	wstring pattern_;
	type type_;
	nodePtr left_;
	nodePtr right_;
	Node(type, wstring&, nodePtr left = NULL, nodePtr right = NULL);
	virtual ~Node();  // 一定要让父类的析构函数是虚函数
public:
	virtual void evaluate(IVisitor& v) = 0;
};

// RangeNode用于表示一个范围,如a, [a-z]等
class RangeNode :public Node
{
public:
	set<wchar_t> content_;
	vector<int> index_;  // 用于记录下标
public:
	bool includeAll_;  // 是否包含所有的字符
	bool notIncludeContent_;  // 是否不包含content_中的内容
	RangeNode(wstring &, list<setPtr>&);
	void getRange(wstring& str); // 获得范围
	void parseRange(const wstring&);

	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

// ConcatNode用于连接两个节点
class ConcatNode : public Node 
{
public:
	ConcatNode(wstring&, nodePtr&, nodePtr&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

// alternative 表示两个节点的选择
class AlterNode : public Node
{
public:
	AlterNode(wstring&, nodePtr&, nodePtr&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

// 重复,包括*, ?, +,以及{m, n}这几种形式
class RepeateNode : public Node 
{
public:
	int minTimes_;	/* 最少重复的次数 */
	int maxTimes_;	/* 最多重复的次数 */
	bool infinity_; /* 是否无限 */
public:
	RepeateNode(wstring&, nodePtr&);
	void getRepeateTimes(wstring&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
private:
	bool isDigit(wchar_t t) {
		return (t >= L'0' && t <= L'9');
	}
};

// MethodNode是一个通用的方法节点
class MethodNode : public Node 
{
public:
	enum methodType { forwardMatch, backwardMatch, anonymousCatch, Catch, Range};
	wstring name_;  /* 变量的名称 */
public:
	MethodNode(wstring&, nodePtr&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

#endif
