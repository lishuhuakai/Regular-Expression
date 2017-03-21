#include "EpsNfa.h"
#include "SyntaxTree.h"
#include <functional>

EpsNfa::EpsNfa(SyntaxTree& tree) :
	tree_(tree),
	charMap_(tree.allCharSets_)
{

}


statusPtr EpsNfa::makeNewNode() {
	int key = nextLabel();
	statusPtr s(new Status(key), std::bind(&EpsNfa::deleteStatusPtr, this, placeholders::_1));
	allStatus_[key] = s;
	return s;
}

edgePtr EpsNfa::makeNewEdge(const statusPtr& from, const statusPtr& to) {
	int key = nextLabel2();
	edgePtr e(new Edge(key, from, to), bind(&EpsNfa::deleteEdgePtr, this, placeholders::_1));
	allEdges_[key] = e;
	return e;
}

/*
 * makeNewEdge 下面这个重载版本主要用来构建功能边.
 */
edgePtr EpsNfa::makeNewEdge(Edge::type type, const statusPtr& from, const statusPtr& to) {
	int key = nextLabel2();
	edgePtr e(new Edge(key, type, from, to), bind(&EpsNfa::deleteEdgePtr, this, placeholders::_1));
	allEdges_[key] = e;
	return e;
}

void EpsNfa::visit(RangeNode& node) {
	if (node.index_.size() == 0) {
		if (node.includeAll_) charMap_.getAllLabels(node.index_);
		else charMap_.getLabels(node.content_, node.notIncludeContent_, node.index_);
	}
	statusPtr start = makeNewNode();
	statusPtr end = makeNewNode();
	edgePtr edge = makeNewEdge(start, end);
	start->outEdges.push_back(edge);
	end->inEdges.push_back(edge);
	edge->matchContent = node.index_;
	result_ = make_shared<SmallStatMachine>(start, end);
}

ssPtr EpsNfa::buildRepeate(RepeateNode& node, statusPtr& prevSSEnd, statusPtr& mid) {
	node.left_->evaluate(*this);
	ssPtr s2 = result_;

	edgePtr e1 = makeNewEdge(prevSSEnd, s2->start);
	edgePtr e2 = makeNewEdge(s2->end, mid);
	if (node.greedy_) { /* 贪婪模式 */
		prevSSEnd->outEdges.insert(prevSSEnd->outEdges.begin(), e1); /* 在最前面插入 */
		e1->highPriority = true;
		s2->start->inEdges.push_back(e1);
		s2->end->outEdges.push_back(e2); /* 优先级一般 */
		mid->inEdges.push_back(e2);
	}
	else {
		prevSSEnd->outEdges.push_back(e1); /* 优先级一般 */
		s2->start->inEdges.push_back(e1);
		s2->end->outEdges.insert(s2->end->outEdges.begin(), e2); /* 优先级高 */
		e2->highPriority = true;
		mid->inEdges.push_back(e2);
	}
	return s2;
}

void EpsNfa::visit(RepeateNode& node) {
	statusPtr start = makeNewNode();
	statusPtr mid = makeNewNode();
	statusPtr end = makeNewNode();
	ssPtr s1;
	statusPtr loop_end = mid; /* 用于记录loop边的终点,默认指向mid */
	statusPtr prevSSEnd = start;

	/* 下面处理最少的重复次数 */
	int times = node.minTimes_ - 1; /* 创建前置节点的个数 */
	for (int i = 1; i <= times; ++i) { /* 至少要重复n次 */
		node.left_->evaluate(*this);
		ssPtr s1 = result_;
		if (i == 1) /* 记录下loop边的终点 */
			loop_end = s1->start;

		edgePtr edge = makeNewEdge(prevSSEnd, s1->start);
		prevSSEnd->outEdges.push_back(edge);
		s1->start->inEdges.push_back(edge);
		prevSSEnd = s1->end;
	}

	if (node.minTimes_ != 0) {
		prevSSEnd = makeNewNode();
		s1 = buildRepeate(node, prevSSEnd, mid);
		loop_end = prevSSEnd;
	}

	/* 下面处理最大的重复次数 */
	if (!node.infinity_) { 
		int times = node.maxTimes_ - node.minTimes_;
		for (int i = 1; i <= times; ++i) {
			s1 = buildRepeate(node, s1->end, mid);
		}
		/* end边 */
		edgePtr e1 = makeNewEdge(Edge::end, mid, end);
		mid->outEdges.push_back(e1); end->inEdges.push_back(e1);
	}
	else /* 这里代表要无限重复 */
 	{
		node.left_->evaluate(*this);
		s1 = result_;

#define TRANSFER(priority)					\
do {											\
	for (auto edge : s1->start->outEdges) {		\
		edge->from = mid;						\
		if (priority) edge->highPriority = true;\
		mid->outEdges.push_back(edge);			\
	}											\
	s1->start->outEdges.clear();				\
} while (0)										\

		edgePtr e1 = makeNewEdge(Edge::end, mid, end);
		if (node.greedy_) { /* 贪心的走法 */
			TRANSFER(true); /* 将s1.start的出边全部转移到mid上 */
			mid->outEdges.push_back(e1);
		}
		else { /* 非贪心的走法 */
			mid->outEdges.push_back(e1); /* end边优先 */
			e1->highPriority = true;
			TRANSFER(false); /* 开始将s1.start的出边全部转移到mid上 */
		}
		/* 将s1->end的入边的内容全部转移到mid中 */
		for (auto edge : s1->end->inEdges) {
		edge->to = mid;						
		mid->inEdges.push_back(edge);		
		}										
		s1->end->inEdges.clear();
	
		end->inEdges.push_back(e1);
	}
	/* 最后添加一条loop边 */
	edgePtr e1 = makeNewEdge(Edge::loop, start, loop_end);
	start->outEdges.push_back(e1);  loop_end->inEdges.push_back(e1);
	result_ = make_shared<SmallStatMachine>(start, end);
}

void EpsNfa::visit(MethodNode& node) {
	statusPtr start = makeNewNode();
	statusPtr end = makeNewNode();
	switch (node.methodtype_) {
	case MethodNode::anonymousCatch: { /* 匿名捕获 */
		node.left_->evaluate(*this);
		ssPtr s = result_;
		edgePtr e1 = makeNewEdge(Edge::capture, start, s->start);
		start->outEdges.push_back(e1); s->start->inEdges.push_back(e1);
		edgePtr e2 = makeNewEdge(Edge::end, s->end, end);
		s->end->outEdges.push_back(e2), end->inEdges.push_back(e2);
		break;
	}
	case MethodNode::Catch: { /* 命名捕获 */
		node.left_->evaluate(*this);
		ssPtr s = result_;
		edgePtr e1 = makeNewEdge(Edge::storage, start, s->start);
		e1->name = node.name_;
		start->outEdges.push_back(e1); s->start->inEdges.push_back(e1);
		edgePtr e2 = makeNewEdge(Edge::end, s->end, end);
		s->end->outEdges.push_back(e2), end->inEdges.push_back(e2);
		break;
	}
	case MethodNode::forwardMatch: { /* 正向预查 */
		// todo
		assert(0);
	}
	case MethodNode::backwardMatch: {
		// todo
		assert(0);
	}
	case MethodNode::Check: { /* 检查 */
		edgePtr e1 = makeNewEdge(Edge::check, start, end);
		e1->name = node.name_; /* 记录下check的名称 */
		start->outEdges.push_back(e1); end->inEdges.push_back(e1);
		break;
	}
	default:
		break;
	}
	result_ = make_shared<SmallStatMachine>(start, end);
}

void EpsNfa::visit(ConcatNode& node) {
	node.left_->evaluate(*this);
	ssPtr lss = result_;
	node.right_->evaluate(*this);
	ssPtr rss = result_;

	edgePtr edge = makeNewEdge(lss->end, rss->start); /* edge的matchContent为空,代表eps(ε) */
	lss->end->outEdges.push_back(edge);
	rss->start->inEdges.push_back(edge);
	result_ = make_shared<SmallStatMachine>(lss->start, rss->end);
}

void EpsNfa::visit(AlterNode& node) {
	statusPtr start = makeNewNode();
	statusPtr end = makeNewNode();
	node.left_->evaluate(*this);
	ssPtr lss = result_;
	node.right_->evaluate(*this);
	ssPtr rss = result_;
	edgePtr e1 = makeNewEdge(start, lss->start);
	edgePtr e2 = makeNewEdge(start, rss->start);
	edgePtr e3 = makeNewEdge(lss->end, end);
	edgePtr e4 = makeNewEdge(rss->end, end);
	start->outEdges.push_back(e1); lss->start->inEdges.push_back(e1);
	start->outEdges.push_back(e2); rss->start->inEdges.push_back(e2);
	lss->end->outEdges.push_back(e3); end->inEdges.push_back(e3);
	rss->end->outEdges.push_back(e4); end->inEdges.push_back(e4);
	result_ = make_shared<SmallStatMachine>(start, end);
}

ssPtr EpsNfa::buildEpsNfa() {
	nodePtr t = tree_.buildSyntaxTree();
	t->evaluate(*this);
	//printEdges();
	result_->end->finalStatus = true;
	return result_;
}

void EpsNfa::cleanEnv() {
	clearAllEdges();
	for (auto pr = allStatus_.begin(); pr != allStatus_.end(); ) {
		auto oldPr = pr++;
		statusPtr sPtr((*oldPr).second.lock());
		if (sPtr) {
			sPtr->outEdges.clear();
			sPtr->inEdges.clear();
		}
	}
	assert(allStatus_.size() == 0);
	//allStatus_.clear();
}


/*
 * clearAllEdges 清除allEdges里面的所有边,不出意外的话,这些边会立马析构掉.
 * 当然,如果你还持有该边的指针,那自然不会析构.
 */
void EpsNfa::clearAllEdges() {
	for (auto pr = allEdges_.begin(); pr != allEdges_.end(); ) {
		auto oldPr = pr++;
		edgePtr ePtr((*oldPr).second.lock());
		if (ePtr) {
			removeElement(ePtr->from->outEdges, ePtr);
			removeElement(ePtr->to->inEdges, ePtr);
			ePtr->from.reset();
			ePtr->to.reset();
		}
	}
	assert(allEdges_.size() == 0);
	//allEdges_.clear();
}

void EpsNfa::printEdges() {
	for (auto pr : allEdges_) {
		auto edge(pr.second.lock());
		if (edge) {
			cout << edge->from->label << "==>" << edge->to->label;
			string c = edge->type2String();
			if (edge->isEps())
				c = "eps";
			cout << "[" << c.c_str() << "]" << endl;
		}
	}
}



