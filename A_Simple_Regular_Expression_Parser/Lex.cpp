#include "Lex.h"
#include <assert.h>


Lex::Lex(const wchar_t* regularExpression) :
	expression_(regularExpression),
	currentPos_(expression_.begin()),
	mode_(normal)
{
}


Lex::~Lex()
{

}

tokenPtr Lex::handleSpecial() {  /* 在特殊的模式下处理字符 */
	/* 什么时候会进入特殊模式?当getNextToken返回了一个(?也就是MethodLBracket的时候,
	 * 会进入特殊模式,为什么要进入特殊模式?因为在这个模式里,平时我们当做普通字符的!,:之类的字符
	 * 有了特殊的含义.
	 */
	mode_ = normal;
	if (*currentPos_ == L'=') {
		return productToken(Token::kEqual, 1);
	}
	else if (*currentPos_ == L'!') {
		return productToken(Token::kExClamation, 1);
	}
	else if (*currentPos_ == L':') {
		if (*(currentPos_ + 1) == L'<') /* 命名捕获 */
			mode_ = special;
		return productToken(Token::kColon, 1);
	}
	else if (*currentPos_ == L'<') { /* 产生name这种Token */
		auto end = find(currentPos_, expression_.end(), L'>'); /* 在余下的字符中寻找'>' */
		if (end != expression_.end()) {
			if (*(currentPos_ + 1) == '$')
				return productToken(Token::kCheckName, end - currentPos_ + 1);
			else if (*(currentPos_ + 1) == '#')
				return productToken(Token::kCaptureName, end - currentPos_ + 1);
			else
				assert(0); /* 语法错误 */
		}
	}
}

/*
 * productToken 我们使用一个统一的函数来产生对象.
 */
tokenPtr Lex::productToken(Token::tokenType t, int len) { 
	wstring::iterator it = currentPos_;
	currentPos_ += len;
	return make_shared<Token>(t, it, currentPos_);
}

tokenPtr Lex::getNextToken() {
	if (mode_ == normal)
		return handleNormal();
	else
		return handleSpecial();
}

/*
 * 在一般模式下处理字符.
 */
tokenPtr Lex::handleNormal()
{
	while (currentPos_ != expression_.end()) {
		if (L'\\' == *currentPos_) { /* 如果是转义字符 */
			wstring::iterator nextPos = currentPos_ + 1;
			if (nextPos != expression_.end()) {
				if (isSpecialCharact(*nextPos)) { /* 关于特殊字符的转义 */
					++currentPos_; /* 去除掉"\\" */
					return productToken(Token::kRange, 1);
				}
				else if (isMethodCharact(*nextPos)) { /* 如果是方法元字符 */
					return productToken(Token::kRange, 2);
				}
			}
			else
				assert(0);
		}
		else if (!isSpecialCharact(*currentPos_)) /* 非特殊字符 */
			return productToken(Token::kRange, 1);
		else { /* 接下来处理一些特殊的字符,基本上属于一些符号 */
			switch (*currentPos_) {
			/* ^仅能在正则式的开头出现,而$仅能在结尾出现 */
			case L'^': 
			case L'$':
				return productToken(Token::kBoundary, 1);		  /* 边界字符 */
			case L'(': 
				return handleBracket();
			case L')': return productToken(Token::kRBracket, 1);  /* 右括号 */
			case L'[': return handleSquareLBracket();
			case L'{': return handleCrulyBracket();	
			case L'|': return productToken(Token::kAlter, 1);	  /* 选择 */
			case L'.': return productToken(Token::kRange, 1);
			case L'+':
			case L'*': return productToken(Token::kRepeate, 1);   /* (+, *)代表重复 */
			case L'?': return productToken(Token::kGreedy, 1);    /* (?)代表贪心 */
			default:
				assert(0);
			}
		}
	}
	return make_shared<Token>(Token::kEOF, L"");  /* 表示结束的字符 */
}

tokenPtr Lex::handleSquareLBracket() {
	auto end = find(currentPos_, expression_.end(), L']'); /* 在余下的字符中寻找']' */
	if (end != expression_.end())
		return productToken(Token::kRange, end - currentPos_ + 1);
	else
		assert(0);
}

/*
 * handleBracket 主要是用于处理(?: 方法字符.
 */
tokenPtr Lex::handleBracket() {
	/* 这里意味着*currentPos_ == '(',这里我稍微要拓展一下,其实这些所谓的功能,包括表达式命名
	 * 引用等,其实都和(是差不多的
	 */
	wstring::iterator nextPos = currentPos_ + 1;
	if (*nextPos == L'?') {
		mode_ = special;
		return productToken(Token::kMethodLBracket, 2);
	}
	else /* 这里只是单纯的(而已 */
		return productToken(Token::kLBracket, 1);
}

/*
 * handleCrulyBracket  这个函数主要用于处理{}号, {}一般用于表示重复的次数.
 */
tokenPtr Lex::handleCrulyBracket() { 
	auto end = find(currentPos_, expression_.end(), L'}'); /* 在余下的字符中寻找'}' */
	if (end != expression_.end()) 
		return productToken(Token::kRepeate, end - currentPos_ + 1);
	else 
		assert(0);
}

wchar_t* Token::type2String() {
	switch (type)
	{
	case Token::kRange:
		return L"kRange";
	case Token::kAlter:
		return L"kAlter";
	case Token::kRepeate:
		return L"kRepeate";
	case Token::kCaptureName:
		return L"kCaptureName";
	case Token::kCheckName:
		return L"kCheckName";
	case Token::kMethodLBracket:
		return L"kMethodLBracket";
	case Token::kBoundary:
		return L"kBoundary";
	case Token::kRBracket:
		return L"kRBracket";
	case Token::kLBracket:
		return L"kLBracket";
	case Token::kGreedy:
		return L"kGreedy";
	case Token::kEqual:
		return L"kEqual";
	case Token::kColon:
		return L"kColon";
	case Token::kExClamation:
		return L"kExClamation";
	case Token::kEOF:
		return L"kEOF";
	default:
		assert(0);
	}
}

