//
// Created by Anton on 13.11.2022.
//

#ifndef P5_INTERPRETER_P5_COMMON_H
#define P5_INTERPRETER_P5_COMMON_H

#include <vector>

namespace P5 {
	typedef int addr_t;  //q
	typedef unsigned char ins_t; //opcode
	typedef unsigned char lvl_t; //p
	typedef unsigned char set_el_t;
	typedef std::vector<set_el_t> set;
	typedef std::vector<unsigned char> store_t;
	typedef double real_t;
	typedef unsigned char bool_t;
	typedef char char_t;
	typedef int int_t;

	static int int_size = 4;
	static int char_size = 1;
	static int bool_size = 1;
	static int set_size = 32;
	static int max_store = 16777216;
}

#endif //P5_INTERPRETER_P5_COMMON_H
