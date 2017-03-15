#pragma once
#include "common.h"
#include "Nfa.h"

class TwoDimArray;

class Dfa
{
public:
	Dfa(Nfa& n);
	~Dfa();
public:
	bool match(const wstring& str);
	void buildDFA();
private:
	Nfa& nfa_;
	bool builted;
	shared_ptr<TwoDimArray> transferChart_; // 转换表
	set<int> finalStatus_; // 用于记录最终的结束状态
private:
	bool findElement(const vector<int>& vec, int v); 
	void getAllChars(shared_ptr<set<statusPtr>>&, set<int>&);
	int assignNewStatusLabel();
};

