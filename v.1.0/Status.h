#pragma once
#include "common.h"
#include <memory>
using namespace std;

struct Edge;
struct Status;
typedef shared_ptr<Edge> edgePtr;
typedef shared_ptr<Status> statusPtr;
typedef weak_ptr<Edge> edgeWeakPtr;
typedef weak_ptr<Status> statusWeakPtr;

struct Status {
	int label;
	vector<edgePtr> inEdges; // 进边
	vector<edgePtr> outEdges; // 出边
	bool finalStatus;
	Status() :
		finalStatus(false),
		label(nextLabel())
	{}
};

/*
 * Edge 遍历树生成图的过程中少不了边.
 */
struct Edge {
	vector<int> matchContent;
	statusPtr from;
	statusPtr to;
	Edge(const statusPtr& f, const statusPtr& t) :
		from(f),
		to(t)
	{}
};

/*
 * SmallStatMachine 这个结构体主要用于记录每一棵子树生成的图的起始和终止节点.
 */
struct SmallStatMachine {
	statusPtr start;
	statusPtr end;

	SmallStatMachine(statusPtr& s, statusPtr& e) :
		start(s),
		end(e)
	{}

	SmallStatMachine() {}
};
typedef shared_ptr<SmallStatMachine> ssPtr;
