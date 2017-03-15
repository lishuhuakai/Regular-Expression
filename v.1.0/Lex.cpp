#include "Lex.h"
#include <assert.h>


Lex::Lex(const wchar_t* regularExpression) :
	expression_(regularExpression),
	currentPos_(expression_.begin())
{
}


Lex::~Lex()
{

}


tokenPtr Lex::productToken(Token::tokenType t, int len, int operators) { // 使用一个统一的函数来产生对象
	wstring::iterator it = currentPos_;
	currentPos_ += len;
	return make_shared<Token>(t, it, currentPos_, operators);
}

tokenPtr Lex::getNextToken()
{
	while (currentPos_ != expression_.end()) {
		if (L'\\' == *currentPos_) { // 如果是转义字符
			wstring::iterator nextPos = currentPos_ + 1;
			if (nextPos != expression_.end()) {
				if (isSpecialCharact(*nextPos)) { // 关于特殊字符的转义
					++currentPos_; // 去除掉"\\"
					return productToken(Token::kNormal, 1);
				}
				else if (isMethodCharact(*nextPos)) {
					return productToken(Token::kNormal, 2);
				}

			}
			else printErrorAndExit(L"错误的结尾!");
		}
		else if (!isSpecialCharact(*currentPos_)) // 非特殊字符
			return productToken(Token::kNormal, 1);
		else { // 接下来处理一些特殊的字符,基本上属于一些符号
			switch (*currentPos_) {
			// ^仅能在正则式的开头出现,而$仅能在结尾出现
			case L'(': return handleBracket(); 
			case L')': return productToken(Token::kRBracket, 1,  0);
			case L'[': return handleSquareBracket();
			case L'{': return handleCrulyBracket();	
			case L'|': return productToken(Token::kVerticalBar, 1, 2);
			case L'.': return productToken(Token::kNormal, 1, 0);
			case L'*': return productToken(Token::kRepeate, 1, 1);
			case L'?': return productToken(Token::kRepeate, 1, 1);
			case L'+': return productToken(Token::kRepeate, 1, 1);
			default:
				printErrorAndExit(L"无法识别的字符");
			}
		}
	}
	return make_shared<Token>(Token::kEOF, L"");
}

tokenPtr Lex::handleBracket() {
	// 这里意味着*currentPos_ == '(',这里我稍微要拓展一下,其实这些所谓的功能,包括表达式命名
	// 引用等,其实都和(是差不多的
	wstring::iterator nextPos = currentPos_ + 1;
	if (*nextPos == L'?') { // 匿名捕获 例子:(?[0-9]+)
		if (nextPos != expression_.end()) {
			nextPos++;
			if (*nextPos == L'=' || *nextPos == L'!') { // 正向预查 windows(?=98|2000) 反向预查 windows(?!98|2000)
				goto end;
			}
			else if (*nextPos == L':') { // 匿名捕获的另外一种形式 (?:
				if (nextPos != expression_.end()) {
					nextPos++;
					if (*nextPos == L'<') { // 这里包含两种形式,一种是(?:<#xxx> 命名捕获 以及 (?:<$xxx>命名检查,不再细分下去
						nextPos = find(nextPos, expression_.end(), L'>');
						if (nextPos != expression_.end())
							goto end;
					}
					else { // 否则的话,这里就是纯粹的匿名捕获了
						return productToken(Token::kMethod, nextPos - currentPos_);
					}
				}
				else
					printErrorAndExit(L"匿名捕获不完整");
			}
			else {
				nextPos--;
				goto end;
			}

		}
	}
	else // 这里只是单纯的(而已
		return productToken(Token::kLBracket, 1);
end:
	return productToken(Token::kMethod, nextPos - currentPos_ + 1);
}

tokenPtr Lex::handleCrulyBracket() { // 这个函数主要用于处理{}号, {}一般用于表示重复的次数
	auto end = find(currentPos_, expression_.end(), L'}'); // 在余下的字符中寻找}
	if (end != expression_.end())
		return productToken(Token::kRepeate, end - currentPos_ + 1, 1);
	else {
		printErrorAndExit(L"仅有{,而没有},错误的正则表达式!");
		//return make_shared<Token>();
	}
}

tokenPtr Lex::handleSquareBracket() { // 这个函数主要用于处理[xyz]以及[^xyz]符号,[xyz]一般表示从xyz中选择一个,而[^xyz]一般表示从xyz之外的元素选择一个
	auto end = find(currentPos_, expression_.end(), L']'); 
	if (end != expression_.end()) 
		return productToken(Token::kNormal, end - currentPos_ + 1,  0);
	else
		printErrorAndExit(L"仅有[,而没有],错误的正则表达式!");
}
