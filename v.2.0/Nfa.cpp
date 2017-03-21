#include "Nfa.h"
#include "EpsNfa.h"


Nfa::Nfa(EpsNfa& epsNfa) :
	epsNfa_(epsNfa)
{
}

Nfa::~Nfa()
{
}

/*
 * getEffectEdges 用于获取所有的有效边.
 */
void Nfa::getEffectEdges(set<statusPtr>& closure, set<edgePtr>& edges) {
	for (auto stat : closure) {
		for (auto edge : stat->outEdges) {
			if (!edge->isEps())
				edges.insert(edge);
		}
	}
}

/*
 * epsClosure  求s节点的ε闭包,将结果放入closure之中.
 */
void Nfa::epsClosure(statusPtr &s, set<statusPtr>& closure, set<edgePtr>& allEpsEdges) {
	for (auto i : s->outEdges) { /* 遍历出边 */
		if (i->isEps()) {
			closure.insert(i->to);
			allEpsEdges.insert(i); /* 将所有的ε边全部收集起来,方便一起删除 */
			epsClosure(i->to, closure, allEpsEdges);
		}
	}
}

statusPtr Nfa::buildNfa() {
	ssPtr s = epsNfa_.buildEpsNfa();
	cout << "-----------epsNfa------------" << endl;
	epsNfa_.printEdges();
	finalStatus_.insert(s->end);
	/* 第一步,得到所有的有效状态 */
	set<statusPtr> effectStatus;
	set<statusPtr> notEffectStatus;  /* 非有效的状态 */
	set<edgePtr> allEpsEdges;  /* 用于存储所有的ε边 */
	for (auto pr : epsNfa_.allStatus_) {
		statusPtr stat(pr.second.lock());
		if (stat) {
			bool effective = false;
			for (auto edge : stat->inEdges) {
				if (!edge->isEps()) {
					effectStatus.insert(stat);
					effective = true;
					break;
				}
			}
			if (!effective)
				notEffectStatus.insert(stat);
		}
		else {
			//	assert(0);
			cout << "Some status deconstructed!" << endl;
		}
	}
	effectStatus.insert(s->start); /* 起始状态是一个有效状态 */
	notEffectStatus.erase(s->start); /* 有可能将起始状态误判为了无效状态 */

	/* 第二步,对于每一个有效的状态,找到其的eps闭包 */
	vector<edgePtr> newAddedEdges;  /* 用于记录新添加的边 */
	set<edgePtr> effectEdges;
	set<edgePtr> backupEdges;
	for (statusPtr stat : effectStatus) {
		set<statusPtr> closure;
		epsClosure(stat, closure, allEpsEdges);
		closure.erase(stat);
		/* 闭包中包含了结束状态的有效状态是新的NFA的结束状态,结束状态可能不止一个. */
		if (closure.find(s->end) != closure.end()) { /* 构建的epsNfa中,最终状态只有一个,那就是s->end */
			finalStatus_.insert(stat); /* 记录下结束状态 */
			stat->finalStatus = true;
		}
		/* 然后要从closure出发找到所有的边 */
		getEffectEdges(closure, effectEdges);
		for (auto edge : effectEdges) {
			backupEdges.insert(edge); /* 这些边虽然不是ε边,但是最终还是要删除的 */
			/* 这里正确的做法是复制一条边 */
			edgePtr e1;
			if (closure.find(edge->to) != closure.end()) {
				/* 如果该边指向的status属于stat的闭包,那么这条新建的边应该是从stat指向stat */
				e1 = epsNfa_.makeNewEdge(edge->tp, stat, stat);
			}
			else {
				e1 = epsNfa_.makeNewEdge(edge->tp, stat, edge->to);
			}
			if (edge->highPriority) e1->highPriority = true; /* 记录下高优先级 */
			newAddedEdges.push_back(e1);
			// 这里必须要提一点,那就是如果此时就在某个状态的outEdges里面添加新的边,
			// 如果该边也是有效边,而且该状态的检查在这个状态之后,会出现一些bug.所以
			// 不直接在这里将边加入到状态的的出边和入边列表中,等待检查做完之后,统一添加
			//stat->outEdges.push_back(e1);
			//edge->to->inEdges.push_back(e1);
			e1->matchContent = edge->matchContent; /* 拷贝内容 */
			e1->name = edge->name;
		}
		effectEdges.clear();
	}
	/* 统一将边添加到对应状态的出边和入边中 */
	for (auto edge : newAddedEdges) {
		if (edge->highPriority) { /* 一般来说,所有的状态最多只有一个优先级比较高的出边 */
			edge->from->outEdges.insert(edge->from->outEdges.begin(), edge);
		}
		else edge->from->outEdges.push_back(edge);
		edge->to->inEdges.push_back(edge);
	}
	newAddedEdges.clear();
	/* 第三步,删除所有的ε边和无效状态 */
	/* 所有从有效状态出发的非ε边都不能删除,虽然从图上看,所有的边都被删除了,但是不排除特殊情况,因此,还是谨慎一点 */
	for (auto edge : backupEdges) { /* 这些边已经被拷贝了,可以删除 */
		removeElement(edge->from->outEdges, edge);
		removeElement(edge->to->inEdges, edge);
		edge->from.reset();
		edge->to.reset();
	}
	backupEdges.clear();
	/* 删除所有的ε边 */
	for (auto edge : allEpsEdges) {
		removeElement(edge->from->outEdges, edge);
		removeElement(edge->to->inEdges, edge);
		edge->from.reset();
		edge->to.reset();
	}
	allEpsEdges.clear();
	/* 删除所有的非有效状态 */
	for (auto stat : notEffectStatus) {
		stat->outEdges.clear();
		stat->inEdges.clear();
	}
	notEffectStatus.clear();
	return s->start; /* 开始节点始终是开始节点 */
}


int Nfa::char2Index(wchar_t ch) {
	return epsNfa_.charMap_.char2Index(ch);
}

/*
 * clearEnv 这个函数主要是为了避免内存泄露,要将所有的Node以及Edge全部析构掉.
 */
void Nfa::clearEnv() {
	epsNfa_.cleanEnv();
	clearAllNewEdges(); /* 将新添加的边全部删除掉 */
}

statusPtr Nfa::compressNfa() {
	statusPtr t = buildNfa();
	cout << "--------------Nfa---------------" << endl;
	epsNfa_.printEdges(); /* 先输出边看一下是否正确 */
	printFinalStat();
	set<statusPtr> marked;
	set<statusPtr> notMarked;
	/* 第一步,找到这样的状态,它的所有入边都不消耗字符 */
	for (auto pr : epsNfa_.allStatus_) {
		auto status(pr.second.lock());
		if (status) {
			bool allNotConsumed = true;
			for (auto edge : status->inEdges) {
				if (edge->isConsumedChar()) { /* 消耗了字符 */
					allNotConsumed = false;
					break;
				}
			}
			if (allNotConsumed) marked.insert(status);
			else notMarked.insert(status);
		}
	}
	marked.erase(t); /* 移除起始状态 */
	notMarked.insert(t);
	for (auto fn : finalStatus_) {
		marked.erase(fn); /* 移除终止状态 */
		notMarked.insert(fn);
	}
	vector<edgePtr> collectors;  /* 用于存储新添加的边 */
	set<statusPtr> beforeFinalStatus; /* 用于记录在有边有edge到final状态的状态 */
	/* 虽然非常繁琐,但是我觉得做这一步还是很重要的. */
	for (auto fsPtr : finalStatus_) {
		if (fsPtr) {
			for (auto edge : fsPtr->inEdges) {
				beforeFinalStatus.insert(edge->from);
			}
		}
	}
	/* 第二步,遍历所有的未标记的状态,准备建立新边 */
	for (auto status : notMarked) {
		/* 准备开始递归 */
		collectorPtr cc = make_shared<collector>(); /* 收集器 */
		buildNewMethodEdges(status, status, cc, collectors, beforeFinalStatus);
	}

	for (auto edge : collectors) {
		edge->from->outEdges.push_back(edge);
		edge->to->inEdges.push_back(edge);
	}

	/* 到了这里,所有的新边就已经构建完成了,接下来是删除所有的旧边已经别标记的状态 */

	/* 清理掉旧边 */
	epsNfa_.clearAllEdges();
	/* 清理掉被标记的状态 */
	for (auto status : marked) {
		status->outEdges.clear();
		status->inEdges.clear();
	}
	printNewEdges();
	return t;
}

void Nfa::buildNewMethodEdges(statusPtr& from, statusPtr& stat, collectorPtr& cc,
	vector<edgePtr>& collectors, set<statusPtr>& beforeFinalStatus) {
	collectorPtr newCc;
	if (stat->outEdges.size() > 1)
		newCc = make_shared<collector>(*cc); /* 复制一份数据 */
	size_t i = 0;
	for (; i < stat->outEdges.size(); ++i) {
		if (i != 0) { /* 运行到了这里表示存在多条边 */
			cc = newCc;
			if (i != stat->outEdges.size() - 1) /* 最后一条边不用重新拷贝数据 */
				newCc = make_shared<collector>(*newCc);
		}
		if (!stat->outEdges[i]->isConsumedChar()) { /* 不消耗字符 */
			cc->push_back(Item(stat->outEdges[i]->tp, stat->outEdges[i]->name));
			buildNewMethodEdges(from, stat->outEdges[i]->to, cc, collectors, beforeFinalStatus);
		}
		else { /* 消耗了字符 */
			bool shouldCopyEdge = false;
			if (beforeFinalStatus.find(stat->outEdges[i]->to) != beforeFinalStatus.end())
				shouldCopyEdge = true;
			edgePtr e = makeCompressedEdge(Edge::compressed, from, stat->outEdges[i]->to);
			if (stat->outEdges[i]->tp == Edge::check) /* 如果是check,依旧要压入 */
				cc->push_back(Item(stat->outEdges[i]->tp, stat->outEdges[i]->name));
			e->items.swap(*cc);
			e->matchContent = stat->outEdges[i]->matchContent;
			collectors.push_back(e);
			if (shouldCopyEdge) { /* 这个状态有不消耗字符的边连着final状态,需要拷贝一条新边到final */
				copyNewEdge(stat->outEdges[i]->to, e, collectors);
			}
		}
	}
	/*
	 * 运行到了这里的话,表示要么表示已经没有出边了,这一般是到了终点了,要么 
	 */
	if (finalStatus_.find(from) == finalStatus_.end() && i == 0) { /* 没有了出边,并且起始点不是终点 */
		edgePtr e = makeCompressedEdge(Edge::compressed, from, stat);
		e->items.swap(*cc);
		collectors.push_back(e);
	}
}

void Nfa::copyNewEdge(statusPtr& from, edgePtr& edge, vector<edgePtr>& collectors) {
	for (auto e : from->outEdges) {
		if (finalStatus_.find(e->to) != finalStatus_.end()) { /* 寻找到了对应的边 */
			/* 接下来要添加新的边 */
			edgePtr e1 = copyCompressedEdge(edge); /* 拷贝新边 */
			e1->to = e->to; /* 这条边指向final状态 */
			e1->items.push_back(Item(e->tp, e->name)); /* 添加功能边的内容 */
			collectors.push_back(e1);
		}
	}
}


/*
 * makeNewEdge 构建一条新边,需要构建新边的时候应该调用这个函数,方便后期的内存管理.
 */
edgePtr Nfa::makeCompressedEdge(Edge::type type, const statusPtr& from, const statusPtr& to) {
	int key = nextLabel2();
	edgePtr e(new Edge(key, type, from, to), bind(&Nfa::deleteEdgePtr, this, placeholders::_1));
	allNewEdges_[key] = e;
	return e;
}

/*
 * copyCompressedEdge 拷贝一条新边.
 */
edgePtr Nfa::copyCompressedEdge(edgePtr& edge) {
	int key = nextLabel2();
	edgePtr e(new Edge(*edge), bind(&Nfa::deleteEdgePtr, this, placeholders::_1));
	e->label = key;
	allNewEdges_[key] = e;
	return e;
}

void Nfa::printNewEdges() {
	for (auto pr : allNewEdges_) {
		auto edge(pr.second.lock());
		if (edge->isEps())
			cout << "* ";
		else
			cout << "~  ";
		if (edge) {
			cout << edge->from->label << "-->" << edge->to->label << " ";
			for (auto item : edge->items) {
				cout << Edge::type2String(item.tp) << " ";
			}
			cout << endl;
		}
	}
}

void Nfa::clearAllNewEdges() {
	for (auto pr = allNewEdges_.begin(); pr != allNewEdges_.end(); ) {
		auto oldPr = pr++;
		edgePtr ePtr((*oldPr).second.lock());
		if (ePtr) {
			removeElement(ePtr->from->outEdges, ePtr);
			removeElement(ePtr->to->inEdges, ePtr);
			ePtr->from.reset();
			ePtr->to.reset();
		}
	}
}

void Nfa::printFinalStat() {
	cout << "Final Stats:" << endl;
	for (auto s : finalStatus_) {
		cout << s->label << endl;
	}
}
