#include "Dfa.h"
#include "Status.h"
#include "Nfa.h"
#include "TwoDimArray.h"

typedef shared_ptr<set<statusPtr>> statusSetPtr;

struct NewStatus { // 我们需要构建一个新的数据结构
	statusSetPtr oldStatusSet; // 用于记录所有的状态的集合
	int label; // 每一个新状态都有一个标签
	NewStatus(statusSetPtr& ptr, int lb) :
		oldStatusSet(ptr),
		label(lb)
	{ }
};

typedef shared_ptr<NewStatus>  newStatusPtr;

struct Compare {
	bool operator()(const newStatusPtr& l1, const newStatusPtr& l2) const
	{
		return *((*l1).oldStatusSet) > *((*l2).oldStatusSet); // 直接比较两个指针比较的内容,而不是指针的值
	}
};

struct Record { // 用于记录一条变换
	int from;
	int to;
	int ch;
	bool finalTransfer;  // 记录是否是最后的转换
	Record(int from, int ch, int to, int finalTransfer) :
		from(from),
		to(to),
		ch(ch),
		finalTransfer(finalTransfer)
	{ }
};
typedef shared_ptr<Record> recordPtr;


Dfa::Dfa(Nfa& n) :
	nfa_(n),
	builted(false)
{
}


Dfa::~Dfa()
{
}

bool Dfa::match(const wstring& str) { // 测试是str是否匹配
	if (!builted) buildDFA();
	int nextStatus = 0;
	int currentStatus = 0;
	for (auto i : str) {
		int ch = nfa_.char2Index(i);
		if (ch == -1) return false;  // 返回-1代表正则式压根就不知道这个字符
		nextStatus = (*transferChart_)[currentStatus][nfa_.char2Index(i)];
		if (nextStatus == -1)
			return false;
		currentStatus = nextStatus;
	}
	return (finalStatus_.find(currentStatus) != finalStatus_.end());
}

void Dfa::buildDFA() {
	// 这个函数里面,直接用NFA来构造一张表吧,不用麻烦地来构造一张图了.
	// 最好能够动态地来构建一张表
	statusPtr start = nfa_.buildNfa();
	queue<newStatusPtr> l; // 队列l
	set<newStatusPtr, Compare> d; // 集合d
	vector<recordPtr> records; // 用于记录转换
	// 首先将起始状态放入l和d中
	statusSetPtr oldStatusSet = make_shared<set<statusPtr>>();
	oldStatusSet->insert(start);
	newStatusPtr startPtr = make_shared<NewStatus>(oldStatusSet, assignNewStatusLabel());
	d.insert(startPtr);
	l.push(startPtr);

	while (!l.empty()) {
		// 从队列中取出一个状态
		newStatusPtr s = l.front(); l.pop();
		// 计算从这个状态出发,所接收的字符的并集
		set<int> chars;
		getAllChars(s->oldStatusSet, chars);
		
		// 接下来开始一个三重循环
		for (auto ch : chars) { // 对于每一个字符
			statusSetPtr oldStatusSet = make_shared<set<statusPtr>>();
			bool finalTransfer = false;
			for (auto oldStatus : *s->oldStatusSet) { // 对于组成新节点的每个旧节点
				//newStatusPtr newStatus = make_shared<NewStatus>();
				for (auto edge : oldStatus->outEdges) { // 遍历每一条出边
					if (findElement(edge->matchContent, ch)) { // 如果能够匹配的话
						oldStatusSet->insert(edge->to);
						if (edge->to->finalStatus)
							finalTransfer = true;
					}
				}
			}
			// 运行到这里需要注意的一点是,oldStatusSet不可能为空
			assert(oldStatusSet->size() != 0);
			newStatusPtr newStatus = make_shared<NewStatus>(oldStatusSet, 0);
			auto pos = d.find(newStatus);
			int toLabel = 0;
			if (pos != d.end()) { // 这个状态已经存在了
				toLabel = (*pos)->label;
			}
			else {
				toLabel = assignNewStatusLabel();
				newStatus->label = toLabel;
				d.insert(newStatus);
				l.push(newStatus);
			}
			if (finalTransfer) finalStatus_.insert(toLabel); // 记录下最终的状态
			records.push_back(make_shared<Record>(s->label, ch, toLabel, finalTransfer));
		}
	}
	int numOfStatus = d.size();
	// 
	transferChart_ = make_shared<TwoDimArray>(numOfStatus, nfa_.numOfUniqueSet());
	for (auto record : records) {
		//cout << (record->finalTransfer ? "*" : " ");
		//cout << record->from << "--" << record->ch << "->" << record->to << endl;
		(*transferChart_)[record->from][record->ch] = record->to;
	}
	builted = true;
	nfa_.clearEnv();
}


bool Dfa::findElement(const vector<int>& vec, int v) {
	for (auto i : vec) {
		if (i == v)
			return true;
	}
	return false;
}

void Dfa::getAllChars(shared_ptr<set<statusPtr>>& st, set<int>& res) {
	// o(n^3) 效率实在不怎么样,但是优化的地步也不大
	for (auto status : (*st)) { // 对于每一个状态
		for (auto edge : (*status).outEdges) { // 对于每一条出边
			for (auto i : edge->matchContent)
				res.insert(i);
		}
	}
}

int Dfa::assignNewStatusLabel() { // 用于分配一个新的编号
	static int newStatusCount = 0;
	return newStatusCount++;
}