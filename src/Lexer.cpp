//
// Created by Anton on 14.11.2022.
//

#include <limits>
#include <iostream>

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

template<> char Lexer::get<char>() {
	char val = ' ';
	if (strm.peek() == '\n') {
		return val;
	}
	strm >> std::noskipws >> val;
	return val;
}

char Lexer::peek() {
	return strm.peek();
}