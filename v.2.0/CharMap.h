#ifndef CHARMAP_H
#define CHARMAP_H

#include "common.h"
/* 
 * 这个类主要实现字符表的压缩,因为如果真的要一个一个字符来映射的话,wchar_t有65536个字符,这实际上是非常大的一张表
 * 而实际上,我们没有必要实现得那么大,因为很多字符都是等价的(对待每一个状态的转换都是一致的),因此可以大大地压缩.
 */

class CharMap
{
public:
	CharMap(list<setPtr>&);
	~CharMap();
	int buffer_[numOfChars];
public:
	void buildCharClass(); // 构建字符类
	void getLabels(const charSet& s, bool include, vector<int>& records);
	void getAllLabels(vector<int>& records); // 获得所有的标签
	int char2Index(wchar_t ch);
	int numOfUniqueSet() {
		assert(builted == true);
		return numOfUniqueSets_;
	}
private:
	int numOfUniqueSets_;
	list<setPtr>& allCharSets_;
	vector<setPtr> uniqueCharSets_;
	bool builted;  // 用于记录是否已经构建了CharClass
	void addOthers(); // 其他所有的字符为一个类,这一般出现在[^],以及.中
};

#endif // !CHARMAP_H

