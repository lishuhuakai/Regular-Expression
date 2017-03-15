#ifndef LEX_H_
#define LEX_H_

#include <string>
#include <iostream>
#include <memory>
#include "common.h"
using namespace std;

struct Token;

// Lex类主要用于解析字符,通过分析一个一个字符,分析出一个一个token单元
typedef shared_ptr<Token> tokenPtr;
struct Token {
	enum tokenType {
		kLBracket = 0, kRBracket, kVerticalBar,\
		kRepeate, kConcat,
		kMethod,  /* 这个应该属于扩展的token,主要用于描述功能 */
		kOrigin, /* 最后的kOrigin是虚构的一个元素 */
		kNormal, /* 用于表示一个单一的字符 */
		kEOF /* 用于表示最后一个token */
	};
	tokenType type;
	wstring content;
	int operators;	/* 作用的运算符的数目 */

	Token(tokenType t,const wchar_t * content, int operators=0) :
		type(t),
		content(content),
		operators(operators)
	{}


	Token(tokenType t, wstring::iterator pos1, wstring::iterator pos2, int operators) :
		type(t),
		content(pos1, pos2),
		operators(operators)
	{
	}

	friend ostream& operator<<(ostream& os, Token& t) {
		return os << t.content.c_str() << " ";
	}
};



class Lex
{
public:
	Lex(const wchar_t*);
	~Lex();
public:
	tokenPtr getNextToken();		/* 获取下一个Token */
	wstring expression_;
	void clear() {
		currentPos_ = expression_.begin();
	}
private:
	wstring::iterator currentPos_;  /* 当前正处理到的位置 */
	bool isSpecialCharact(wchar_t c) { /* 判断字符是否是特殊的字符 */
		return ((c == '^') || (c == '$') || (c == '(') || (c == ')') \
			|| (c == '[') || (c == ']') || (c == '.') || (c == '*') \
			|| (c == '|') || (c == '\\') || (c == '{') || (c == '}') \
			|| (c == '?') || (c == '+'));
	}

	/** 
	* 功能型字符说明:
	* \d  [0-9]
	* \D  [^0-9]
	* \w  [0-9A-Za-z_]
	* \W  [^0-9A-Za-z_]
	* \s  [ \t]
	*/
	bool isMethodCharact(wchar_t c) { /* 判断字符是否为功能性字符 */
		return ((c == 'd') || (c == 'D') || (c == 'w') || (c == 'W') \
			|| (c == 's'));
	}

	bool isEscapeCharacter(wchar_t c) { /* 用于判断是否为字符的转义,暂时只支持下面这些字符的转义 */
		return ((c == '(') || (c == ')') || (c == '\\') || (c == '*') || (c == '['));
	}

	tokenPtr handleCrulyBracket();
	tokenPtr handleSquareBracket(); 
	tokenPtr handleBracket();
	tokenPtr productToken(Token::tokenType t, int len, int operators=0);
};

#endif

