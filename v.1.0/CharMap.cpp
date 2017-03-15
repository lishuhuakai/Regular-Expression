#include "CharMap.h"

CharMap::CharMap(list<setPtr>& allCharSets) :
	builted(false),
	allCharSets_(allCharSets)
{
	memset(buffer_, -1, sizeof(buffer_));
}


CharMap::~CharMap()
{
	allCharSets_.clear();
	uniqueCharSets_.clear();
}

void CharMap::buildCharClass() {
	charSet intersection;
	while (true) {
	start:
		if (0 == allCharSets_.size()) break;
		for (list<setPtr>::iterator i1 = allCharSets_.begin(); i1 != allCharSets_.end(); ++i1) {
			for (list<setPtr>::iterator i2 = next(i1); i2 != allCharSets_.end(); ++i2) {
				set_intersection((*i1)->begin(), (*i1)->end(), (*i2)->begin(), 
					(*i2)->end(), insert_iterator<charSet>(intersection, intersection.begin()));
				if ((intersection.size() == (*i1)->size()) && 
					(intersection.size() == (*i2)->size())) { // 出现了两个相同的元素
					allCharSets_.erase(i1);
				}
				else if (intersection.size() == (*i1)->size()) { // i1是i2的子集
					setPtr sub1 = make_shared<charSet>();
					set_difference((*i2)->begin(), (*i2)->end(), intersection.begin(),
						intersection.end(), insert_iterator<charSet>(*sub1, sub1->begin()));
					allCharSets_.erase(i2);
					allCharSets_.push_front(move(sub1));
				} 
				else if (intersection.size() == (*i2)->size()) { // i2是i1的子集
					setPtr sub1 = make_shared<charSet>();
					set_difference((*i1)->begin(), (*i1)->end(), intersection.begin(),
						intersection.end(), insert_iterator<charSet>(*sub1, sub1->begin()));
					allCharSets_.erase(i1);
					allCharSets_.push_front(move(sub1));
				}
				else if (intersection.size() != 0) { // 两个集合存在交集
				    // 那么我们要做的,是将元素拆开
					setPtr sub1 = make_shared<charSet>();
					setPtr sub2 = make_shared<charSet>();
					// 求出差集
					set_difference((*i1)->begin(), (*i1)->end(), intersection.begin(),
						intersection.end(), insert_iterator<charSet>(*sub1, sub1->begin()));

					set_difference((*i2)->begin(), (*i2)->end(), intersection.begin(),
						intersection.end(), insert_iterator<charSet>(*sub2, sub2->begin()));
					
					allCharSets_.erase(i1); allCharSets_.erase(i2); // 删除原来的元素
					
					allCharSets_.push_front(move(sub1)); allCharSets_.push_front(move(sub2)); 
					allCharSets_.push_front(make_shared<charSet>(move(intersection))); // intersection内容不再使用
				}
				else {
				// 如果不存在交集,那么继续往下
					continue;
				}

				intersection.clear();
				goto start;
			}
			// 如果运行到了这里,表示i1这个元素和其余所有元素没有交集,因此,这是一个解
			uniqueCharSets_.push_back(move(*i1)); 
			allCharSets_.erase(i1);
			goto start; // 移除i1后,指针失效,因此break,不是很好的做法
		}
	}
	builted = true;
	// 接下来需要执行映射
	size_t size = uniqueCharSets_.size();
	for (size_t i = 0; i < size; ++i) {
		for (wchar_t ch : *(uniqueCharSets_[i])) {
			buffer_[ch] = i;
		}
	}
	numOfUniqueSets_ = size;
}

int CharMap::char2Index(wchar_t ch) {
	return buffer_[ch];
}

void CharMap::getLabels(const charSet& s, bool notIncludeContent, vector<int>& records) {
	if (!builted) buildCharClass();
	// include表示是否包括含相等的集合的编号
	size_t size = uniqueCharSets_.size();
	charSet intersection;
	for (size_t i = 0; i < size; ++i) {
		set_intersection(uniqueCharSets_[i]->begin(), uniqueCharSets_[i]->end(), s.begin(), s.end(),
			insert_iterator<charSet>(intersection, intersection.begin()));
		bool equal = (intersection.size() == uniqueCharSets_[i]->size());
		if (equal && !notIncludeContent) // 两个集合相等,records要包含相等集合的编号
			records.push_back(i);
		else if (!equal && notIncludeContent)  // 两个集合不相等,records要包含不相等集合的编号 
			records.push_back(i);
		else if (equal & notIncludeContent) {
			// nothing to do
		}
		else if (!equal && notIncludeContent) {
			// nothing to do
		}
		intersection.clear();
	}

	if (notIncludeContent) {
		if (numOfUniqueSets_ != size - 1)
			addOthers();
		records.push_back(size); // 最后一个集合表示其他字符
	}
}

void CharMap::getAllLabels(vector<int>& records) {
	if (!builted) buildCharClass();
	size_t i = 0, size = uniqueCharSets_.size();
	for (; i < size; ++i) {
		records.push_back(i);
	}
	if (numOfUniqueSets_ != size + 1)
		addOthers();
	records.push_back(i); // 最后这个表示其他字符
	// 一旦调用了这个函数,表明正则表达式中出现了.,也就是要考虑所有的字符,这样的话,必须在已有的字符类上面添加一个
	// 新的字符类,表示除了已有的字符类之外的所有字符
}

void CharMap::addOthers() {
	numOfUniqueSets_ = uniqueCharSets_.size();
	for (int j = 0; j < numOfUniqueSets_; j++) {
		if (buffer_[j] == -1)
			buffer_[j] = numOfUniqueSets_;
	}
	numOfUniqueSets_++;
}
