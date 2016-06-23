#ifndef BILI_LEXICAL_CAST_H
#define BILI_LEXICAL_CAST_H

#include <string>
#include <sstream>

//=======================================================
//                 字面内容类型转换
//=======================================================
class LexicalCastTemp : public std::stringstream
{
public:
	template<class T>
	LexicalCastTemp(const T& val)
	{
		*this << val;
	}
};

template<class T>
T lexical_cast(LexicalCastTemp val)
{
	T tmp;
	val >> tmp;
	return tmp;
}

template<>
inline std::string lexical_cast<std::string>(LexicalCastTemp val)
{
	return val.str();
}

#endif
