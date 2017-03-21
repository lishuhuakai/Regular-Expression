#ifndef TWODIMARRAY_H
#define TWODIMARRAY_H
#include "Common.h"
/*
 * TwoDimArray主要是对动态分配二维数组的一个简单封装.
 */
class TwoDimArray
{
public:
	TwoDimArray(int lines, int size);
	~TwoDimArray();
	vector<int>& operator[](int);
private:
	int numOflines_;  // 行的数目 
	int size_; // 每一行的大小
	vector<vector<int>> buff_;
};

#endif // !TWODIMARRAY_H
