//
// Created by Anton on 14.11.2022.
//

#ifndef P5_INTERPRETER_LEXER_H
#define P5_INTERPRETER_LEXER_H

#include <string>
#include <utility>
#include <fstream>

#include "p5_common.h"

class Lexer {
public:
	explicit Lexer(std::string &filename);
	void start();
	bool is_eof();

	template <typename T>
	T get();

	void get_line();
	void skip_spaces();
	int line_num();
	char peek();
private:
	std::string filename;
	std::ifstream strm;
	int line = 1;
};

template<typename T> T Lexer::get() {
	T val;
	strm >> std::skipws >> val;
	return val;
}

#endif //P5_INTERPRETER_LEXER_H
