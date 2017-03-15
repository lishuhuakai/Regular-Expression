#ifndef NFA_H_
#define NFA_H_

#include "common.h"
#include "EpsNfa.h"

class Nfa
{
public:
	Nfa(EpsNfa& epsNfa);
	~Nfa();
public:
	int char2Index(wchar_t);
	statusPtr buildNfa(); // 消除ε,构建NFA
	void clearEnv(); // 清理环境
	int numOfUniqueSet() {
		return epsNfa_.charMap_.numOfUniqueSet();
	}
private:
	EpsNfa& epsNfa_;
	void epsClosure(statusPtr&, set<statusPtr>&, set<edgePtr>&); // 用于寻找一个status的ε闭包
	void getEffectEdges(set<statusPtr>&, set<edgePtr>&); // 寻找闭包所有的有效边
};

#endif //!NFA_H_