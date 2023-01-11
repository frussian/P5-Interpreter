//
// Created by Anton on 14.11.2022.
//

#include <limits>
#include <iostream>
#include <ctype.h>
#include "Lexer.h"
#include "p5_errors.h"

Lexer::Lexer(std::string &filename):
	filename(filename) {
}

void Lexer::start() {
	strm.open(filename);
	if (!strm.is_open()) {
		P5_ERR("cannot open file %s\n", filename.c_str());
	}
}

bool Lexer::is_eof() {
	return strm.eof();
}

void Lexer::get_line() {
	line++;
	strm.ignore(std::numeric_limits<std::streamsize>::max(), strm.widen('\n'));
}

int Lexer::line_num() {
	return line;
}

template<> 
char Lexer::get<char>() {
	char val = ' ';
	if (strm.peek() == '\n') {
		return val;
	}
	strm >> std::noskipws >> val;
	return val;
}

template<>
unsigned char Lexer::get<unsigned char>() {
	int val;
	strm >> val;
	return val;
}

template <>
std::string Lexer::get<std::string>() {
	std::string str;
	auto &s = strm >> std::skipws;
	char c;
//strm
//	do {
//		strm >> c;
////		printf("%c(%d) ", c, c);
//
//	} while (iswspace(c));

	strm >> c;
	str += c;

//	printf("%d\n", s.peek());

	while (isalpha(c = s.peek())) {
		s >> c;
		str += c;
	}
//	strm >> std::skipws >> str;
	return str;
}

char Lexer::peek() {
	return strm.peek();
}

void Lexer::skip_spaces() {
	while (strm.peek() == ' ') {
		get<char>();
	}
}