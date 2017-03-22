#ifndef LEX_H_
#define LEX_H_

#include <string>
#include <iostream>
#include <memory>
using namespace std;

struct Token;

typedef shared_ptr<Token> tokenPtr;
struct Token {
	enum tokenType {
		kRange,				/* 用于表示普通的字符 */
		kAlter,				/* 用于表示选择(|) */
		kRepeate,			/* 用于表示重复 */
		kCheckName,			/* 命名检查时用的名称 */
		kCaptureName,		/* 命名捕获的名称 */
		kMethodLBracket,	/* 用于表示方法 */
		kBoundary,			/* 用于边界检查(^, $) */
		kRBracket,			/* 表示右括号 */
		kLBracket,			/* 表示左括号 */
		kGreedy,			/* 代表贪心(?) */
		kEqual,				/* (=),只有在特殊模式下才能解析出这种token */
		kColon,				/* (:),只有在特殊模式下才能解析出这种token */
		kExClamation,		/* (!),只有在特殊模式下才能解析出这种token */
		kEOF
	};
	tokenType type;
	wstring content;

	Token(tokenType t,const wchar_t * content) :
		type(t),
		content(content)
	{}


	Token(tokenType t, wstring::iterator pos1, wstring::iterator pos2) :
		type(t),
		content(pos1, pos2)
	{
	}

	friend wostream& operator<<(wostream& os, Token& t) {
		return os << t.content.c_str() << L" ";
	}
	
	wchar_t* type2String();
};


/*
 * Lex 主要用于解析字符,通过分析一个一个字符,分析出一个一个token单元.
 */
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
	enum mode {
		normal,			/* 一般的处理模式 */
		special,		/* 特殊的处理模式 */
	};
	mode mode_;
	bool isSpecialCharact(wchar_t c) { /* 判断字符是否是特殊的字符 */
		return ((c == L'^') || (c == L'$') || (c == L'(') || (c == L')') \
			|| (c == L'[') || (c == L']') || (c == L'.') || (c == L'*') \
			|| (c == L'|') || (c == L'\\') || (c == L'{') || (c == L'}') \
			|| (c == L'?') || (c == L'+'));
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
		return ((c == L'd') || (c == L'D') || (c == L'w') || (c == L'W') \
			|| (c == L's'));
	}

	bool isEscapeCharacter(wchar_t c) { /* 用于判断是否为字符的转义,暂时只支持下面这些字符的转义 */
		return ((c == L'(') || (c == L')') || (c == L'\\') || (c == L'*') || (c == L'['));
	}
	tokenPtr handleSpecial();
	tokenPtr handleNormal();
	tokenPtr handleSquareLBracket();
	tokenPtr handleCrulyBracket();
	tokenPtr handleBracket();
	tokenPtr productToken(Token::tokenType t, int len);
};

#endif // !LEX_H

