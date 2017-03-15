#pragma once
#include "Status.h"
#include "Node.h"
#include "CharMap.h"
class SyntaxTree;


class EpsNfa: public Node::IVisitor {
public:
	EpsNfa(SyntaxTree& tree);
public:
	ssPtr buildEpsNfa();
	statusPtr makeNewNode(); 
	edgePtr makeNewEdge(const statusPtr& from, const statusPtr& to);
	void cleanEnv();
private:
	void visit(RepeateNode&);
	void visit(ConcatNode&);
	void visit(MethodNode&);
	void visit(AlterNode&);
	void visit(RangeNode&);
public:
	ssPtr result_;
	CharMap charMap_;
	SyntaxTree& tree_;
	vector<statusWeakPtr> allStatus_;
	vector<edgeWeakPtr> allEdges_;
};
