#include "Match.h"
#include "Nfa.h"


Match::Match(Nfa& nfa, const wstring& s) :
	nfa_(nfa),
	prevNotEndedCommandPos_(-1),
	matchContent_(s)
{

}


Match::~Match()
{
}


bool isNotEnded(Edge::type tp) {
/* 判断字符是否为完结 */
	switch (tp)
	{
	case Edge::loop:
	case Edge::capture:
	case Edge::storage:
		return true;
	default:
		break;
	}
	return false;
}

/*
 * greedyMatch 贪婪匹配算法的实现,该算法实现后,扩展的正则表达式引擎的大部分工作都已经完成了.
 */
bool Match::greedyMatch() {
	statusPtr start = nfa_.compressNfa();
	/* 首先压栈 */
	prevNotEndedCommandPos_ = -1;
	int size = matchContent_.size();
	status_.push(make_shared<StatusItem>(start, -1, 0, 0));
	bool shouldRestoreStack = false;
	while (!status_.empty()) {
		statusItemPtr s = status_.top();
		int currentPos = s->startPos;
		int remains = size - currentPos; /* 剩余的字符 */
		if (nfa_.finalStatus_.find(s->stat) != nfa_.finalStatus_.end() && remains == 0) { /* 到达了最终的节点 */
			clearCommandAndStatusStack();
			cout << "Yes, We Got it!" << endl;
			return true;
		}
		if (shouldRestoreStack) 
			restoreStack(s); /* 不断地要恢复原来的状态 */

		for (s->label++; s->label < s->stat->outEdges.size(); s->label++) { /* 不断地尝试新的边 */
			edgePtr e = s->stat->outEdges[s->label]; /* 按照顺序来选择一条边 */
			if (!e->isEps() && (remains > 0) && /* 新边可以匹配字符,而且匹配成功了 */
				(nfa_.char2Index(matchContent_[currentPos]) == e->matchContent[0])) {
				/* 开始处理这条新的非ε边 */
				if (!handleMethodItem(e, currentPos)) {
					/* 一般这里不会出错,否则的话,一定是正则表达式写错了. */
					assert(0);
				}
				currentPos++;
				status_.push(make_shared<StatusItem>(e->to, -1, commands_.size(), currentPos));
				goto success;
			}
			else if (e->isEps()) { /* 该边为ε边,也就是不消耗字符的边 */
				if (handleMethodItem(e, currentPos)) { /* 处理成功 */
					status_.push(make_shared<StatusItem>(e->to, -1, commands_.size(), currentPos));
					goto success;
				}
			}
			restoreStack(s);
		}
		/* 运行到了这里表示失败了! */
		shouldRestoreStack = true;
		status_.pop();
		continue;

	success:
		shouldRestoreStack = false;
	}
	clearAllStack(); /* 清空所有堆栈 */
	return false;
}



void Match::restoreStack(statusItemPtr &s) { /* 匹配不成功的时候,很有必要将栈恢复到原来的状态 */
	/* 一旦要换另外一条路,那么堆栈什么的,都需要复原成原来的样子 */
	int pos = s->depth - 1;
	if (pos < 0) prevNotEndedCommandPos_ = -1;
	else if (isNotEnded(commands_[pos].tp)) {
		prevNotEndedCommandPos_ = s->depth - 1;
	}
	else
		prevNotEndedCommandPos_ = commands_[pos].prevNotEndedCommandPos;
	while (commands_.size() > s->depth)
		commands_.pop_back();
	/* Storage这个栈也需要复原 */
	while (!storage_.empty() && storage_.top().depth > commands_.size())
		storage_.pop();
}



/*
 * handleEpsEdge 处理eps边,也就是不消耗字符的边.
 */
bool Match::handleMethodItem(edgePtr& e, int& currentPos) { /* 处理eps边 */
	for (auto item : e->items) {
		switch (item.tp)
		{
		case Edge::head: { /* 遇到head一定要保证是第一个字符 */
			if (currentPos != 0) return false;
			break;
		}
		case Edge::tail: { /* 遇到tail一定要保证是最后一个字符 */
			if (currentPos != matchContent_.size() - 1) return false;
			break;
		}
		case Edge::capture:
		case Edge::storage:
		case Edge::loop: {
			/* 如果碰到了loop的话,直接入栈 */
			commands_.push_back(CommandItem(item.tp, item.name, prevNotEndedCommandPos_, currentPos));
			prevNotEndedCommandPos_ = commands_.size() - 1;
			break;
		}
		case Edge::end: {
			int currentNotEndedCommandPos = prevNotEndedCommandPos_;
			prevNotEndedCommandPos_ = commands_[prevNotEndedCommandPos_].prevNotEndedCommandPos;
			CommandItem item = CommandItem(item.tp, item.name, prevNotEndedCommandPos_, currentPos);
			handleEnd(item, currentNotEndedCommandPos);
			break;
		}
		case Edge::check: {
			/* 如果碰到了check命令的话,要立即开始检查 */
			CaptureItem item = storage_.top();
			int size = matchContent_.size();
			for (int i = 0; i < item.len; ++i) {
				int lp = item.startPos + i;
				int rp = currentPos + i;
				/* 不能越界 */
				if (!(lp < size && rp < size) || (matchContent_[lp] != matchContent_[rp])) {
					return false; /* 这一步行不通,要换另外一条路 */
				}
			}
			currentPos += item.len; /* check是要消耗字符的 */
		}
		default:
			assert(0);
			break;
		}
	}
	return true;
}

void Match::handleEnd(CommandItem& item, int currentNotEndedCommandPos) {
	/* 如果碰到了end命令,那么我们要查看之前未处理完成的命令 */
	CommandItem curItem = commands_[currentNotEndedCommandPos];

	switch (curItem.tp) {
	case Edge::loop: {
		/* 如果碰到了loop,立即推入end */
		commands_.push_back(item);
		break;
	}
	case Edge::capture:
	case Edge::storage: {
		/* 如果碰到了storage命令,立即往storage堆栈中推入东西 */
		storage_.push(CaptureItem(curItem.name, commands_.size(), curItem.startPos, item.startPos - curItem.startPos));
		commands_.push_back(item);
		break;
	default:
		assert(0);
	}
	}
}

void Match::printCaptureContent() {
	while (!storage_.empty()) {
		CaptureItem it = storage_.top(); storage_.pop();
		wcout << '[' << it.name.c_str() << ']' <<
			matchContent_.substr(it.startPos, it.len).c_str() << endl;
	}
}

void Match::clearCommandAndStatusStack() {
	commands_.clear();
	while (!status_.empty())
		status_.pop();
}

void Match::clearAllStack() {
	commands_.clear();
	while (!status_.empty())
		status_.pop();
	while (!storage_.empty())
		storage_.pop();
}

