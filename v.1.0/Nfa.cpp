#include "Nfa.h"
#include "EpsNfa.h"


Nfa::Nfa(EpsNfa& epsNfa) :
	epsNfa_(epsNfa)
{
}

Nfa::~Nfa()
{
}

void Nfa::getEffectEdges(set<statusPtr>& closure, set<edgePtr>& edges) { // 获取所有的有效边
	for (auto stat : closure) {
		for (auto edge : stat->outEdges) {
			if (edge->matchContent.size() != 0)
				edges.insert(edge);
		}
	}
}
void Nfa::epsClosure(statusPtr &s, set<statusPtr>& closure, set<edgePtr>& allEpsEdges) { // 求s节点的ε闭包,将结果放入closure之中
	for (auto i : s->outEdges) { // 遍历出边
		if (i->matchContent.size() == 0) { // 0代表这条边是一条ε边
			closure.insert(i->to);
			allEpsEdges.insert(i); // 将所有的ε边全部收集起来,方便一起删除
			epsClosure(i->to, closure, allEpsEdges);
		}
	}
}

statusPtr Nfa::buildNfa() {
	ssPtr s = epsNfa_.buildEpsNfa();
	// 第一步,得到所有的有效状态
	set<statusPtr> effectStatus;
	set<statusPtr> notEffectStatus;  // 非有效的状态
	set<edgePtr> allEpsEdges;  // 用于存储所有的ε边
	for (auto i : epsNfa_.allStatus_) {
		statusPtr stat(i.lock());
		if (stat) {
			bool effective = false;
			for (auto edge : stat->inEdges) {
				if (edge->matchContent.size() != 0) { // 如果入边匹配的元素不是ε(匹配的内容为0),那么该状态是有效状态
					effectStatus.insert(stat);
					effective = true;
					break;
				}
			}
			if (!effective)
				notEffectStatus.insert(stat);
		}
		else
			assert(0);
	}
	effectStatus.insert(s->start); // 起始状态是一个有效状态
	notEffectStatus.erase(s->start); // 有可能将起始状态误判为了无效状态

	// 第二步,对于每一个有效的状态,找到其的eps闭包
	set<edgePtr> effectEdges;
	set<edgePtr> backupEdges;
	for (statusPtr stat : effectStatus) {
		set<statusPtr> closure;
		epsClosure(stat, closure, allEpsEdges);
		closure.erase(stat);
		// 闭包中包含了结束状态的有效状态是新的NFA的结束状态,结束状态可能不止一个.
		if (closure.find(s->end) != closure.end())
			stat->finalStatus = true;
		// 然后要从closure出发找到所有的边
		getEffectEdges(closure, effectEdges);
		for (auto edge : effectEdges) {
			backupEdges.insert(edge); // 这些边虽然不是ε边,但是最终还是要删除的
									  // 这里正确的做法是复制一条边
			edgePtr e1 = epsNfa_.makeNewEdge(stat, edge->to);
			stat->outEdges.push_back(e1);
			edge->to->inEdges.push_back(e1);
			e1->matchContent = edge->matchContent; // 拷贝内容
		}
		effectEdges.clear();
	}
	// 第三步,删除所有的ε边和无效状态
	// 所有从有效状态出发的非ε边都不能删除,虽然从图上看,所有的边都被删除了,但是不排除特殊情况,因此,还是谨慎一点
	for (auto edge : backupEdges) { // 这些边已经被拷贝了,可以删除
		removeElement(edge->from->outEdges, edge);
		removeElement(edge->to->inEdges, edge);
		edge->from.reset();
		edge->to.reset();
	}
	backupEdges.clear();
	// 删除所有的ε边
	for (auto edge : allEpsEdges) {
		removeElement(edge->from->outEdges, edge);
		removeElement(edge->to->inEdges, edge);
		edge->from.reset();
		edge->to.reset();
	}
	allEpsEdges.clear();
	// 删除所有的非有效状态
	for (auto stat : notEffectStatus) {
		stat->outEdges.clear();
		stat->inEdges.clear();
	}
	notEffectStatus.clear();
	return s->start; // 开始节点始终是开始节点
}


int Nfa::char2Index(wchar_t ch) {
	return epsNfa_.charMap_.char2Index(ch);
}

void Nfa::clearEnv() { // 这个函数主要是为了避免内存泄露,要将所有的Node以及Edge全部析构掉
	epsNfa_.cleanEnv();
}