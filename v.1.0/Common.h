#ifndef COMMON_H_
#define COMMON_H_
#include <set>
#include <assert.h>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <queue>
using namespace std;
/*
 * 这个文件主要包含一些全局的变量.
 */

typedef set<wchar_t> charSet;
typedef shared_ptr<charSet> setPtr;

const int numOfChars = 65535;

int nextLabel();

/*
 * printErrorAndExit -- 打印出错信息,然后退出.
 */
void printErrorAndExit(const wstring& msg);

/*
 * removeElement -- 从vector中删除对应的元素.
 */
template<typename T>
void removeElement(vector<T>& vec, const T & element)
{
	for (vector<T>::iterator it = vec.begin(); it != vec.end(); ++it) {
		if (*it == element) {
			vec.erase(it);
			return;
		}
	}
}


#endif