#ifndef EPSNFA_H_
#define EPSNFA_H_

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
	edgePtr makeNewEdge(Edge::type type, const statusPtr& from, const statusPtr& to);
	void cleanEnv();
	void printEdges();
	void clearAllEdges();
private:
	void visit(RepeateNode&);
	void visit(ConcatNode&);
	void visit(MethodNode&);
	void visit(AlterNode&);
	void visit(RangeNode&);
	ssPtr buildRepeate(RepeateNode&, statusPtr&, statusPtr&);
	void deleteStatusPtr(Status * s) {
		allStatus_.erase(s->label);
		delete s;
	}
	void deleteEdgePtr(Edge *e) {
		allEdges_.erase(e->label);
		delete e;
	}
public:
	ssPtr result_;
	CharMap charMap_;
	SyntaxTree& tree_;
	statusCollector allStatus_;
	edgesCollector allEdges_;
};

#endif // !EPSNFA_H_
