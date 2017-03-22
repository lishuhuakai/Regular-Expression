#include "Node.h"

Node::Node(type t,const wstring& pattern, nodePtr left, nodePtr right) :
	type_(t),
	pattern_(pattern),
	left_(left),
	right_(right)
{
}


Node::~Node()
{

}




ConcatNode::ConcatNode(const wstring& pattern, nodePtr& l, nodePtr& r) :
	Node(concat, pattern, l, r)
{

}


AlterNode::AlterNode(const wstring& pattern, nodePtr& l, nodePtr& r) :
	Node(alternative, pattern, l, r)
{

}



MethodNode::MethodNode(const wstring& pattern):
	Node(method, pattern)
{
}

MethodNode::MethodNode(const wstring& pattern, nodePtr& sub) :
	Node(method, pattern, sub)
{
}