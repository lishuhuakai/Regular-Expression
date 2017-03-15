#include "EpsNfa.h"
#include "SyntaxTree.h"

EpsNfa::EpsNfa(SyntaxTree& tree) :
	tree_(tree),
	charMap_(tree.allCharSets_)
{

}

statusPtr EpsNfa::makeNewNode() {
	statusPtr s = make_shared<Status>();
	allStatus_.push_back(s);
	return s;
}

edgePtr EpsNfa::makeNewEdge(const statusPtr& from, const statusPtr& to) {
	edgePtr e = make_shared<Edge>(from, to);
	allEdges_.push_back(e);
	return e;
}

void EpsNfa::visit(RangeNode& ptr) {
	if (ptr.index_.size() == 0) {
		if (ptr.includeAll_) charMap_.getAllLabels(ptr.index_);
		else charMap_.getLabels(ptr.content_, ptr.notIncludeContent_, ptr.index_);
	}
	statusPtr start = makeNewNode();
	statusPtr end = makeNewNode();
	edgePtr edge = makeNewEdge(start, end);
	start->outEdges.push_back(edge);
	end->inEdges.push_back(edge);
	edge->matchContent = ptr.index_;
	result_ = make_shared<SmallStatMachine>(start, end);
}

void EpsNfa::visit(RepeateNode& ptr) {
	statusPtr start = makeNewNode();
	statusPtr end = makeNewNode();
	ptr.left_->evaluate(*this);
	ssPtr s1 = result_; // 得到运算的结果
	edgePtr e1 = makeNewEdge(start, s1->start);
	start->outEdges.push_back(e1);
	s1->start->inEdges.push_back(e1);

	if (ptr.minTimes_ == 0) { // 最少是0次的话,直接从首部指向尾部
		edgePtr e1 = make_shared<Edge>(start, end);
		start->outEdges.push_back(e1);
		end->inEdges.push_back(e1);
	}
	int times = ptr.minTimes_ - 2; // 应该前面的有n-1个,已经做了1个,所以还剩n-2个
	for (int i = 0; i < times; ++i) { // 至少要重复n次
		
		ptr.left_->evaluate(*this);
		ssPtr s2 = result_;
		
		edgePtr edge = makeNewEdge(s1->end, s2->start);
		s1->end->outEdges.push_back(edge);
		s2->start->inEdges.push_back(edge);
		s2.swap(s1);
	}
	times = ptr.maxTimes_ - ptr.minTimes_ + 1;
	if (ptr.maxTimes_ != -1) { // 等于-1的话,代表无限
		for (int i = 0; i < times; ++i) {
			ptr.left_->evaluate(*this);
			ssPtr s2 = result_;

			edgePtr edge1 = makeNewEdge(s1->end, s2->start);
			s1->end->outEdges.push_back(edge1);
			s2->start->inEdges.push_back(edge1);
			edgePtr edge2 = makeNewEdge(s2->end, end);
			s2->end->outEdges.push_back(edge2);
			end->inEdges.push_back(edge2);
			s2.swap(s1);
		}
	}
	else // 这里代表要无限重复
 	{
		assert(0); // 暂时不支持
		//edgePtr edge = makeNewEdge(s1->end, s1->start);
		//s1->end->outEdges.push_back(edge);
		//s1->start->inEdges.push_back(edge);
	}
	result_ = make_shared<SmallStatMachine>(start, end);
	// 接下来是最多要重复的次数

	//if (ptr->pattern_ == L"*") { // 0个或者多个
	//	
	//	edgePtr e1 = makeNewEdge(start, end);
	//	start->outEdges.push_back(e1); end->inEdges.push_back(e1);
	//	edgePtr e2 = makeNewEdge(end, s1->start);
	//	end->outEdges.push_back(e2); s1->start->inEdges.push_back(e2);
	//	edgePtr e3 = makeNewEdge(s1->end, end);
	//	s1->end->outEdges.push_back(e3); end->inEdges.push_back(e3);
	//	return make_shared<SmallStatMachine>(start, end);
	//}
	//else if (ptr->pattern_ == L"+") { // 重复0次或者1次
	//	edgePtr e1 = makeNewEdge(s1->start, s1->end);
	//	s1->start->outEdges.push_back(e1);
	//	s1->end->inEdges.push_back(e1);
	//	return s1;
	//}
	//else
	//	assert(0);  // 暂时还不支持别的重复,to do

}

void EpsNfa::visit(MethodNode&) {
	// todo
	assert(0);
}

void EpsNfa::visit(ConcatNode& ptr) {
	ptr.left_->evaluate(*this);
	ssPtr lss = result_;
	ptr.right_->evaluate(*this);
	ssPtr rss = result_;

	edgePtr edge = makeNewEdge(lss->end, rss->start); // edge的matchContent为空,代表eps
	lss->end->outEdges.push_back(edge);
	rss->start->inEdges.push_back(edge);
	result_ = make_shared<SmallStatMachine>(lss->start, rss->end);
}

void EpsNfa::visit(AlterNode& ptr) {
	statusPtr start = makeNewNode();
	statusPtr end = makeNewNode();
	ptr.left_->evaluate(*this);
	ssPtr lss = result_;
	ptr.right_->evaluate(*this);
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
	result_->end->finalStatus = true;
	return result_;
}

void EpsNfa::cleanEnv() {
	for (auto edge : allEdges_) {
		edgePtr ePtr(edge.lock());
		if (ePtr) {
			removeElement(ePtr->from->outEdges, ePtr);
			removeElement(ePtr->to->inEdges, ePtr);
			ePtr->from.reset();
			ePtr->to.reset();
		}
	}
	allEdges_.clear();
	for (auto stat : allStatus_) {
		statusPtr sPtr(stat.lock());
		if (sPtr) {
			sPtr->outEdges.clear();
			sPtr->inEdges.clear();
		}
	}
	allStatus_.clear();
}