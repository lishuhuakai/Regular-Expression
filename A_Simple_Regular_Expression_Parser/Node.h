#ifndef NODE_H_
#define NODE_H_

#include <vector>
#include <memory>
using namespace std;

class Node;
class RangeNode;
class ConcatNode;
class MethodNode;
class AlterNode;
class RepeateNode;

typedef shared_ptr<Node> nodePtr;

/*
 * Node类作为接下来所有类的一个父类,起到了非常重要的作用
 */
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
	Node(type, const wstring&, nodePtr left = NULL, nodePtr right = NULL);
	virtual ~Node();  /* 一定要让父类的析构函数是虚函数 */
public:
	virtual void evaluate(IVisitor& v) = 0;
};

/*
 * RangeNode 用于表示一个范围,如a, [a-z]等.
 */
class RangeNode :public Node
{
public:
	RangeNode(const wstring& str) :
		Node(Node::range, str)
	{}
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

/*
 * ConcatNode 用于连接两个节点.
 */
class ConcatNode : public Node 
{
public:
	ConcatNode(const wstring&, nodePtr&, nodePtr&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

/* 
 * alternative 表示两个节点的选择
 */
class AlterNode : public Node
{
public:
	AlterNode(const wstring&, nodePtr&, nodePtr&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

/*
 * RepeateNode 用于表示重复,包括*, ?, +,以及{m, n}这几种形式
 */
class RepeateNode : public Node 
{
public:
	RepeateNode(const wstring& str, nodePtr& sub) :
		Node(Node::repeate, str, sub)
	{}
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

/*
 * MethodNode是一个通用的方法节点.
 */
class MethodNode : public Node 
{
public:
	MethodNode(const wstring&, nodePtr&);
	MethodNode(const wstring&);
	void evaluate(IVisitor& v) {
		v.visit(*this);
	}
};

#endif // !NODE_H