#ifndef CSHANTY_TOKEN_H
#define CSHANTY_TOKEN_H

#include <string>
#include "position.hpp"

namespace cshanty{

class Token{
public:
	Token(Position * pos, int kindIn);
	virtual std::string toString();
	size_t line() const;
	size_t col() const;
	int kind() const;
	Position * pos() const;
protected:
	Position * myPos;
private:
	const int myKind;
};

class IDToken : public Token{
public:
	IDToken(Position * posIn, std::string valIn);
	const std::string value() const;
	virtual std::string toString() override;
private:
	const std::string myValue;
	
};

class StrToken : public Token{
public:
	StrToken(Position * posIn, std::string valIn);
	virtual std::string toString() override;
	const std::string str() const;
private:
	const std::string myStr;
};

class IntLitToken : public Token{
public:
	IntLitToken(Position * posIn, int numIn);
	virtual std::string toString() override;
	int num() const;
private:
	const int myNum;
};

}

#endif
