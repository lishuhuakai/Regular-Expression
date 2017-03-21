#ifndef STATUS_H_
#define STATUS_H_

#include "common.h"
#include <memory>
using namespace std;

struct Edge;
struct Item;
struct Status;
typedef shared_ptr<Edge> edgePtr;
typedef shared_ptr<Status> statusPtr;
typedef weak_ptr<Edge> edgeWeakPtr;
typedef weak_ptr<Status> statusWeakPtr;
typedef map<int, edgeWeakPtr> edgesCollector;
typedef map<int, statusWeakPtr> statusCollector;

struct Status {
	int label;
	vector<edgePtr> inEdges; /* 进边 */
	vector<edgePtr> outEdges; /* 出边 */
	bool finalStatus;
	Status(int id) :
		finalStatus(false),
		label(id)
	{}
};



/*
 * Edge 遍历树生成图的过程中少不了边.
 */
struct Edge {
	int label;
	vector<int> matchContent;
	enum type { 
		normal=0,
		/* 下面全部属于功能边 */
		head, tail, check, loop, capture,
		storage, end, positive, negative,
		compressed
	};
	type tp;
	wstring name;
	statusPtr from;
	statusPtr to;
	bool highPriority;  /* 是否高优先级 */

	vector<Item> items; /* 用于收集功能边的内容 */
	bool isEps() { /* 判断边是否为ε边 */
		return ((tp == normal || tp == compressed) && (matchContent.size() == 0));
	}
	bool isConsumedChar() { /* 判断edge是否消耗了字符 */
		return ((tp == check) || (matchContent.size() != 0));
	}
	string type2String();
	static string type2String(Edge::type);

	Edge(int id, const statusPtr& f, const statusPtr& t) :
		from(f), to(t), tp(normal), label(id), highPriority(false)
	{}

	Edge(int id, type tp, const statusPtr& f, const statusPtr& t) :
		from(f), to(t), tp(tp),	label(id), highPriority(false)
	{}

	Edge(const Edge& e) {
		label = e.label;
		tp = e.tp;
		highPriority = e.highPriority;
		name = e.name;
		items = e.items;
		matchContent = e.matchContent;
		from = e.from;
		to = e.to;
	}
};


struct Item {
	Edge::type tp;
	wstring name;  /* 用于记录正向匹配或者命名检查的名称 */
	
	explicit Item(Edge::type tp, wstring& str) :
		tp(tp), name(str)
	{}
	Item(const Item& t) {
		tp = t.tp;
		name = t.name;
	}
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

#endif // !STATUS_H_