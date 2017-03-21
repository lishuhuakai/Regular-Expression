#ifndef NFA_H_
#define NFA_H_

#include "common.h"
#include "EpsNfa.h"

class Match;

class Nfa
{
	typedef vector<Item> collector;
	typedef shared_ptr<collector> collectorPtr;
public:
	Nfa(EpsNfa& epsNfa);
	~Nfa();
public:
	friend class Match;
	int char2Index(wchar_t);
	statusPtr buildNfa(); /* 消除ε,构建NFA */
	void clearEnv(); /* 清理环境 */
	int numOfUniqueSet() {
		return epsNfa_.charMap_.numOfUniqueSet();
	}
	statusPtr compressNfa(); /* 对Nfa进行压缩 */
private:
	EpsNfa& epsNfa_;
	edgesCollector allNewEdges_; /* 用于记录所有的新添加的边 */
	set<statusPtr> finalStatus_;  /* 用于记录最终的状态 */
private:
	void deleteEdgePtr(Edge *e) {
		allNewEdges_.erase(e->label);
		delete e;
	}
	edgePtr makeCompressedEdge(Edge::type, const statusPtr&, const statusPtr&);
	edgePtr copyCompressedEdge(edgePtr&);
	void epsClosure(statusPtr&, set<statusPtr>&, set<edgePtr>&); /* 用于寻找一个status的ε闭包 */
	void getEffectEdges(set<statusPtr>&, set<edgePtr>&); /* 寻找闭包所有的有效边 */
	void buildNewMethodEdges(statusPtr&, statusPtr&, 
		collectorPtr&, vector<edgePtr>&, set<statusPtr>&);
	void copyNewEdge(statusPtr&, edgePtr&, vector<edgePtr>&);  /* 拷贝新边 */
	void printNewEdges(); /* 输出新边的信息 */
	void printFinalStat();
	void clearAllNewEdges();
};

#endif //!NFA_H_