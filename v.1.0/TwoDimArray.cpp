#include "TwoDimArray.h"
#include <iostream>
using namespace  std;


TwoDimArray::TwoDimArray(int line, int size) :
	numOflines_(line),
	size_(size)
{
	buff_.reserve(line);
	for (int i = 0; i < line; ++i) {
		buff_.push_back(move(vector<int>(size, -1)));
	}
}

vector<int>& TwoDimArray::operator [](int i) {
	return buff_[i];
}

TwoDimArray::~TwoDimArray()
{
}
